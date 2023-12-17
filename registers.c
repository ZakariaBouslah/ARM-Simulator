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
#include "registers.h"
#include "arm_constants.h"
#include <stdlib.h>

struct registers_data {
    uint32_t reg_data;
};

registers registers_create() {
    registers r = calloc(37,sizeof(struct registers_data));
    
    return r;
}

void registers_destroy(registers r) {
    free(r);
}

uint8_t registers_get_mode(registers r) {
    uint8_t mode = r[31].reg_data & 0x1f;
    return mode;
}

static int registers_mode_has_spsr(registers r, uint8_t mode) {
    if(mode == USR || mode == SYS ){
        return 0;
    }else{
        return 1;
    }
}

int registers_current_mode_has_spsr(registers r) {
    return registers_mode_has_spsr(r, registers_get_mode(r));
}

int registers_in_a_privileged_mode(registers r) {
    uint8_t mode = registers_get_mode(r);
    if(mode == USR){
        return 0;
    }
    return 1;
}

uint32_t registers_read(registers r, uint8_t reg, uint8_t mode) {
    uint32_t value = 0;
    if(reg<8){
        value = r[reg].reg_data;
    }else if(reg<13){
        if(mode == FIQ){
            value = r[reg+16].reg_data;
        }else{
            value = r[reg].reg_data;
        }
    }else if(reg<15){
        switch(mode){
            //17 fiq
            case 0b10001:
                value = r[reg+16].reg_data;
                break;
            //18 irq
            case 0b10010:
                value = r[reg+9].reg_data;
                break;
            //19 svc
            case 0b10011:
                value = r[reg+3].reg_data;
                break;
            //23 abt
            case 0b10111:
                value = r[reg+5].reg_data;
                break;
            //27 und
            case 0b11011:
                value = r[reg+7].reg_data;
                break;    
            default:
                value = r[reg].reg_data;
                break;
        }
    }else{
        value = r[reg].reg_data;
    }
    return value;
}

uint32_t registers_read_cpsr(registers r) {
    uint32_t value = r[31].reg_data;
    return value;
}

uint32_t registers_read_spsr(registers r, uint8_t mode) {
    uint32_t value = 0;
    switch (mode){
        //17 fiq
        case 0b10001:
            value = r[36].reg_data;
            break;
        //18 irq
        case 0b10010:
            value = r[35].reg_data;
            break;
        //19 svc
        case 0b10011:
            value = r[32].reg_data;
            break;
        //23 abt
        case 0b10111:
            value = r[33].reg_data;
            break;
        //27 und
        case 0b11011:
            value = r[34].reg_data;
            break;    
        default:
            value = 0;
            break;
    }
    return value;
}

void registers_write(registers r, uint8_t reg, uint8_t mode, uint32_t value) {
    if(reg<8){
        r[reg].reg_data = value;
    }else if(reg<13){
        if(mode == FIQ){
            r[reg+16].reg_data = value;
        }else{
            r[reg].reg_data = value;
        }
    }else if(r<15){
        switch(mode){
            //17 fiq
            case 0b10001:
                r[reg+16].reg_data = value;
                break;
            //18 irq
            case 0b10010:
                r[reg+9].reg_data = value;
                break;
            //19 svc
            case 0b10011:
                r[reg+3].reg_data = value;
                break;
            //23 abt
            case 0b10111:
                r[reg+5].reg_data = value;
                break;
            //27 und
            case 0b11011:
                r[reg+7].reg_data = value;
                break;    
            default:
                r[reg].reg_data = value;
                break;
        }
    }else{
        r[reg].reg_data = value;
    }
}

void registers_write_cpsr(registers r, uint32_t value) {
    r[31].reg_data = value;
}

void registers_write_spsr(registers r, uint8_t mode, uint32_t value) {
    switch (mode){
        //17 fiq
        case 0b10001:
            r[36].reg_data = value;
            break;
        //18 irq
        case 0b10010:
            r[35].reg_data = value;
            break;
        //19 svc
        case 0b10011:
            r[32].reg_data = value;
            break;
        //23 abt
        case 0b10111:
            r[33].reg_data = value;
            break;
        //27 und
        case 0b11011:
            r[34].reg_data = value;
            break;    
        default:
            break;
    }
}
