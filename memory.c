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
#include <stdlib.h>
#include "memory.h"
#include "util.h"

struct memory_data {
    int size; //le nombre d'octets qu'on peut avoir dans la memoire

    uint8_t* memory_array; //tableau des data(octets)
};

memory memory_create(size_t size) {
    memory mem = malloc(sizeof(struct memory_data));
    if(mem){
        mem->size=size;
        mem->memory_array=malloc(sizeof(uint8_t)*size);
        if(mem->memory_array){
            return mem;
        }
        free(mem);
    }
    return NULL;
}

size_t memory_get_size(memory mem) {
    if(mem){
        return mem->size;
    }
    return -1;
}

void memory_destroy(memory mem) {
    if(mem){
        free(mem->memory_array);
        free(mem);
    }
}

int memory_read_byte(memory mem, uint32_t address, uint8_t *value) {
    if(mem){
        if(address<mem->size){
            *value = *(mem->memory_array+address);
            return 0; 
        }

    }
    return -1;
}

int memory_read_half(memory mem, uint32_t address, uint16_t *value, uint8_t be) {
        if(mem){
            if(address<mem->size){
                uint8_t b1,b2;
                memory_read_byte(mem, address, &b1);
                memory_read_byte(mem, address + sizeof(uint8_t), &b2);
                if (be){
                    value[0]=b2;
                    value[1]=b1;
                }
                else{
                    uint8_t* val = malloc(sizeof(uint8_t)*2);
                    *(val)=b1;
                    *(val+1)=b2;
                    *value = *val;
                }
                return 0; 
            }

    }
    return -1;
}

int memory_read_word(memory mem, uint32_t address, uint32_t *value, uint8_t be) {
            if(mem){
                if(address<mem->size){
                    uint8_t b1,b2,b3,b4;
                    memory_read_byte(mem, address, &b1);
                    memory_read_byte(mem, address + sizeof(uint8_t), &b2);
                    memory_read_byte(mem, address + sizeof(uint8_t)*2, &b3);
                    memory_read_byte(mem, address + sizeof(uint8_t)*3, &b4);
                    if(be){
                        value[0]=b4;
                        value[1]=b3;
                        value[2]=b2;
                        value[3]=b1;
                    }
                    else{
                        uint8_t* val = malloc(sizeof(uint8_t)*4);
                        *(val)=b1;
                        *(val+1)=b2;
                        *(val+2)=b3;
                        *(val+3)=b4;
                        *value = *val;
                    }
                    return 0;
                }
            }
    return -1;
}


int memory_write_byte(memory mem, uint32_t address, uint8_t value) {
    if(mem){
        if(address<mem->size){
            *(mem->memory_array+address)=value;
            return 0; 
        }

    }
    return -1;
}

int memory_write_half(memory mem, uint32_t address, uint16_t value, uint8_t be) {
        if(mem){
        if(address<mem->size){
            uint8_t b1 = *(&value);
            uint8_t b2 = *(&value+1);
            if(be){
                memory_write_byte(mem,address,b2);
                memory_write_byte(mem,address + sizeof(uint8_t),b1);
            }
            else{
                memory_write_byte(mem,address,b1);
                memory_write_byte(mem,address + sizeof(uint8_t),b2);
            }
            return 0;
        }

    }
    return -1;
}

int memory_write_word(memory mem, uint32_t address, uint32_t value, uint8_t be) {
        if(mem){
            if(address<mem->size){
                uint8_t b1 = *(&value);
                uint8_t b2 = *(&value+1);
                uint8_t b3 = *(&value+2);
                uint8_t b4 = *(&value+3);
                if(be){
                    memory_write_byte(mem,address,b4);
                    memory_write_byte(mem,address + sizeof(uint8_t),b3);
                    memory_write_byte(mem,address + sizeof(uint8_t)*2,b2);
                    memory_write_byte(mem,address + sizeof(uint8_t)*3,b1);
                }
                else{
                    memory_write_byte(mem,address,b1);
                    memory_write_byte(mem,address + sizeof(uint8_t),b2);
                    memory_write_byte(mem,address + sizeof(uint8_t)*2,b3);
                    memory_write_byte(mem,address + sizeof(uint8_t)*3,b4);
                }
            }
            return 0;
        }
    return -1;
}