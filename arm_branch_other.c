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
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"
#include <debug.h>
#include <stdlib.h>

int32_t sign_extend_30(uint32_t value) {
    if (value & (1 << 23)) {
        value |= 0xFC000000; 
    } else {
        value &= 0x00FFFFFF; 
    }
    return (int32_t)value; 
}

int arm_branch(arm_core p, uint32_t ins) {
    uint32_t PC = arm_read_register(p, 15);
    int32_t offset;
    uint32_t target_address;
    int is_BL = get_bit(ins, 24);
    int32_t signed_immed_24 = get_bits(ins, 23, 0);
    offset = sign_extend_30(signed_immed_24) << 2; 

    target_address = PC + 8 + offset; // vers l'adresse de la deuxième instruction après l'instruction en cours

    if (is_BL) {
        uint32_t LR = PC + 4; // LR est l'adresse de la prochaine instruction de l'instruction en cours
        arm_write_register(p, 14, LR);
    }
    arm_write_register(p, 15, target_address);
    return 0;
}

int arm_coprocessor_others_swi(arm_core p, uint32_t ins) {
    if (get_bit(ins, 24)) {
        return SOFTWARE_INTERRUPT;
    }
    return UNDEFINED_INSTRUCTION;
}

int arm_miscellaneous(arm_core p, uint32_t ins) {
    //Vérifiez si l'instruction est une instruction CLZ
    if ((ins & 0x0FF000F0) == 0x016F0F10) { 
        uint32_t Rm = get_bits(ins, 3, 0);
        uint32_t Rd = get_bits(ins, 15, 12);
        uint32_t value = arm_read_register(p, Rm); 

        uint32_t leading_zeros = value ? __builtin_clz(value) : 32;

        arm_write_register(p, Rd, leading_zeros);
        return 0;
    }
    return UNDEFINED_INSTRUCTION;}
//com