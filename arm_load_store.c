/*
Armator - simulateur de jeu d'instruction ARMv5T à but pédagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique Générale GNU publiée par la Free Software
Foundation (version 2 ou bien toute autre version ultérieure choisie par vous).

Ce programme est distribué car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but spécifique. Reportez-vous à la
Licence Publique Générale GNU pour plus de détails.

Vous devez avoir reçu une copie de la Licence Publique Générale GNU en même
temps que ce programme ; si ce n'est pas le cas, écrivez à la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
États-Unis.

Contact: Guillaume.Huard@imag.fr
	 Bâtiment IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'Hères
*/
#include "arm_load_store.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "util.h"
#include "debug.h"
#include <assert.h>

int32_t signExtend8to32(uint8_t value) {
    // Check the sign bit of the 8-bit value
    int32_t signBit = (value & 0x80) ? 0xFFFFFF00 : 0x00000000;

    // Perform sign extension
    return signBit | value;
}

int32_t signExtend16to32(uint16_t value) {
    // Check the sign bit of the 16-bit value
    int32_t signBit = (value & 0x8000) ? 0xFFFF0000 : 0x00000000;

    // Perform sign extension
    return signBit | value;
}

uint32_t shift_index_calculator(uint32_t Rm,uint8_t shift_imm,uint8_t shift,uint8_t C_flag){
   uint32_t index=0;
   switch(shift){
       case 0b00:/* LSL */
           index = Rm << shift_imm;
           break;


       case 0b01: /* LSR */
           if (shift_imm ==0 ){index =0;}
           else{
               index = Rm >> shift_imm;
           }
           break;
      
       case 0b10:  /* ASR */
           if (shift_imm ==0 ){
               if (get_bit(Rm,31)==1){
                   index = 0xFFFFFFFF;
               }
               else{
                   index = 0;
               }
           }
           else{
               index = Rm >> shift_imm;
           }
           break;
       case 0b11: /* ROR or RRX */
           if (shift_imm ==0){ /* RRX*/
               index = (C_flag >> 31) | (Rm >> 1);
           }
           else{ /*ROR*/


                   // Ensure the shift_imm value is within the range of 0 to numBits - 1
                   shift_imm = shift_imm % 32;


                   // Perform the right rotation
                   index = (Rm >> shift_imm) | (Rm << (32 - shift_imm));
           }
   }
   return index;
}


/*---------------------------------ADDRESS MODES-------------------------------*/

/*________________________________MODE 3_________________________*/

uint32_t calculate_address_mode3(arm_core p, uint32_t ins){
    uint32_t address;
    uint32_t Rn = arm_read_register(p, get_bits(ins, 19, 16));
    int Rn_num = get_bits(ins, 19, 16);

 //-------------------------IMMEDIATE CASE-----------------------------------
    if(get_bit(ins, 22)){
        uint8_t immedH = get_bits(ins, 11, 8);
        uint8_t immedL = get_bits(ins, 3, 0);
        uint8_t offset_8 = (immedH << 4) | immedL;
        // A5.3.2 Miscellaneous Loads and Stores - Immediate offset Page A5-35

        if(bitP(ins)){
            
            if(bitU(ins)){
                address = Rn + offset_8;
            }
            else{
                address = Rn - offset_8;
            }

            // A5.3.4 Miscellaneous Loads and Stores - Immediate pre-indexed Page A5-37
            
            if(bitW(ins)){
                arm_write_register(p, Rn_num, address);
            }
        }
        // A5.3.6 Miscellaneous Loads and Stores - Immediate post-indexed Page A5-39
        else if(!bitP(ins) && !bitW(ins)){
            address = Rn;
            if(bitU(ins)){
                arm_write_register(p, Rn_num, Rn + offset_8);
            }
            else{
                arm_write_register(p, Rn_num, Rn - offset_8);
            }
        }
    }
 //-------------REGISTER OFFSET CASE------------------------------------------------
    else{
        uint32_t Rm = arm_read_register(p, get_bits(ins, 3, 0));

        //A5.3.3 Miscellaneous Loads and Stores - Register offset Page A5-36

        if(bitP(ins)){
            
            if(bitU(ins)){
                address = Rn + Rm;
            }
            else{
                address = Rn - Rm;
            }

            //A5.3.5 Miscellaneous Loads and Stores - Register pre-indexed Page A5-38

            if(bitW(ins)){
                arm_write_register(p, Rn_num, address);
            }
        }

        // A5.3.7 Miscellaneous Loads and Stores - Register post-indexed Page A5-40

        else if(!bitP(ins) && !bitW(ins)){
            address = Rn;
            if(bitU(ins)){
                arm_write_register(p, Rn_num, Rn + Rm);
            }
            else{
                arm_write_register(p, Rn_num, Rn - Rm);
            }
        }
    }
    return address;
}

/*________________________________MODE 2_____________________________*/

uint32_t calculate_address(arm_core p, uint32_t ins){
    uint32_t address;
    uint32_t Rn = arm_read_register(p, get_bits(ins, 19, 16));
    int Rn_num = get_bits(ins, 19, 16);
    int bit25 = get_bit(ins, 25);
//------------------------IMMEDIATE CASE--------------------------------------------------------------
    
    if(!bit25){  
        uint16_t offset_12 = get_bits(ins, 11, 0);
     /*A5.2.2 Load and Store Word or Unsigned Byte - Immediate offset Page A5-20*/       
        
        if (bitP(ins)){ 
            if(bitU(ins)){
                address = Rn + offset_12;
            }
            else{
                address = Rn - offset_12;
            }

            /*A5.2.5 Load and Store Word or Unsigned Byte - Immediate pre-indexed Page A5-24*/

            if(bitW(ins)){
                arm_write_register(p, Rn_num, address);
            }

        }


     /*A5.2.8 Load and Store Word or Unsigned Byte - Immediate post-indexed Page A5-28*/

        else if (!bitP(ins) && !bitW(ins)){
            address = Rn;
            
            if(bitU(ins)){
                Rn = Rn + offset_12;
                arm_write_register(p, Rn_num, Rn);
            }
            else{
                Rn = Rn - offset_12;
                arm_write_register(p, Rn_num, Rn);
            }
        }


     /*UNTREATED CASES*/
        
        else{
            //instruction is not handeled
            return -1;
        }
    
    } 

//-------------REGISTER OFFSET CASE-------------------------------------------------
    
    else if (get_bit(ins, 25) && !get_bits(ins, 11, 4)){
        uint32_t Rm = arm_read_register(p, get_bits(ins, 4, 0));

        /*A5.2.3 Load and Store Word or Unsigned Byte - Register offset Page A5-21*/

        if (bitP(ins)){
            if (bitU(ins)){
                address = Rn + Rm;
            }
            else{
                address = Rn - Rm;
            }

            /*A5.2.6 Load and Store Word or Unsigned Byte - Register pre-indexed Page A5-25*/
            
            if(bitW(ins)){
                arm_write_register(p, Rn_num, address); 
            }
        }

        /*A5.2.9 Load and Store Word or Unsigned Byte - Register post-indexed Page A5-30*/

        else if (!bitP(ins) && !bitW(ins)){
            address = Rn;
            if(bitU(ins)){
                arm_write_register(p, Rn_num, Rn + Rm);
            }
            else{
                arm_write_register(p, Rn_num, Rn - Rm);
            }
        }
        
        /*UNTREATED CASES*/
        else{
            //instruction is not handeled
            return -1;
        }

    }

//-------------SCALED REGISTER CASE-------------------------------------------------

    else if(get_bit(ins, 25) && get_bits(ins, 11, 4)!=0){
        uint32_t Rm = arm_read_register(p, get_bits(ins, 3, 0));
        uint8_t shift_imm = get_bits(ins, 11, 7);
        uint8_t shift = get_bits(ins, 6,5);
        int C_flag =get_bit(arm_read_cpsr(p), 29); 
        uint32_t index = shift_index_calculator(Rm,shift_imm, shift, C_flag);
     /*A5.2.4 Load and Store Word or Unsigned Byte - Scaled register offset Page A5-22*/
     
        if(bitP(ins)){
            if(bitU(ins)){
                address = Rn + index;
            }
            else{
                address = Rn - index;
            }

            /*A5.2.7 Load and Store Word or Unsigned Byte - Scaled register pre-indexed Page A5-26*/

            if(bitW(ins)){
               arm_write_register(p, Rn_num, address); 
            }
        }

     /*A5.2.10 Load and Store Word or Unsigned Byte - Scaled register post-indexed Page A5-35*/
        
        else if(!bitP(ins) && !bitW(ins)){
            address = Rn;
            if(bitU(ins)){
                arm_write_register(p, Rn_num, Rn + index);
            }
            else{
                arm_write_register(p, Rn_num, Rn - index);
            }
        }

        /*UNTREATED CASES*/
        
        else{
            //instruction is not handeled
            return -1;
        }
    }
    else{
            //instruction is not handeled
            return -1;
    }
    return address;
}

int arm_load_store(arm_core p, uint32_t ins) {
    uint32_t address = calculate_address(p, ins);
    if (address == -1 ){
        return UNDEFINED_INSTRUCTION;
    }
    uint8_t Rd_num = get_bits(ins, 15, 12);
   
//-------------ADDRESSING MODE 2----------------------------------------
    
    if (get_bit(ins, 26)){
        //INSTRUCTION LDR
        if(!bitB(ins)){
            if(bitL(ins)){ 
                uint32_t val;
                arm_read_word(p, address, &val);
                arm_write_register(p, Rd_num, val);
                return 0;
            }
        //INSTRUCTION STR
            else{
                uint32_t val = arm_read_register(p, Rd_num);
                arm_write_word(p, address, val);
                return 0;
            }
        }
        else {
        //INSTRUCTION LDRB
            if(bitL(ins)){
                uint8_t val;
                arm_read_byte(p, address, &val);
                arm_write_register(p, Rd_num, val);
                return 0;
            }
        //INSTRUCTION STRB
            else{
                uint32_t Rd = arm_read_register(p, Rd_num);
                uint8_t val = get_bits(Rd, 7, 0);
                arm_write_byte(p, address, val);
                return 0;
            }
        }
    }


//----------ADDRESSING MODE 3------------------------------------------
    
    else{
        //INSTRUCTIONS STRH
        if(!bitL(ins)){
            if(Rd_num == 15){
                return UNDEFINED_INSTRUCTION; //UNPREDICTABLE
            }
            address = calculate_address_mode3(p, ins);
            uint16_t val = get_bits(arm_read_register(p, Rd_num), 15, 0);
            arm_write_half(p, address, val);
            return 0;
        }
        //INSTRUCTIONS LDRH LDRSB LDRSH
        switch (get_bits(ins, 7, 4)){
        
        case 0b1011: // INSTRUCTION LDRH
            if(Rd_num==15){
                return UNDEFINED_INSTRUCTION; // UNPREDICTABLE
            }
            uint16_t val;
            arm_read_half(p, address, &val);
            arm_write_register(p, Rd_num, val);
            return 0;
        break;

        case 0b1101: //INSTRUCTION LDRSB
            if(Rd_num==15){
                return UNDEFINED_INSTRUCTION; // UNPREDICTABLE
            }
            uint8_t value;  
            arm_read_byte(p, address, &value);
            arm_write_register(p, Rd_num,signExtend8to32(value));
            return 0;
        break;
        
        case 0b1111: //INSTRUCTION LDRSH
            if(Rd_num==15){
                return UNDEFINED_INSTRUCTION; // UNPREDICTABLE
            }
            uint16_t vals;
            arm_read_half(p, address, &vals);
            arm_write_register(p, Rd_num, signExtend16to32(vals));
            return 0;
        break;
        
        default:
            return UNDEFINED_INSTRUCTION;
        break;
        }
    }
    
    return UNDEFINED_INSTRUCTION;
}


//_______________________MULTIPLE BY ZHANG__________________________
//multiple
int Number_of_Set_Bits_In(uint32_t ins){
    int i = 0;
    int res = 0;
    for(i = 0; i < 16; i++){
        res = res + get_bit(ins,i);
    }
    return res;
}
//DA IA DB IB  A5.4 p481
void addr_ls_multiple(arm_core p,uint32_t ins, uint32_t *start, uint32_t *end){
    if(get_bit(ins,24) && get_bit(ins,23)){//Increment before
        *start = arm_read_register(p,bits_rn(ins)) + 4;
        *end = arm_read_register(p,bits_rn(ins)) + 4 * Number_of_Set_Bits_In(ins);
        if(bitW(ins)){
            arm_write_register(p,bits_rn(ins),arm_read_register(p,bits_rn(ins)) + Number_of_Set_Bits_In(ins) *4);
        }
    }else if(get_bit(ins,24) && !get_bit(ins,23)){//Decrement before
        *start = arm_read_register(p,bits_rn(ins)) - 4*Number_of_Set_Bits_In(ins);
        *end = arm_read_register(p,bits_rn(ins)) - 4;
        if(bitW(ins)){
            arm_write_register(p,bits_rn(ins),arm_read_register(p,bits_rn(ins)) - Number_of_Set_Bits_In(ins) *4);
        }
    }else if(!get_bit(ins,24) && get_bit(ins,23)){//Increment after
        *start = arm_read_register(p,bits_rn(ins));
        *end = arm_read_register(p,bits_rn(ins)) + 4*Number_of_Set_Bits_In(ins) -4;
        if(bitW(ins)){
            arm_write_register(p,bits_rn(ins),arm_read_register(p,bits_rn(ins)) + Number_of_Set_Bits_In(ins) *4);
        }
    }else if(!get_bit(ins,24) && !get_bit(ins,23)){//Decrement after
        *start = arm_read_register(p,bits_rn(ins)) - 4*Number_of_Set_Bits_In(ins) + 4;
        *end = arm_read_register(p,bits_rn(ins));
        if(bitW(ins)){
            arm_write_register(p,bits_rn(ins),arm_read_register(p,bits_rn(ins)) - Number_of_Set_Bits_In(ins) *4);
        }
    }
}
//A4-36 p186
//loads a non-empty subset of the general-purpose registers to sequential memory locations.
int instruction_ldm1(arm_core p, uint32_t ins){
    uint32_t start,end;
    uint32_t address,word;
    addr_ls_multiple(p,ins,&start,&end);
    address = start;
    int i;
    for(i = 0;i<15;i++){
        if(get_bit(ins,i)==1){
            arm_read_word(p,address,&word);
            arm_write_register(p,i,word);
            address +=4;
        }
    }
    if(get_bit(ins,15)==1){
        arm_read_word(p,address,&word);
        arm_write_register(p,15,word & 0xFFFFFFFE);
         //T bit = value[0]
        uint8_t t_bit = get_bit(word,0);
        if(t_bit == 0){
            arm_write_cpsr(p,clr_bit(arm_read_cpsr(p),5));
        }else
            arm_write_cpsr(p,set_bit(arm_read_spsr(p),5));

        address +=4;
    }
    assert(end == (address -4));
    return 0;
}
//A4-189 p339
//stores a non-empty subset of the general-purpose registers to sequential memory locations.
int instruction_stm1(arm_core p, uint32_t ins){
    uint32_t start,end;
    uint32_t address;
    addr_ls_multiple(p,ins,&start,&end);
    address = start;
    int i ;
    for(i = 0;i<16;i++){
        if(get_bit(ins,i)==1){
            arm_write_word(p,address,arm_read_register(p,i));
            address +=4;
        }
    }
    assert(end == (address -4));
    return 0;
}

int arm_load_store_multiple(arm_core p, uint32_t ins) {
    int bit_15 = get_bit(ins,15);

    if(bitL(ins) && !bitB(ins)){
        instruction_ldm1(p,ins);
    }else if(!bitL(ins) && !bitB(ins)){
        instruction_stm1(p,ins);
    }
    else{
    return UNDEFINED_INSTRUCTION;
    }
    return 0;
}

int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    /* Not implemented */
    return UNDEFINED_INSTRUCTION;
}