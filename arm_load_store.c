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
int is_immediate_offset(uint32_t ins){//offset +-12
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
uint32_t determiner_rn(arm_core p,uint32_t ins,uint32_t addr){//apres visiter les donne A5-24 address written back to teh base register Rn
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
int instruction_ldr(arm_core p, uint32_t ins){//A4-43
    uint8_t rd = get_bits(ins,15,12);
    // uint32_t offset = get_bits(ins,11,0);
    uint32_t address = determiner_addr(p,ins);
    uint32_t word;
    if(arm_read_word(p,address,&word)==0){
        if(rd == 15){
            arm_write_register(p,15,word & 0xFFFFFFFE);
            uint8_t t_bit = get_bit(word,0);
            if(t_bit == 0){
                arm_write_cpsr(p,clr_bit(arm_read_cpsr(p),5));
            }else
                arm_write_cpsr(p,set_bit(arm_read_spsr(p),5));
        }else{
            arm_write_register(p,bits_rd(ins),word);
        }
    }
    determiner_rn(p,ins,address);
    return 0;
}
int instruction_ldrb(arm_core p, uint32_t ins){
    
}
int instruction_ldrh(arm_core p, uint32_t ins){
    
}
int instruction_str(arm_core p, uint32_t ins){
    
}
int instruction_strb(arm_core p, uint32_t ins){
    
}
int instruction_strh(arm_core p, uint32_t ins){
    
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
int instruction_ldm1(arm_core p, uint32_t ins){
    
}
int instruction_stm1(arm_core p, uint32_t ins){
    
}
int instruction_ldm2(arm_core p, uint32_t ins){
    
}
int instruction_stm2(arm_core p, uint32_t ins){
    
}
int instruction_ldm3(arm_core p, uint32_t ins){
    
}

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
