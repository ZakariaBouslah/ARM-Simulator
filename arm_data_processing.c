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
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"
#include <math.h>
typedef struct {
    uint8_t shifter_carry_out;
    uint8_t shifter_operand;
} shifter_values;

int carryFrom(uint32_t a, uint32_t b, uint32_t c){
	return((a+b+c) > pow(2,32)-1);
}
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

	//the flags
	uint32_t cpsr = arm_read_cpsr(p);
    	uint8_t FLAG_n = 0;
	FLAG_n = get_bit(cpsr,31);
  	uint8_t FLAG_z = 0;
	FLAG_z = get_bit(cpsr,30);
  	uint8_t FLAG_c = 0;
	FLAG_c = get_bit(cpsr,29);
 	uint8_t FLAG_v = 0;
	FLAG_v = get_bit(cpsr,28);

	//the 4th bit specify if it's an Immediate shifts(bit_4 == 0) or  Register shifts(bit_4 == 1)
	uint8_t bit_4 = 0;
	bit_4 = (uint8_t)get_bit(ins,4);

	//
	shifter_values * MyShifterValue = shifter_values_calculator(p,ins,FLAG_c,bit_4);

	//Specifies the operation of the instruction.
	uint8_t opcode = (uint8_t)get_bits(ins,24,21);

	switch (opcode){

	case 0b0000: //AND (page A4-8) opcode:0000
		arm_write_register(p,Rd_num,(uint32_t) Rn_value & MyShifterValue->shifter_operand);
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,arm_read_spsr);}else{return UNDEFINED_INSTRUCTION;}
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

	case 1: //0001 EOR
		instruction_EOR (p,ins);
		break;
	
	case 2: //0010 SUB
		instruction_SUB (p,ins);
		break;

	case 3: //0011 RSB
		instruction_RSB (p,ins);
		break;
	
	case 0b0100: //ADD (page A4-6) opcode: 0100
		arm_write_register(p,Rd_num,(uint32_t) (Rn_value+MyShifterValue->shifter_operand));
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,arm_read_spsr);}else{return UNDEFINED_INSTRUCTION;}
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
			if(get_bit(Rn_value,31)==get_bit(MyShifterValue->shifter_operand,31) && get_bit(Rd_value,31)!=get_bit(Rn_value,31)){set_bit(cpsr_val,28);}else{clr_bit(cpsr_val,28);}
			arm_write_cpsr(p,cpsr_val);
		}
		break;

	case 0b0101: //0101 ADC Syntax: ADC{<cond>}{S} <Rd>, <Rn>, <shifter_operand> (page A4-4)
		arm_write_register(p,Rd_num,(uint32_t) (Rn_value+MyShifterValue->shifter_operand+(uint32_t)FLAG_c));
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,arm_read_spsr);}else{return UNDEFINED_INSTRUCTION;}
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
			if(get_bit(Rn_value,31)==get_bit(MyShifterValue->shifter_operand,31) && get_bit(Rd_value,31)!=get_bit(Rn_value,31)){set_bit(cpsr_val,28);}else{clr_bit(cpsr_val,28);}
			arm_write_cpsr(p,cpsr_val);
		}
		break;

	case 6: //0110 SBC
		instruction_SBC (p,ins);
		break;
	
	case 7: //0111 RSC
		instruction_RSC (p,ins);
		break;

	case 8: //1000 TST
		instruction_TST (p,ins);
		break;

	case 9: //1001 TEQ
		instruction_TEQ (p,ins);
		break;

	case 10: //1010 CMP I
		instruction_CMP (p,ins);
		break;

	case 11: //1011 CMN I
		instruction_CMN (p,ins);
		break;
	
	case 0b1100: //1100 ORR
		arm_write_register(p,Rd_num,(uint32_t) Rn_value | MyShifterValue->shifter_operand);
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,arm_read_spsr);}else{return UNDEFINED_INSTRUCTION;}
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

	case 0b1101: //MOV (page A4-68) opcode:1101
		arm_write_register(p,Rd_num,(uint32_t) MyShifterValue->shifter_operand);
		Rd_value = arm_read_register(p,Rd_num);
		if (S==1 && Rd_num==15){
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,arm_read_spsr);}else{return UNDEFINED_INSTRUCTION;}
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
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,arm_read_spsr);}else{return UNDEFINED_INSTRUCTION;}
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
			if(arm_current_mode_has_spsr(p)){arm_write_cpsr(p,arm_read_spsr);}else{return UNDEFINED_INSTRUCTION;}
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
    return UNDEFINED_INSTRUCTION;
}

shifter_values* shifter_values_calculator(arm_core p,uint32_t ins, uint8_t FLAG_c, uint8_t bit_4){

    	shifter_values * MyShifterValue = (shifter_values *)malloc(sizeof(shifter_values));	
	
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
						MyShifterValue->shifter_operand = 0xFFFFFFFF;
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
			switch (get_bits(ins,7,4)){
			//Data-processing operands - Logical shift left by register Syntax: <Rm>, LSL <Rs> (page A5-10)
			case 0b0001:
				uint16_t Rs7_0 = get_bits(Rs,7,0);
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
				uint16_t Rs7_0 = get_bits(Rs,7,0);
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
				uint16_t Rs7_0 = get_bits(Rs,7,0);
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
						MyShifterValue->shifter_operand = 0xFFFFFFFF;
						MyShifterValue->shifter_carry_out = (uint8_t)get_bit(Rm,31);
					}
				}
				
				return MyShifterValue;
				break;

			// Data-processing operands - Rotate right by register Syntax: <Rm>, ROR <Rs> (page A5-16)
			case 0b0111:
				uint16_t Rs7_0 = get_bits(Rs,7,0);
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


