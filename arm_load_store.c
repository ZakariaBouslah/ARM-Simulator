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


int arm_load_store(arm_core p, uint32_t ins) {
    //variables utilisés
    int adr = arm_read_register(p, (unit8_t)(ins & 0x000F0000)>>16);
    int off = 0;
    uint8_t rdest = ((ins & 0x0000F000)>>12);
    int i = 0x00400000;
    int h = 1;
	if(ins & 0x04000000){
		i = 0x02000000;
        h = 0;
	}
    int op = 0;
    int val;


    //CALCUL DE L'OFFSET
    if(!(i & ins) && !h){               //word ou byte avec valeur immédiate
        off = ins & 0x00000FFF;
    }
    else if(!(i & ins)){                //half avec valeur immédiate
        off = (ins & 0X0000000F) + ((ins & 0x00000F00)>>4);
    }
    else if(!h){                        //word ou byte avec registre comme offset
        off = arm_read_register(p, (unit8_t)(ins & 0x0F));
        int sh = (ins & 0x00000060)>>4;
        op = (ins & 0x00000F80)>>7;
        if(sh == 0){off = lsl(off, op);}        //choix du shift demandé pour la valeur du registre d'offset
        else if(sh == 1){off = lsr(off, op);}
        else if(sh == 2){off = asr(off, op);}
        else if(op == 0){off = rxr(off);}
        else{op = ror(off, op);}
    }
    else{                                //half avec registre comme offset
        off = arm_read_register(p, (unit8_t)(ins & 0x0F));
    }
    



    if(ins & 0x00800000){adr = adr + off;}
    else{adr = adr - off;}              //calcul dudécallage avec l'offset(bit U)


    if(((ins & 0x01000000)>>24) == ((ins & 0x00200000)>>21)){   //mise à jour du registre adresse
        arm_write_register(p, (ins & 0x000F0000)>>16, adr);
    }


    int val;
    if(ins & 0x00100000){               //ldr
        if(h){
            (uint16_t)(val);
            val = arm_read_half(p, adr, &val);
        }
        else if(ins & 0x00400000){
            (uint8_t)(val);
            val = arm_read_byte(p, adr, &val);
        }
        else{
            (uint32_t)(val);
            val = arm_read_word(p, adr, &val);
        }
        arm_write_register(p, rdest, val);
        return 0;
    }

    else{                               //str
    val = arm_read_register(p, rdest);
        if(h){
            (uint16_t)(val);
            arm_write_half(p, adr, val);
            return 0;
        }
        else if(ins & 0x00400000){
            (uint8_t)(val);
            arm_write_byte(p, adr, val);
            return 0;
        }
        else{
            (uint32_t)(val);
            arm_write_word(p, adr, val);
            return 0;
        }
    }

    return UNDEFINED_INSTRUCTION;
}

int arm_load_store_multiple(arm_core p, uint32_t ins) {
    //récupération des parties utiles de l'instruction
    uint32_t adr = arm_read_register(p, (uint8_t)((ins & 0x000F0000)>>16));
    uint16_t reg = ins & 0xFFFF;
    int p = ins & 0x01000000;
    int u = ins & 0x00800000;
    int s = ins & 0x00400000;
    int w = ins & 0x00200000;
    int l = ins & 0x00100000;
    uint8_t i = 0;
    int j = 0;
    if(p){
        if(u){adr = adr + 4;}
        else{adr = adr - 4;}
    }



    while(!j){
        if(reg & (1<<i)){            //le registre codé par le bit numéro i est concerné
            if(l && !s){arm_write_register(p, i, arm_read_word(p, adr, p->reg[i]));}
            else if(l && s){arm_write_usr_register(p, i, arm_read_word(p, adr, p->reg[i]);)}
            else if(!l && !s){arm_write_word(p, adr, arm_read_register(p, i))}
            else{arm_write_word(p, adr, arm_read_usr_register(p, i));}
            if(u){adr = adr + 4;}
            else{adr = adr - 4;}
        }

        if(i == 15){j++;}
        i++;
    }


    if((reg & (1 << 15)) && s && arm_current_mode_has_spsr(p)){
        arm_write_cpsr(p, arm_read_spsr(p));
        }

    if(w){
        arm_write_register(p, (unit8_t)(ins & 0x000F0000)>>16), adr);
    }

    return 0;

    return UNDEFINED_INSTRUCTION;
}

int arm_coprocessor_load_store(arm_core p, uint32_t ins) {
    /* Not implemented */
    return UNDEFINED_INSTRUCTION;
}
