/*
Armator - simulateur de jeu d'instruction ARMv5T   but p dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G n rale GNU publi e par la Free Software
Foundation (version 2 ou bien toute autre version ult rieure choisie par vous).

Ce programme est distribu  car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp cifique. Reportez-vous   la
Licence Publique G n rale GNU pour plus de d tails.

Vous devez avoir re u une copie de la Licence Publique G n rale GNU en m me
temps que ce programme ; si ce n'est pas le cas,  crivez   la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
 tats-Unis.

Contact: Guillaume.Huard@imag.fr
	 B timent IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'H res
*/
#ifndef __ARM_INSTRUCTION_H__
#define __ARM_INSTRUCTION_H__
#include "arm_core.h"

#define CATEGORY_MASK 0x0FF00000
#define LOAD_STORE_HALF_MASK 0x0E0000F0
#define CONDITION_MASK 0xF0000000
#define INSTRUCTION_MASK 0x0C000000
#define LOAD_BIT 0x00100000
#define BYTE_HALFWORD_BIT 0x00400000
#define IMMEDIATE_BIT 0x02000000
#define OPCODE_MASK 0x0FC00000
#define ADRESSE_MASK 0x000F0000
#define DESTINATION_MASK 0x0000F000
#define RM_MASK 0x0000000F
#define SHIFT_TYPE_MASK 0x00000060
#define SHIFT_IMM_MASK 0x00000F80
#define OFFSET_MASK 0x00000FFF

#define IS_MSR_INSTRUCTION(instruction) \
    ((((instruction) & CATEGORY_MASK) == 0x01000000) || \
     (((instruction) & CATEGORY_MASK) == 0x01400000))

#define IS_DATA_PROCESSING_INSTRUCTION(instruction) \
    ((((instruction) & CATEGORY_MASK) >> 24) == 0x0 || \
     (((instruction) & CATEGORY_MASK) >> 24) == 0x1)

#define IS_BRANCH_INSTRUCTION(instruction) \
    ((((instruction) & CATEGORY_MASK) >> 24) == 0xA)

#define IS_LOAD_STORE_INSTRUCTION(instruction) \
    ((((instruction) & CATEGORY_MASK) >> 24) == 0x2 || \
     (((instruction) & CATEGORY_MASK) >> 24) == 0x3)

#define IS_LOAD_STORE_MULTIPLE_INSTRUCTION(instruction) \
    ((((instruction) & CATEGORY_MASK) >> 24) == 0x4)

#define IS_LOAD_STORE_HALF_INSTRUCTION(instruction) \
    (((instruction) & LOAD_STORE_HALF_MASK) == 0xB0)


#define IS_LDR_INSTRUCTION(instruction) \
    (((instruction) & INSTRUCTION_MASK) == 0x04000000 && \
     ((instruction) & LOAD_BIT) && \
     !((instruction) & BYTE_HALFWORD_BIT))

#define IS_LDRB_INSTRUCTION(instruction) \
    (((instruction) & INSTRUCTION_MASK) == 0x04000000 && \
     ((instruction) & LOAD_BIT) && \
     ((instruction) & BYTE_HALFWORD_BIT))

#define IS_LDRH_INSTRUCTION(instruction) \
    (((instruction) & INSTRUCTION_MASK) == 0x01000000 && \
     ((instruction) & LOAD_BIT) && \
     ((instruction) & IMMEDIATE_BIT) && \
     ((instruction) & LOAD_STORE_HALF_MASK) == 0xB0)

#define IS_STR_INSTRUCTION(instruction) \
    (((instruction) & INSTRUCTION_MASK) == 0x04000000 && \
     !((instruction) & LOAD_BIT) && \
     !((instruction) & BYTE_HALFWORD_BIT))

#define IS_STRB_INSTRUCTION(instruction) \
    (((instruction) & INSTRUCTION_MASK) == 0x04000000 && \
     !((instruction) & LOAD_BIT) && \
     ((instruction) & BYTE_HALFWORD_BIT))

#define IS_STRH_INSTRUCTION(instruction) \
    (((instruction) & INSTRUCTION_MASK) == 0x00000000 && \
     !((instruction) & LOAD_BIT) && \
     ((instruction) & IMMEDIATE_BIT) && \
     ((instruction) & LOAD_STORE_HALF_MASK) == 0xB0)

#define GET_CONDITION_CODE(instruction) \
    (((instruction) & CONDITION_MASK) >> 28)

#define GET_OPCODE(instruction) \
    (((instruction) & OPCODE_MASK) >> 21)

#define GET_ADRESSE(instruction) \
    (((instruction) & ADRESSE_MASK) >> 16)

#define GET_DESTINATION(instruction) \
    (((instruction) & DESTINATION_MASK) >> 12)

#define GET_RM(instruction) \
    ((instruction) & RM_MASK)

#define GET_SHIFT_TYPE(instruction) \
    (((instruction) & SHIFT_TYPE_MASK) >> 5)

#define GET_SHIFT_IMM(instruction) \
    (((instruction) & SHIFT_IMM_MASK) >> 7)

#define GET_OFFSET(instruction) \
    ((instruction) & OFFSET_MASK)



int arm_step(arm_core p);

#endif