/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
Licence Publique G�n�rale GNU pour plus de d�tails.

Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 B�timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'H�res
*/
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "debug.h"


//L/S      :
//      byte/word :LDR ,LDRB,LDRH,STR,STRB,STRH
//      misce
//Multiple :LDM(1),STM(1)

uint32_t calcul_addr_immediate(arm_core p,uint32_t ins){//A5-20 p460
    uint32_t addr = 0;
    //uint8_t rn = get_bits(ins,19,16);
    uint16_t offset = get_bits(ins,11,0);
    if (bitU(ins)){
        return addr = arm_read_register(p,bits_rn(ins)) + offset;
    }else{
        return addr = arm_read_register(p,bits_rn(ins)) - offset;
    }
    
}
uint32_t calcul_addr_register(arm_core p,uint32_t ins){//P A5-21
    uint32_t addr = 0;
    //uint8_t rn = get_bits(ins,19,16);
    //uint8_t rm = get_bits(ins,3,0);
    if (bitU(ins)){
        return addr = arm_read_register(p,bits_rn(ins)) + arm_read_register(p,bits_rm(ins));
    }else{
        return addr = arm_read_register(p,bits_rn(ins)) - arm_read_register(p,bits_rm(ins));
    }
}
uint32_t calcul_addr_scaled(arm_core p,uint32_t ins){//P A5-22
    uint32_t addr = 0;
    uint32_t shift = get_bits(ins,6,5);
    uint32_t index = 0;
    uint32_t shift_imm = get_bits(ins,11,7);
    switch (shift){
        case 0b00://LSL
            index = arm_read_register(p,bits_rm(ins)) << shift_imm;
            break;
        case 0b01://LSR
            if(shift_imm == 0){
                index = 0;
            }else{
                index = arm_read_register(p,bits_rm(ins)) >> shift_imm;
            }
            break;
        case 0b10://ASR
            if(shift_imm ==0){
                if(get_bit(arm_read_register(p,bits_rm(ins)),31) == 1){
                    index = 0xFFFFFFFF;
                }else
                    index = 0;
            }else{
                index = asr(arm_read_register(p,bits_rm(ins)),shift_imm);
            }
            break;
        case 0b11://ROR/RRX
            if(shift_imm ==0){//RRX
                uint32_t c_flag = get_bit(arm_read_cpsr(p),29) << 31;
                uint32_t rm_val = arm_read_register(p,bits_rm(ins)) >> 1;
                index = c_flag | rm_val;

            }else{//ROR
                index = ror(arm_read_register(p,bits_rm(ins)),shift_imm);
            }
            break;
    
        default:
            index = 0;
            break;
    }
    if (bitU(ins)){
        return addr = arm_read_register(p,bits_rn(ins)) + index;
    }else{
        return addr = arm_read_register(p,bits_rn(ins)) - index;
    }
    
}
//A5-18 p458 used to calculate the address for load and store word or byte
int is_immediate_offset(uint32_t ins){//offset +-12  A5-18 p458
    if(get_bits(ins,27,25)==0x010){
        return 1;
    }
    return 0;
}
int is_register_offset(uint32_t ins){//offset +- arm_read_register(p,Rm)
    if(get_bits(ins,27,25) == 0x011 && get_bits(ins,11,4)==0 ){
        return 1;
    }
    return 0;
}
int is_scaled_offset(uint32_t ins){
    if(get_bits(ins,27,25) == 0x011 && !get_bit(ins,4)){
        return 1;
    }
    return 0;
}
uint32_t determiner_addr(arm_core p,uint32_t ins){
    int bitP = get_bit(ins,24);
    int bitW = get_bit(ins,21);
    //int bitI = get_bit(ins,25);
    uint32_t addr;
    if(bitP==0 && bitW ==0){//post index 
        if(is_immediate_offset){//A5-28 p468
            addr = arm_read_register(p,bits_rn(ins));
        }else if(is_register_offset){//A5-30
            addr = arm_read_register(p,bits_rn(ins));
        }else if(is_scaled_offset){//A5-31
            addr = arm_read_register(p,bits_rn(ins));
        }
    }else if(bitP ==1 && bitW == 0){//immediate offset
        if(is_immediate_offset){//A5-20 p460
            addr = calcul_addr_immediate(p,ins);
        }else if(is_register_offset){//A5-21
            addr = calcul_addr_register(p,ins);
        }else if(is_scaled_offset){//A5-22
            addr = calcul_addr_scaled(p,ins);
        }
    }else if(bitP == 0 && bitW == 1){
        return UNDEFINED_INSTRUCTION;
        
    }else if(bitP == 1 && bitW == 1){//pre index
        if(is_immediate_offset){//A5-24 p464
            addr = calcul_addr_immediate(p,ins);
        }else if(is_register_offset){//A5-25
            addr = calcul_addr_register(p,ins);
        }else if(is_scaled_offset){//A5-26
            addr = calcul_addr_scaled(p,ins);
        }
    }
    return addr;
}
//apres visiter les donne A5-24 address written back to teh base register Rn
uint32_t write_back_rn(arm_core p,uint32_t ins,uint32_t addr){
    int bitP = get_bit(ins,24);
    int bitW = get_bit(ins,21);
    //int bitI = get_bit(ins,25);
    uint32_t addr;
    //ifconditionpassed then
    if(bitP==0 && bitW ==0){//post index 
        if(is_immediate_offset){//A5-28 p468
            arm_write_register(p,bits_rn(ins),calcul_addr_immediate(p,ins));//+-12
        }else if(is_register_offset){//A5-30
            arm_write_register(p,bits_rn(ins),calcul_addr_register(p,ins));//+-rm
        }else if(is_scaled_offset){//A5-31
            arm_write_register(p,bits_rn(ins),calcul_addr_scaled(p,ins));//+-index
        }
    }else if(bitP == 1 && bitW == 1){//pre index
        if(is_immediate_offset){//A5-24 p464
            arm_write_register(p,bits_rn(ins),addr);
        }else if(is_register_offset){//A5-25
            arm_write_register(p,bits_rn(ins),addr);
        }else if(is_scaled_offset){//A5-26
            arm_write_register(p,bits_rn(ins),addr);
        }
    }
}


//A5-33 p473 used to calculate the address for load and store  halfword  load signed byte or load and store doubleword
int is_misimmediate_offset(uint32_t ins){
    if(get_bits(ins,27,25)==0x000 && bitB(ins) == 1){
        return 1;
    }
    return 0;
}
int is_misregister_offset(uint32_t ins){
    if(get_bits(ins,27,25)==0x000 && bitB(ins) == 0){
        return 1;
    }
    return 0;
}

//miscellaneous loads and store immediate offset
uint32_t calcul_addr_misimmediate(arm_core p,uint32_t ins){//A5-35 p475
    uint32_t addr = 0;
    uint8_t immedH = get_bits(ins,11,8);
    uint8_t immedL = get_bits(ins,3,0);

    uint8_t offset = (immedH << 4) | immedL;
    if (bitU(ins)){
        return addr = arm_read_register(p,bits_rn(ins)) + offset;
    }else{
        return addr = arm_read_register(p,bits_rn(ins)) - offset;
    }
    
}
//miscellaneous loads and store immediate offset
uint32_t calcul_addr_misregister(arm_core p,uint32_t ins){//A5-36
    uint32_t addr = 0;
    //uint8_t rn = get_bits(ins,19,16);
    //uint8_t rm = get_bits(ins,3,0);
    if (bitU(ins)){
        return addr = arm_read_register(p,bits_rn(ins)) + arm_read_register(p,bits_rm(ins));
    }else{
        return addr = arm_read_register(p,bits_rn(ins)) - arm_read_register(p,bits_rm(ins));
    }
}
//bit P W determiner pre/post index A5-34
uint32_t determiner_mis_addr(arm_core p,uint32_t ins){
    uint32_t addr;
    if(bitP(ins)==0 && bitW(ins) ==0){//post index 
        if(is_misimmediate_offset){//A5-39
            addr = calcul_addr_misimmediate(p,bits_rn(ins));
        }else if(is_misregister_offset){//A5-40
            addr = calcul_addr_misregister(p,bits_rn(ins));
        }  
    }else if(bitP(ins) ==1 && bit(ins) == 0){//unchanged offset addressing A5-35
        if(is_misimmediate_offset){
            addr = calcul_addr_misimmediate(p,ins);
        }else if(is_misregister_offset){
            addr = calcul_addr_misregister(p,ins);
        }    
    }else if(bitP(ins) == 0 && bitW(ins) == 1){
        return UNDEFINED_INSTRUCTION;
        
    }else if(bitP(ins) == 1 && bitW(ins) == 1){//pre index A5-37
        if(is_misimmediate_offset){
            addr = calcul_addr_misimmediate(p,ins);
            //rn = addr
        }else if(is_misregister_offset){
            addr = calcul_addr_misregister(p,ins);
            //rn = addr
        } 
    }
    return addr;
}
//apres visiter les donne A5-24 address written back to teh base register Rn
uint32_t write_back_mis_rn(arm_core p,uint32_t ins,uint32_t addr){
    int bitP = get_bit(ins,24);
    int bitW = get_bit(ins,21);
    //int bitI = get_bit(ins,25);
    uint32_t addr;
    //ifconditionpassed then
    if(bitP==0 && bitW ==0){//post index 
        if(is_immediate_offset){//A5-39
            arm_write_register(p,bits_rn(ins),calcul_addr_misimmediate(p,ins));//+-8
        }else if(is_register_offset){//A5-40
            arm_write_register(p,bits_rn(ins),calcul_addr_misregister(p,ins));//+-rm
        }
    }else if(bitP == 1 && bitW == 1){//pre index
        if(is_misimmediate_offset){
            arm_write_register(p,bits_rn(ins),addr);
            //rn = addr
        }else if(is_misregister_offset){
            arm_write_register(p,bits_rn(ins),addr);
            //rn = addr
        }    
    }
    return 0;
}



//instrction load/store
int instruction_ldr(arm_core p, uint32_t ins){//A4-43 p193
    //uint8_t rd = get_bits(ins,15,12);
    // uint32_t offset = get_bits(ins,11,0);
    uint32_t address = determiner_addr(p,ins);
    uint32_t word;
    if(arm_read_word(p,address,&word)==0){
        if(bits_rd(ins) == 15){
            arm_write_register(p,bits_rd(ins),word & 0xFFFFFFFE);
            uint8_t t_bit = get_bit(word,0);
            if(t_bit == 0){
                arm_write_cpsr(p,clr_bit(arm_read_cpsr(p),5));
            }else
                arm_write_cpsr(p,set_bit(arm_read_spsr(p),5));
        }else{
            arm_write_register(p,bits_rd(ins),word);
        }
    }
    write_back_rn(p,ins,address);
    return 0;
}
int instruction_ldrb(arm_core p, uint32_t ins){//A4-46
    uint8_t byte;
    uint32_t address = determiner_addr(p,ins);
    arm_read_byte(p,address,&byte);
    arm_write_register(p,bits_rd(ins),byte);
    write_back_rn(p,ins,address);
    return 0;
}
int instruction_ldrbt(arm_core p, uint32_t ins){
    //a completer
    return UNDEFINED_INSTRUCTION;
}
int instruction_ldrd(arm_core p, uint32_t ins){
    //a completer
    return UNDEFINED_INSTRUCTION;
}
int instruction_ldrh(arm_core p, uint32_t ins){ //A4-54 p 204
    uint16_t half;
    uint32_t address = determiner_misaddr(p,ins);
    if(bitU(ins)==0){//CP15_reg1_Ubit ? 
        if(get_bit(address,0) == 0){
        half = arm_read_half(p,address,&half);
        }else
            return UNDEFINED_INSTRUCTION;
    }else{//u=1
        half = arm_read_half(p,address,&half);
    }

    arm_write_register(p,bits_rd(ins),half);
    write_back_mis_rn(p,ins,address);
    return 0;
}
int instruction_ldrsb(arm_core p, uint32_t ins){
    //a completer
    return UNDEFINED_INSTRUCTION;
}
int instruction_ldrsh(arm_core p, uint32_t ins){
    //a completer
    return UNDEFINED_INSTRUCTION;
}
int instruction_ldrt(arm_core p, uint32_t ins){
    //a completer
    return UNDEFINED_INSTRUCTION;
}
int instruction_str(arm_core p, uint32_t ins){//A4-193 p343
    uint32_t address = determiner_addr(p,ins);  
    arm_write_word(p,address,arm_read_register(p,bits_rd(ins)));
    write_back_rn(p,ins,address);
    return 0;
}
int instruction_strb(arm_core p, uint32_t ins){//A4-195
    uint32_t address = determiner_addr(p,ins);  
    arm_write_word(p,address,arm_read_register(p,bits_rd(ins)&0xFF));  //rd[7:0]
    write_back_rn(p,ins,address);
    return 0;
}
int instruction_strh(arm_core p, uint32_t ins){//A4-204 p354
    uint16_t half;
    uint32_t address = determiner_misaddr(p,ins);
    //if(bitU(ins)==0)
    if(get_bit(address,0) == 0b0){
        half = arm_read_register(p,bits_rd(ins));
    }else
        return UNDEFINED_INSTRUCTION;

    arm_write_half(p,address,half);
    write_back_mis_rn(p,ins,address);
    return 0;
}
int instruction_strbt(arm_core p, uint32_t ins){
    //a completer
    return UNDEFINED_INSTRUCTION;
}
int instruction_strbt(arm_core p, uint32_t ins){
    //a completer
    return UNDEFINED_INSTRUCTION;
}
int instruction_strd(arm_core p, uint32_t ins){
    //a completer
    return UNDEFINED_INSTRUCTION;
}
int instruction_strsh(arm_core p, uint32_t ins){
    //a completer
    return UNDEFINED_INSTRUCTION;
}
int instruction_strt(arm_core p, uint32_t ins){
    //a completer
    return UNDEFINED_INSTRUCTION;
}
int arm_load_store(arm_core p, uint32_t ins) {
    int type = get_bits(ins,27,26);
    
    if(type){
        if(bitL(ins) == 1 && !bitB(ins)){
            instruction_ldr(p,ins);//ldr    !bit24 && bit21 -->ldrt  
        }else if(bitL(ins) && bitB(ins)){
            instruction_ldrb(p,ins);//ldrb
        }else if(!bitL(ins) && !bitB(ins)){
            instruction_str(p,ins);//str
        }else if(!bitL(ins) && bitB(ins)){
            instruction_strb(p,ins);//strb
        }else return UNDEFINED_INSTRUCTION;
    }else if(!type && !bitI(ins)){
        if(bitL(ins)){
            instruction_ldrh(p,ins);//ldrh
        }else if(!bitL(ins)){
            instruction_strh(p,ins);//strh
        }else return UNDEFINED_INSTRUCTION;
    }
    return 0;    
}

//multiple
int Number_of_Set_Bits_In(uint32_t ins){
    int i = 0;
    int res = 0;
    for(int i = 0; i < 16; i++){
        res = res + get_bit(ins,i);
    }
    return res;
}

//A4-36
int instruction_ldm1(arm_core p, uint32_t ins){
    return UNDEFINED_INSTRUCTION;
    
}
int instruction_stm1(arm_core p, uint32_t ins){
    return UNDEFINED_INSTRUCTION;
    
}
int instruction_ldm2(arm_core p, uint32_t ins){
    return UNDEFINED_INSTRUCTION;
}
int instruction_stm2(arm_core p, uint32_t ins){
    return UNDEFINED_INSTRUCTION;
}
int instruction_ldm3(arm_core p, uint32_t ins){
    return UNDEFINED_INSTRUCTION;
}


//DA IA DB IB  A3-26 p134
int arm_load_store_multiple(arm_core p, uint32_t ins) {
    int type = get_bits(ins,27,25);
    int bit_15 = get_bit(ins,15);

    if(type == 4){
        if(bitL(ins) && !bitB(ins)){
            instruction_ldm1(p,ins);
        }else if(bitL(ins) && bitB(ins) && !bitW(ins) && !bit_15){
            instruction_ldm2(p,ins);
        }else if(bitL(ins) && bitB(ins) && bit_15){
            instruction_ldm3(p,ins);
        }else if(!bitL(ins) && !bitB(ins)){
            instruction_stm1(p,ins);
        }else if(!bitL(ins) && bitB(ins) && !bitW(ins)){
            instruction_stm2(p,ins);
        }else return UNDEFINED_INSTRUCTION;
    }
    return 0;
}

int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    /* Not implemented */
    return UNDEFINED_INSTRUCTION;
}
