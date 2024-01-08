/*
Armator - simulateur de jeu d'instruction ARMv5T a but pedagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique Generale GNU publiee par la Free Software
Foundation (version 2 ou bien toute autre version ulterieure choisie par vous).

Ce programme est distribue car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but specifique. Reportez-vous a la
Licence Publique Generale GNU pour plus de details.

Vous devez avoir recu une copie de la Licence Publique Generale GNU en meme
temps que ce programme ; si ce n'est pas le cas, ecrivez a la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
etats-Unis.

Contact: Guillaume.Huard@imag.fr
	 Batiment IMAG
	 700 avenue centrale, domaine universitaire
	 38401 Saint Martin d'Heres
*/
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"
#include <math.h>
#include <stdlib.h>



/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {

	//Specifies the first source operand register.
	uint8_t Rn_num = (uint8_t)get_bits(ins,19,16);
	uint32_t Rn_value = arm_read_register(p,Rn_num);


	//Specifies the destination register.
	uint8_t Rd_num =  (uint8_t)get_bits(ins,15,12);
	uint32_t Rd_value = arm_read_register(p,Rd_num);

	//S bit Indicates that the instruction updates the condition codes.
	uint8_t S = 0;
	S = (uint8_t)get_bit(ins,20);

	//the c flag
	uint32_t cpsr = arm_read_cpsr(p);
	uint8_t FLAG_c = 0;
	FLAG_c = get_bit(cpsr,29);

	//calcul de shifter_operand et shifter_carry_out
	shifter_values * MyShifterValue = shifter_values_calculator(p,ins,FLAG_c);

	//Specifies the operation of the instruction.
	uint8_t opcode = (uint8_t)get_bits(ins,24,21);

	switch (opcode){

	case 0b0000: //AND (page A4-8) opcode:0000
		arm_write_register(p,Rd_num,(uint32_t) Rn_value & MyShifterValue->shifter_operand);
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C flag MyShifterValue->shifter_carry_out*/	
			if (MyShifterValue->shifter_carry_out){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
			/*V FLAG unaffected*/
			arm_write_cpsr(p,cpsr_val);
		}
		break;

	case 0b0001: // EOR (page A4-32) opcode:0001
		arm_write_register(p,Rd_num,(uint32_t) Rn_value ^ MyShifterValue->shifter_operand);
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C flag MyShifterValue->shifter_carry_out*/	
			if (MyShifterValue->shifter_carry_out){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
			/*V FLAG unaffected*/
			arm_write_cpsr(p,cpsr_val);
		}
		break;
	
	case 0b0010: //SUB (page A4-208) opcode:0010
		arm_write_register(p,Rd_num,(uint32_t) Rn_value - MyShifterValue->shifter_operand);
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C Flag = NOT BorrowFrom(Rn - shifter_operand)*/	
			if (BorrowFrom(Rn_value,MyShifterValue->shifter_operand)){clr_bit(cpsr_val,29);}else{set_bit(cpsr_val,29);}
			/*V Flag = OverflowFrom(Rn - shifter_operand)*/
			if(OverflowFrom(Rd_value,Rn_value,MyShifterValue->shifter_operand)){set_bit(cpsr_val,28);}else{clr_bit(cpsr_val,28);}
			arm_write_cpsr(p,cpsr_val);
		}
		break;

	case 0b0011: //RSB (page A4-115) opcode:0011
		arm_write_register(p,Rd_num, MyShifterValue->shifter_operand - (uint32_t) Rn_value );
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C Flag = NOT BorrowFrom(shifter_operand - Rn)*/	
			if (BorrowFrom(MyShifterValue->shifter_operand,Rn_value)){clr_bit(cpsr_val,29);}else{set_bit(cpsr_val,29);}
			/*V Flag = OverflowFrom(shifter_operand - Rn)*/
			if(OverflowFrom(Rd_value,MyShifterValue->shifter_operand,Rn_value)){set_bit(cpsr_val,28);}else{clr_bit(cpsr_val,28);}
			arm_write_cpsr(p,cpsr_val);
		}
		break;
	
	case 0b0100: //ADD (page A4-6) opcode: 0100

		arm_write_register(p,Rd_num,(uint32_t) (Rn_value+MyShifterValue->shifter_operand));
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C flag CarryFrom(Rn_value+MyShifterValue->shifter_operand+(uint32_t)FLAG_c))*/	
			if (carryFrom(Rn_value,MyShifterValue->shifter_operand,(uint32_t)FLAG_c)){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
			/*V FLAG OverflowFrom(Rn_value+MyShifterValue->shifter_operand+(uint32_t)FLAG_c))*/
			if(OverflowFrom(Rd_value,Rn_value,MyShifterValue->shifter_operand)){set_bit(cpsr_val,28);}else{clr_bit(cpsr_val,28);}
			arm_write_cpsr(p,cpsr_val);
		}
		break;

	case 0b0101: //0101 ADC Syntax: ADC{<cond>}{S} <Rd>, <Rn>, <shifter_operand> (page A4-4)

		arm_write_register(p,Rd_num,(uint32_t) (Rn_value+MyShifterValue->shifter_operand+(uint32_t)FLAG_c));
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C flag CarryFrom(Rn_value+MyShifterValue->shifter_operand+(uint32_t)FLAG_c))*/	
			if (carryFrom(Rn_value,MyShifterValue->shifter_operand,(uint32_t)FLAG_c)){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
			/*V FLAG OverflowFrom(Rn_value+MyShifterValue->shifter_operand+(uint32_t)FLAG_c))*/
			if(OverflowFrom(Rd_value,Rn_value,MyShifterValue->shifter_operand)){set_bit(cpsr_val,28);}else{clr_bit(cpsr_val,28);}
			arm_write_cpsr(p,cpsr_val);
		}
		break;

	case 0b0110: //SBC (page A4-125) opcode: 0110
		arm_write_register(p,Rd_num,(uint32_t) Rn_value - MyShifterValue->shifter_operand - (uint32_t)~FLAG_c);
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C Flag = NOT BorrowFrom(Rn - shifter_operand)*/	
			if (BorrowFrom(Rn_value,MyShifterValue->shifter_operand + (uint32_t)~FLAG_c)){clr_bit(cpsr_val,29);}else{set_bit(cpsr_val,29);}
			/*V Flag = OverflowFrom(Rn - shifter_operand)*/
			if(OverflowFrom(Rd_value,Rn_value,MyShifterValue->shifter_operand + (uint32_t)~FLAG_c)){set_bit(cpsr_val,28);}else{clr_bit(cpsr_val,28);}
			arm_write_cpsr(p,cpsr_val);
		}
		break;

	
	case 0b0111: //RSC (page A4-117) opcode:0111
		arm_write_register(p,Rd_num, MyShifterValue->shifter_operand - (uint32_t) Rn_value - (uint32_t)~FLAG_c );
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C Flag = NOT BorrowFrom(shifter_operand - Rn)*/	
			if (BorrowFrom(MyShifterValue->shifter_operand,Rn_value+ (uint32_t)~FLAG_c)){clr_bit(cpsr_val,29);}else{set_bit(cpsr_val,29);}
			/*V Flag = OverflowFrom(shifter_operand - Rn)*/
			if(OverflowFrom(Rd_value,MyShifterValue->shifter_operand,Rn_value + (uint32_t)~FLAG_c)){set_bit(cpsr_val,28);}else{clr_bit(cpsr_val,28);}
			arm_write_cpsr(p,cpsr_val);
		}
		break;

	case 0b1000 : //TST (page A4-230) opcode:1000
		uint32_t alu_out = Rn_value & MyShifterValue->shifter_operand;
		uint32_t cpsr_val = arm_read_cpsr(p);
		/*N FLAG = alu_out[31]*/
		if (get_bit(alu_out,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
		/*Z Flag = if alu_out == 0 then 1 else 0*/
		if (alu_out ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
		/*C flag MyShifterValue->shifter_carry_out*/	
		if (MyShifterValue->shifter_carry_out){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
		/*V FLAG unaffected*/
		arm_write_cpsr(p,cpsr_val);
		break;

	case 9: //1001 TEQ
		uint32_t alu_out = Rn_value ^ MyShifterValue->shifter_operand;
		uint32_t cpsr_val = arm_read_cpsr(p);
		/*N FLAG = alu_out[31]*/
		if (get_bit(alu_out,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
		/*Z Flag = if alu_out == 0 then 1 else 0*/
		if (alu_out ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
		/*C flag MyShifterValue->shifter_carry_out*/	
		if (MyShifterValue->shifter_carry_out){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
		/*V FLAG unaffected*/
		arm_write_cpsr(p,cpsr_val);
		break;

	case 0b1010: //CMP (page:A4-28) opcode:1010
		uint32_t alu_out = Rn_value - MyShifterValue->shifter_operand;
		uint32_t cpsr_val = arm_read_cpsr(p);
		/*N FLAG = alu_out[31]*/
		if (get_bit(alu_out,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
		/*Z Flag = if alu_out == 0 then 1 else 0*/
		if (alu_out ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
		/*C Flag = NOT BorrowFrom(Rn - shifter_operand)*/	
		if (BorrowFrom(Rn_value,MyShifterValue->shifter_operand)){clr_bit(cpsr_val,29);}else{set_bit(cpsr_val,29);}
		/*V Flag = OverflowFrom(Rn - shifter_operand)*/
		if(OverflowFrom(Rd_value,Rn_value,MyShifterValue->shifter_operand)){set_bit(cpsr_val,28);}else{clr_bit(cpsr_val,28);}
		arm_write_cpsr(p,cpsr_val);
		break;

	case 0b1011: //CMN (page A4-26) opcode:1011
		uint32_t alu_out = Rn_value + MyShifterValue->shifter_operand;
		uint32_t cpsr_val = arm_read_cpsr(p);
		/*N FLAG = alu_out[31]*/
		if (get_bit(alu_out,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
		/*Z Flag = if alu_out == 0 then 1 else 0*/
		if (alu_out ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
		/*C flag CarryFrom(Rn_value+MyShifterValue->shifter_operand+(uint32_t)FLAG_c))*/	
		if (carryFrom(Rn_value,MyShifterValue->shifter_operand,(uint32_t)0)){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
		/*V FLAG OverflowFrom(Rn_value+MyShifterValue->shifter_operand+(uint32_t)FLAG_c))*/
		if(OverflowFrom(Rd_value,Rn_value,MyShifterValue->shifter_operand)){set_bit(cpsr_val,28);}else{clr_bit(cpsr_val,28);}
		arm_write_cpsr(p,cpsr_val);
		break;
	
	case 0b1100: //ORR (page A4-84) opcode:1100
		arm_write_register(p,Rd_num,(uint32_t) Rn_value | MyShifterValue->shifter_operand);
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C flag not borrowFrom(rn - shifter_operand)*/	
			if (MyShifterValue->shifter_carry_out){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
			/*V FLAG unaffected*/
			arm_write_cpsr(p,cpsr_val);
		}
		break;

	case 0b1101: //MOV (page A4-68) opcode:1101
		arm_write_register(p,Rd_num,(uint32_t) MyShifterValue->shifter_operand);
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C flag MyShifterValue->shifter_carry_out*/	
			if (MyShifterValue->shifter_carry_out){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
			/*V FLAG unaffected*/
			arm_write_cpsr(p,cpsr_val);
		}
		break;

	case 0b1110: // BIC (page A4-12) opcode:1110
		arm_write_register(p,Rd_num,(uint32_t) Rn_value & ~(MyShifterValue->shifter_operand));
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C flag MyShifterValue->shifter_carry_out*/	
			if (MyShifterValue->shifter_carry_out){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
			/*V FLAG unaffected*/
			arm_write_cpsr(p,cpsr_val);
		}
	
		break;

	case 0b1111: //1111 MVN
		arm_write_register(p,Rd_num,(uint32_t) ~(MyShifterValue->shifter_operand));
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
		}
		else if(S==1){
			uint32_t cpsr_val = arm_read_cpsr(p);
			/*N FLAG = RD[31]*/
			if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
			/*Z FLAG = 1 if Rd==0 */
			if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
			/*C flag MyShifterValue->shifter_carry_out*/	
			if (MyShifterValue->shifter_carry_out){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
			/*V FLAG unaffected*/
			arm_write_cpsr(p,cpsr_val);
		}
		break;
	
	default:
		break;
	}
	return UNDEFINED_INSTRUCTION;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
	
	uint8_t rotate_imm = (uint8_t) get_bits(ins,11,8);
	uint16_t immed_8 = (uint16_t) get_bits(ins,7,0);

	//the c flag
	uint32_t cpsr = arm_read_cpsr(p);
	uint8_t FLAG_c = 0;
	FLAG_c = get_bit(cpsr,29);

	/*shifter_operand = immed_8 Rotate_Right (rotate_imm * 2)
	if rotate_imm == 0 then
	shifter_carry_out = C flag
	else /* rotate_imm != 0 
	shifter_carry_out = shifter_operand[31] (page A5-6)*/
	uint32_t  shifter_operand = (immed_8 >> (rotate_imm * 2)) | (immed_8 << (16 - rotate_imm * 2));
	uint8_t shifter_carry_out = 0;
	if (rotate_imm ==0) {shifter_carry_out = FLAG_c;}
	else{shifter_carry_out = (uint8_t)get_bit(shifter_operand,31);}

	//Specifies the first source operand register.
	uint8_t Rn_num = (uint8_t)get_bits(ins,19,16);
	uint32_t Rn_value = arm_read_register(p,Rn_num);


	//Specifies the destination register.
	uint8_t Rd_num =  (uint8_t)get_bits(ins,15,12);
	uint32_t Rd_value = arm_read_register(p,Rd_num);

	//S bit Indicates that the instruction updates the condition codes.
	uint8_t S = 0;
	S = (uint8_t)get_bit(ins,20);

	

	//Specifies the operation of the instruction.
	uint8_t opcode = (uint8_t)get_bits(ins,24,21);

	switch (opcode){

		case 0b0100: //ADD (page A4-6) opcode: 0100

			arm_write_register(p,Rd_num,(uint32_t) (Rn_value+shifter_operand));
			Rd_value = arm_read_register(p,Rd_num);
			if (S==1 && Rd_num==15){
				if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
			}
			else if(S==1){
				uint32_t cpsr_val = arm_read_cpsr(p);
				/*N FLAG = RD[31]*/
				if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
				/*Z FLAG = 1 if Rd==0 */
				if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
				/*C flag CarryFrom(Rn_value+MyShifterValue->shifter_operand+(uint32_t)FLAG_c))*/	
				if (carryFrom(Rn_value,shifter_operand,(uint32_t)FLAG_c)){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
				/*V FLAG OverflowFrom(Rn_value+MyShifterValue->shifter_operand+(uint32_t)FLAG_c))*/
				if(OverflowFrom(Rd_value,Rn_value,shifter_operand)){set_bit(cpsr_val,28);}else{clr_bit(cpsr_val,28);}
				arm_write_cpsr(p,cpsr_val);
			}
			return 0;
			break;

		case 0b1101: //MOV (page A4-68) opcode:1101
			arm_write_register(p,Rd_num,(uint32_t) shifter_operand);
			Rd_value = arm_read_register(p,Rd_num);
			if (S==1 && Rd_num==15){
				if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,(uint32_t)arm_read_spsr(p));}else{return UNDEFINED_INSTRUCTION;}
			}
			else if(S==1){
				uint32_t cpsr_val = arm_read_cpsr(p);
				/*N FLAG = RD[31]*/
				if (get_bit(Rd_value,31)==1){set_bit(cpsr_val,31); } else{clr_bit(cpsr_val,31);}
				/*Z FLAG = 1 if Rd==0 */
				if (Rd_value ==0) {set_bit(cpsr_val,30);} else{clr_bit(cpsr_val,30);}
				/*C flag = shifter_carry_out*/	
				if (shifter_carry_out){set_bit(cpsr_val,29);}else{clr_bit(cpsr_val,29);}
				/*V FLAG unaffected*/
				arm_write_cpsr(p,cpsr_val);
			}
			return 0;
			break;	
		default:
			return UNDEFINED_INSTRUCTION;
	}



}

shifter_values* shifter_values_calculator(arm_core p,uint32_t ins,uint8_t FLAG_c){

    	shifter_values * MyShifterValue = (shifter_values *)malloc(sizeof(shifter_values));	
 	

	//the 4th bit specify if it's an Immediate shifts(bit_4 == 0) or  Register shifts(bit_4 == 1)
	uint8_t bit_4 = 0;
	bit_4 = (uint8_t)get_bit(ins,4);
	
	if (MyShifterValue != NULL) {
		uint32_t Rm = arm_read_register(p,(uint8_t)get_bits(ins,3,0));

		//Data-processing operands - Register syntax: <Rm> (page A5-8)
		if(get_bits(ins,11,4)==0){
			MyShifterValue->shifter_carry_out = FLAG_c;
			MyShifterValue->shifter_operand = Rm;
		}

		//Data-processing operands - Rotate right with extend syntax: <Rm>, RRX (page A5-17)
		if(get_bits(ins,11,4)==0b00000110){
			MyShifterValue->shifter_carry_out =get_bit(Rm,0) ;
			MyShifterValue->shifter_operand = (FLAG_c << 31) | (Rm >> 1);
		}

		//immediate shift
		if (bit_4 ==0){
			uint8_t shift_imm = get_bits(ins,11,7);
			
			switch (get_bits(ins,6,4)){

			//Data-processing operands - Logical shift left by immediate Syntax: <Rm>, LSL #<shift_imm> (page A5-9)
			case 0b000:
				if(shift_imm ==0){
					MyShifterValue->shifter_operand = Rm;
					MyShifterValue->shifter_carry_out = FLAG_c;
				}
				else{
					MyShifterValue->shifter_operand = Rm << shift_imm;
					MyShifterValue->shifter_carry_out = get_bit(Rm,32-shift_imm);
				
				}
				return MyShifterValue;
				break;

			//Data-processing operands - Logical shift right by immediate Syntax: <Rm>, LSR #<shift_imm> (page A5-11)
			case 0b010:
				if(shift_imm ==0){
					MyShifterValue->shifter_operand = 0;
					MyShifterValue->shifter_carry_out = get_bit(Rm,31);
				}
				else{
					MyShifterValue->shifter_operand = Rm >> shift_imm;
					MyShifterValue->shifter_carry_out = get_bit(Rm,shift_imm-1);
				}
				return MyShifterValue;
				break;

			//Data-processing operands - Arithmetic shift right by immediate Syntax: <Rm>, ASR #<shift_imm> (page A5-13)
			case 0b100:
				if(shift_imm ==0){
					if(get_bit(Rm,31)==0){
						MyShifterValue->shifter_operand = 0;
						MyShifterValue->shifter_carry_out = get_bit(Rm,31);
					}
					else{
						MyShifterValue->shifter_operand = (uint8_t)0xFFFFFFFF;
						MyShifterValue->shifter_carry_out = get_bit(Rm,31);
					}
				}
				else{
					MyShifterValue->shifter_operand = Rm >> shift_imm;
					MyShifterValue->shifter_carry_out = get_bit(Rm,shift_imm-1);
				}
				return MyShifterValue;
				break;
			//Data-processing operands - Rotate right by immediate Syntax: <Rm>, ROR #<shift_imm> (page A5-15)
			case 0b110:
				if(shift_imm ==0){
					//Data-processing operands - Rotate right with extend (page A5-17)
					MyShifterValue->shifter_operand = (FLAG_c << 31) | (Rm >> 1);
					MyShifterValue->shifter_carry_out = get_bit(Rm,0);
				}
				else{
					// Calculate the number of bits in an unsigned int
					uint8_t numBits = sizeof(uint32_t) * 8;

					// Ensure the shift_imm value is within the range of 0 to numBits - 1
					shift_imm = shift_imm % numBits;

					// Perform the right rotation
					MyShifterValue->shifter_operand = (Rm >> shift_imm) | (Rm << (numBits - shift_imm));
					MyShifterValue->shifter_carry_out = get_bit(Rm,shift_imm-1);
				}
				return MyShifterValue;
				break;
			
			default:
				break;
			}
		}
		//Register Shift
		else{
			uint32_t Rm = arm_read_register(p,(uint8_t)get_bits(ins,3,0));
			uint32_t Rs = arm_read_register(p,(uint8_t)get_bits(ins,11,8));
			uint16_t Rs7_0 = get_bits(Rs,7,0);
			switch (get_bits(ins,7,4)){
			//Data-processing operands - Logical shift left by register Syntax: <Rm>, LSL <Rs> (page A5-10)
			case 0b0001:
				
				if(Rs7_0 ==0){
					MyShifterValue->shifter_operand = Rm;
					MyShifterValue->shifter_carry_out = FLAG_c;
				}
				else if(Rs7_0<32){
					MyShifterValue->shifter_operand = Rm << Rs7_0;
					MyShifterValue->shifter_carry_out = (uint8_t)get_bit(Rm,32-Rs7_0);
				}
				else if(Rs7_0 == 32){
					MyShifterValue->shifter_operand = Rm;
					MyShifterValue->shifter_carry_out = (uint8_t)get_bit(Rm,0);
				}
				else/* Rs[7:0] > 32 */{ 
					MyShifterValue->shifter_operand =0;
					MyShifterValue->shifter_carry_out = 0;
				}
				return MyShifterValue;
				break;
			//Data-processing operands - Logical shift right by register Syntax: <Rm>, LSR <Rs> (page A5-12)
			case 0b0011:
				if(Rs7_0 ==0){
					MyShifterValue->shifter_operand = Rm;
					MyShifterValue->shifter_carry_out = FLAG_c;
				}
				else if(Rs7_0<32){
					MyShifterValue->shifter_operand = Rm >> Rs7_0;
					MyShifterValue->shifter_carry_out = (uint8_t)get_bit(Rm,Rs7_0-1);
				}
				else if(Rs7_0 == 32){
					MyShifterValue->shifter_operand = 0;
					MyShifterValue->shifter_carry_out = (uint8_t)get_bit(Rm,31);
				}
				else/* Rs[7:0] > 32 */{ 
					MyShifterValue->shifter_operand =0;
					MyShifterValue->shifter_carry_out = 0;
				}
				return MyShifterValue;
				break;

			// Data-processing operands - Arithmetic shift right by register Syntax: <Rm>, ASR <Rs> (page A5-14)
			case 0b0101:
				if(Rs7_0 ==0){
					MyShifterValue->shifter_operand = Rm;
					MyShifterValue->shifter_carry_out = FLAG_c;
				}
				else if(Rs7_0<32){
					MyShifterValue->shifter_operand = Rm >> Rs7_0;
					MyShifterValue->shifter_carry_out = (uint8_t)get_bit(Rm,Rs7_0-1);
				}
				else /*(Rs7_0 >= 32)*/{
					if(get_bit(Rm,31)==0){
						MyShifterValue->shifter_operand = 0;
						MyShifterValue->shifter_carry_out = (uint8_t)get_bit(Rm,31);
					}
					else/* Rm[31] == 1 */{
						MyShifterValue->shifter_operand = (uint32_t)0xFFFFFFFF;
						MyShifterValue->shifter_carry_out = (uint8_t)get_bit(Rm,31);
					}
				}
				
				return MyShifterValue;
				break;

			// Data-processing operands - Rotate right by register Syntax: <Rm>, ROR <Rs> (page A5-16)
			case 0b0111:
				uint8_t Rs4_0 = get_bits(Rs,4,0);
				if(Rs7_0 ==0){
					MyShifterValue->shifter_operand = Rm;
					MyShifterValue->shifter_carry_out = FLAG_c;
				}
				else if(Rs4_0 == 0){
					MyShifterValue->shifter_operand = Rm;
					MyShifterValue->shifter_carry_out = (uint8_t)get_bit(Rm,31);
				}
				else /* Rs[4:0] > 0 */{
					// Calculate the number of bits in an unsigned int
					uint8_t numBits = sizeof(uint32_t) * 8;

					// Ensure the shift_imm value is within the range of 0 to numBits - 1
					Rs4_0 = Rs4_0 % numBits;

					// Perform the right rotation
					MyShifterValue->shifter_operand = (Rm >> Rs4_0) | (Rm << (numBits - Rs4_0));
					MyShifterValue->shifter_carry_out = (uint8_t)get_bit(Rm,Rs4_0-1);
				}
				
				return MyShifterValue;
				break;
			
			default:
				break;
			}
		}
	}
	return NULL;

}


/*Returns 1 if the addition specified as its parameter caused a carry (true result is bigger than 232−1, where
the operands are treated as unsigned integers), and returns 0 in all other cases. This delivers further
information about an addition which occurred earlier in the pseudo-code. The addition is not repeated.(page: Glossary-4)*/
int carryFrom(uint32_t a, uint32_t b, uint32_t c){
	return((a+b+c) > pow(2,32)-1);
}

// Function to check if subtraction caused a borrow
int BorrowFrom(uint32_t op1, uint32_t op2) {
    return op1 < op2;
}
//overflow function:
int OverflowFrom(uint32_t Rd,uint32_t Rn, uint32_t shifter_operand){
	return get_bit(Rn,31)==get_bit(shifter_operand,31) && get_bit(Rd,31)!=get_bit(Rn,31);
}