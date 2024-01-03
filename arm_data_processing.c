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

typedef struct {
    uint8_t shifter_carry_out;
    uint8_t shifter_operand;
} shifter_values;

/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {

	//Specifies the first source operand register.
	uint8_t Rn = get_bits(ins,19,16);

	//Specifies the destination register.
	uint8_t Rd = get_bits(ins,15,12);

	//S bit Indicates that the instruction updates the condition codes.
	uint8_t S = 0;
	S = get_bit(ins,20);

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
	bit_4 = get_bit(ins,4);

	// Specifies the register whose value is the instruction operand.
	uint8_t Rm = get_bits(ins,3,0);



	//Specifies the operation of the instruction.
	uint8_t opcode = get_bits(ins,24,21);

	switch (opcode){

	case 0: //0000 AND I
		instruction_AND (p,ins);
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
	
	case 4: //0100 ADD I
		instruction_ADD (p,ins);
		break;

	case 5: //0101 ADC I
		instruction_ADC (p,ins);
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
	
	case 12: //1100 ORR
		instruction_ORR (p,ins);
		break;

	case 13: //1101 MOV
		instruction_MOV (p,ins);
		break;

	case 14: //1110 BIC I
		instruction_BIC (p,ins);
		break;

	case 15: //1111 MVN
		instruction_MVN (p,ins);
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

		//Data-processing operands - Register syntax: <Rm> (page A5-8)
		if(get_bits(ins,11,4)==0){
			MyShifterValue->shifter_carry_out = FLAG_c;
			MyShifterValue->shifter_operand = get_bits(ins,3,0);
		}
		//immediate shift
		if (bit_4 ==0){
			uint8_t shift_imm = get_bits(ins,11,7);
			uint32_t Rm = arm_read_register(p,(uint8_t)get_bits(ins,3,0));
			switch (get_bits(ins,6,4)){

			//Data-processing operands - Logical shift left by immediate Syntax: <Rm>, LSL #<shift_imm> (page A5-9)
			case 0b000:
				MyShifterValue->shifter_operand = Rm << shift_imm;
				MyShifterValue->shifter_carry_out = get_bit(Rm,32-shift_imm);
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
			

			
			default:
				break;
			}
		}
		//Register Shift
		else{

		}
	}
	return NULL;

}


