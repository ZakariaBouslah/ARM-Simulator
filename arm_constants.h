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
#ifndef __ARM_CONSTANTS_H__
#define __ARM_CONSTANTS_H__
#include <stdint.h>

/* ARM Modes */
#define USR 0x10
#define FIQ 0x11
#define IRQ 0x12
#define SVC 0x13
#define ABT 0x17
#define UND 0x1b
#define SYS 0x1f

/* Registers */
#define R0		0
#define R1		1
#define R2		2
#define R3		3
#define R4		4
#define R5		5
#define R6		6
#define R7		7
#define R8		8
#define R9		9
#define R10		10
#define R11		11
#define R12		12
#define SR		13		//R13
#define LR		14		//R14		
#define PC		15		//R15
#define R13_SVC		16
#define R14_SVC		17
#define R13_ABT		18
#define R14_ABT		19
#define R13_UND		20
#define R14_UND		21
#define R13_IRQ		22
#define R14_IRQ		23
#define R8_FIQ		24
#define R9_FIQ		25
#define R10_FIQ		26
#define R11_FIQ		27
#define R12_FIQ		28
#define R13_FIQ		29
#define R14_FIQ		30
#define CPSR		31
#define SPSR_SVC	32
#define SPSR_ABT	33
#define SPSR_UND	34
#define SPSR_IRQ	35
#define SPSR_FIQ	36
/* ARM Exceptions (by priority) */
#define RESET                   1
#define DATA_ABORT              2
#define FAST_INTERRUPT          3
#define INTERRUPT               4
#define IMPRECISE_ABORT         5	// Unsupported, ARMV6
#define PREFETCH_ABORT          6
#define UNDEFINED_INSTRUCTION   7
#define SOFTWARE_INTERRUPT      8
/* The last one is not realy an exception, but, as we handle software interrupts
 * within the simulator and we decide there to end the simulation, this is a way
 * to tell it to the outside world
 */
#define END_SIMULATION          9

/* Some CPSR bits */
#define N 31
#define Z 30
#define C 29
#define V 28

/* shift operations */
#define LSL 0
#define LSR 1
#define ASR 2
#define ROR 3

/* Bit mask constants for msr */
/* We simulate architecture v5T */
#define UnallocMask 0x0FFFFF00
#define UserMask    0xF0000000
#define PrivMask    0x0000000F
#define StateMask   0x00000020

char *arm_get_exception_name(unsigned char exception);
char *arm_get_mode_name(uint8_t mode);
int8_t arm_get_mode_number(char *name);
char *arm_get_register_name(uint8_t reg);

#endif
