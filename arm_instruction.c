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
#include "arm_instruction.h"
#include "arm_exception.h"
#include "arm_data_processing.h"
#include "arm_load_store.h"
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"

int arm_condition(arm_core p ,uint32_t ins){
    uint32_t cpsr = arm_read_cpsr(p);
    int n = get_bit(cpsr,31);
    int z = get_bit(cpsr,30);
    int c = get_bit(cpsr,29);
    int v = get_bit(cpsr,28);

    switch(get_bits(ins,31,28)){
        case 0b0000://EQ
            return z;
        case 0b0001://NE
            return !z;
        case 0b0010://CS/HS
            return c;
        case 0b0011://CC/LO
            return !c;
        case 0b0100://MI
            return n;
        case 0b0101://PL
            return !n;
        case 0b0110://VS
            return v;
        case 0b0111://VC
            return !v;
        case 0b1000://HI
            return c && (!z);
        case 0b1001://LS
            return (!c) || z;
        case 0b1010://GE
            return n == v;
        case 0b1011://LT
            return n != v;
        case 0b1100://GT
            return !z && (n == v);
        case 0b1101://LE
            return z || (n !=v);
        case 0b1110://AL
            return 1;
        default://condition code 0b1111
            return 0; //UNDEFINED_INSTRUCTION
    }
}

static int arm_execute_instruction(arm_core p) {
    uint32_t ins;
    uint8_t instype;
    int res = arm_fetch(p,&ins);
    instype = get_bits(ins,27,25);
    
    int bit_24 = get_bit(ins,24);
    int bit_23 = get_bit(ins,23);
    int bit_21 = get_bit(ins,21);
    int bit_20 = get_bit(ins,20);
    int bit_7 = get_bit(ins,7);
    int bit_4 = get_bit(ins,4);

    

    if(res != 0){
        return -1;
    }
    if(arm_condition(p, ins)){
        switch (instype){
            case 0: //Data processing  OR  MSR
                if(bit_24 == 1 && bit_23 == 0 && bit_20 == 0 ){//MSR et MRS
                    return arm_miscellaneous(p,ins);
                }else if(bit_7 == 1 && bit_4 == 1){//Multiplies/Extra load/stores
                    return arm_load_store(p,ins);
                }else{//bit7=0 bit4 = 1  data processing register shift
                    return arm_data_processing_shift(p,ins);
                }
            case 1:
                return arm_data_processing_immediate_msr(p,ins);
            case 2:
                return arm_load_store(p,ins);
            case 3:
                return arm_load_store(p,ins);
            case 4:
                return arm_load_store_multiple(p,ins);
            case 5:
                return arm_branch(p,ins);
            case 6:
                return arm_coprocessor_load_store(p,ins);
            case 7:
                return arm_coprocessor_others_swi(p,ins);
        }
    }
    return 0;
}

int arm_step(arm_core p) {
    int result;

    result = arm_execute_instruction(p);
    if (result) {
        return arm_exception(p, result);
    }
    return result;
}
