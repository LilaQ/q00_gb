#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstdint>
#include "structs.h"
#define DEBUG_
unsigned char opcode;

//	BIT - L2 - T8 - Z01/
//	Test Bit
int op_bit(unsigned char& parameter, int bitnr, Flags& flags, Registers& registers, uint16_t& pc, int m = 2) {
#ifdef DEBUG
	printf("BIT");
#endif // DEBUG
	flags.Z = ~(parameter >> bitnr) & 0x1;
	flags.N = 0;
	flags.H = 1;
	pc += 2;

	//	return m-cycles
	return m;
}

//	CP
//	Compare
int op_cp(unsigned char& parameter, unsigned char& val, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("CP");
#endif // DEBUG
	flags.Z = (parameter == val) ? 1 : 0;
	flags.N = 1;
	flags.H = (parameter & 0xF) < (val & 0xF);
	flags.C = (parameter < val) ? 1 : 0;
	pc += 1;

	//	return m-cycle
	return 1;
}

// LD X, Y
// Load Y into X
int op_ld(unsigned char& parameter, unsigned char& val, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("LD");
#endif // DEBUG
	parameter = val;
	pc += 1;

	//	return m-cycle
	return 1;
}

//	RES - L2 - T8 - ////
//	Reset Bit
int op_res(unsigned char& parameter, int bitnr, Flags& flags, Registers& registers, uint16_t& pc, int m = 2) {
#ifdef DEBUG
	printf("RES");
#endif // DEBUG
	parameter = parameter & ~(0x1 << bitnr);
	pc += 2;

	//	return m-cycles
	return m;
}

//	RLC - L2 - T8 - Z00C
//	Rotate left TO carry bit (bit 7 to CF, and bit 7 to bit 0)
void op_rlc(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("RLC");
#endif // DEBUG
	flags.C = parameter >> 7 & 0x01;
	flags.N = 0;
	flags.H = 0;
	parameter = parameter << 1 | flags.C;
	flags.Z = (parameter == 0);
	pc += 2;
}

//	RRC - L2 - T8 - Z00C
//	Rotate right TO carry bit (bit 0 to CF, and bit 0 to bit 7)
void op_rrc(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("RRC");
#endif // DEBUG
	flags.C = parameter & 0x01;
	flags.N = 0;
	flags.H = 0;
	parameter = parameter >> 1 | (flags.C << 7);
	flags.Z = (parameter == 0);
	pc += 2;
}

//	RL - L2 - T8 - Z00C
//	Rotate left THROUGH carry flag (CF to bit 0, bit 7 to CF)
void op_rl(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("RL");
#endif // DEBUG
	int oldcarry = flags.C;
	flags.C = parameter >> 7 & 0x01;
	flags.N = 0;
	flags.H = 0;
	parameter = parameter << 1 | oldcarry;
	flags.Z = (parameter == 0);
	pc += 2;
}

//	RR - L2 - T8 - Z00C
//	Rotate right THROUGH carry flag (CF to bit 7, bit 0 to CF)
void op_rr(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("RR");
#endif // DEBUG
	int oldcarry = flags.C;
	flags.C = parameter & 0x01;
	flags.N = 0;
	flags.H = 0;
	parameter = parameter >> 1 | (oldcarry << 7);
	flags.Z = (parameter == 0);
	pc += 2;
}

//	RST 
//	RST
void op_rst(uint8_t add, uint16_t& pc, uint16_t &sp, unsigned char memory[]) {
#ifdef DEBUG
	printf("RST");
#endif // DEBUG
	sp--;
	memory[sp] = (pc + 1) >> 8;
	sp--;
	memory[sp] = (pc + 1) & 0xff;
	pc = add;
}

//	SBC - L1 - T4 - Z1HC
//	Subtract (x + CF) from register A
void op_sbc(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("SBC");
#endif // DEBUG
	int newcf = (parameter + flags.C) > registers.A;
	flags.H = ((parameter & 0xf) + flags.C) > (registers.A & 0xf);
	registers.A = registers.A - (parameter + flags.C);
	flags.C = newcf;
	flags.Z = (registers.A == 0);
	flags.N = 1;
	pc += 1;
}

//	SBC - L2- T8 - Z1HC
//	Subtract (u8 + CF) from register A
void op_sbc_u8(unsigned char arg, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("SBC n");
#endif // DEBUG
	int newcf = (arg + flags.C) > registers.A;
	flags.H = ((arg & 0xf) + flags.C) > (registers.A & 0xf);
	registers.A = registers.A - (arg + flags.C);
	flags.C = newcf;
	flags.Z = (registers.A == 0);
	flags.N = 1;
	pc += 2;
}

//	SET - L2 - T8 - ////
//	Set Bit
int op_set(unsigned char& parameter, int bitnr, Flags& flags, Registers& registers, uint16_t& pc, int m = 2) {
#ifdef DEBUG
	printf("SET");
#endif // DEBUG
	parameter = parameter | (0x1 << bitnr);
	pc += 2;

	//	return m-cycles
	return m;
}

//	SLA - L2 - T8 - Z00C
//	Shift left into carry (bit 7 to CF, 0 to bit 0)
void op_sla(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("SLA");
#endif // DEBUG
	flags.C = (parameter >> 7) & 0x01;
	flags.N = 0;
	flags.H = 0;
	parameter = parameter << 1;
	flags.Z = (parameter == 0);
	pc += 2;
}

//	SRA - L2 - T8 - Z00C
//	Shift left into carry (bit 0 to CF, bit 7 to bit 7 [MSB remains same!])
void op_sra(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("SRA");
#endif // DEBUG
	int msb = (parameter >> 7) & 0x01;
	flags.C = parameter & 0x01;
	flags.N = 0;
	flags.H = 0;
	parameter = (parameter >> 1) | (msb << 7);
	flags.Z = (parameter == 0);
	pc += 2;
}

//	SRL - L2 - T8 - Z00C
//	Shift left into carry (bit 0 to CF, 0 to bit 7)
void op_srl(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("SRL");
#endif // DEBUG
	flags.C = parameter & 0x01;
	flags.N = 0;
	flags.H = 0;
	parameter = parameter >> 1;
	flags.Z = (parameter == 0);
	pc += 2;
}

//	SWAP - L2 - T8 - Z000
//	Swap high and low nibble
void op_swap(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("SWAP");
#endif // DEBUG
	flags.C = 0;
	flags.N = 0;
	flags.H = 0;
	parameter = ((parameter >> 4) & 0x0f) | ((parameter << 4) & 0xf0);
	flags.Z = (parameter == 0);
	pc += 2;
}

//	XOR
//	XOR
int op_xor(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc, int m = 1) {
#ifdef DEBUG
	printf("XOR A");
#endif // DEBUG
	registers.A = registers.A ^ parameter;
	flags.Z = (registers.A == 0) ? 1 : 0;
	flags.N = 0;
	flags.H = 0;
	flags.C = 0;
	pc += 1;

	//	return m-cycles
	return m;
}

int processOpcode(uint16_t& pc, uint16_t& sp, unsigned char memory[], Registers& registers, Flags& flags, int& interrupts_enabled) {
	opcode = memory[pc];

	switch (opcode) {

	//	mnemonic:		NOP
	//	length:			1
	//	cycles:			4
	//	flags:			/
	//	description:	NOP / no operation
	case 0x00:
		#ifdef DEBUG
		printf("NOP");
		#endif // DEBUG
		pc += 1;
		break;

	case 0x76:
		printf(">>HALT");
		flags.HALT = 1;
		pc += 1;
		break;

	//	mnemonic:		DI
	//	length:			1
	//	cycles:			4
	//	flags:			/
	//	description:	Disables Interrupts
	case 0xf3:
		#ifdef DEBUG
		printf("DI");
		#endif // DEBUG
		interrupts_enabled = 0;
		pc += 1;
		break;

	//	mnemonic:		EI
	//	length:			1
	//	cycles:			4
	//	flags:			/
	//	description:	Enables Interrupts
	case 0xfb:
		#ifdef DEBUG
		printf("EI");
		#endif // DEBUG
		interrupts_enabled = 1;
		pc += 1;
		break;

	//	RST
	case 0xc7: op_rst(0x00, pc, sp, memory); break;
	case 0xd7: op_rst(0x10, pc, sp, memory); break;
	case 0xe7: op_rst(0x20, pc, sp, memory); break;
	case 0xf7: op_rst(0x30, pc, sp, memory); break;
	case 0xcf: op_rst(0x08, pc, sp, memory); break;
	case 0xdf: op_rst(0x18, pc, sp, memory); break;
	case 0xef: op_rst(0x28, pc, sp, memory); break;
	case 0xff: op_rst(0x38, pc, sp, memory); break;

	//////////////////////////////////////////
	////////	ROT ops
	//////////////////////////////////////////

	//	mnemonic:		RLA
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - 0
	//					N - 0
	//					H - 0
	//					C - last bit 7 data - rotate THROUGH carry flag
	//	description:	Rotates A to the left
	case 0x17:
	{
		#ifdef DEBUG
		printf("RLA");
		#endif // DEBUG
		int oldcarry = flags.C;
		flags.C = registers.A >> 7 & 0x01;
		flags.Z = 0;
		flags.N = 0;
		flags.H = 0;
		registers.A = registers.A << 1 | oldcarry;
		pc += 1;
		break;
	}

	//	mnemonic:		RLCA
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - 0
	//					N - 0
	//					H - 0
	//					C - last bit 7 data - rotate TO carry flag
	//	description:	Rotates A to the left
	case 0x07:
	{
		#ifdef DEBUG
		printf("RLCA");
		#endif // DEBUG
		flags.C = registers.A >> 7 & 0x01;
		flags.Z = 0;
		flags.N = 0;
		flags.H = 0;
		registers.A = registers.A << 1 | flags.C;
		pc += 1;
		break;
	}


	//	mnemonic:		RRA
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - 0
	//					N - 0
	//					H - 0
	//					C - last bit 0 data - rotate THROUGH carry flag
	//	description:	Rotates A to the right
	case 0x1f:
	{
		#ifdef DEBUG
		printf("RRA");
		#endif // DEBUG
		int oldcarry = flags.C;
		flags.C = registers.A & 0x01;
		flags.Z = 0;
		flags.N = 0;
		flags.H = 0;
		registers.A = registers.A >> 1 | (oldcarry << 7);
		pc += 1;
		break;
	}

	//	mnemonic:		RRCA
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - 0
	//					N - 0
	//					H - 0
	//					C - last bit 0 data (carry on rotate)
	//	description:	Rotates A to the right, old bit 0 stored to carry flag (Only store TO carry flag, NOT THROUGH carry flag)
	case 0x0f:
	{
		#ifdef DEBUG
		printf("RRCA");
		#endif // DEBUG
		flags.C = registers.A & 1;
		flags.Z = 0;
		flags.N = 0;
		flags.H = 0;
		registers.A = registers.A >> 1 | (flags.C << 7);
		pc += 1;
		break;
	}




	//////////////////////////////////////////
	////////	PUSH / POP / RET ops
	//////////////////////////////////////////

	//	mnemonic:		POP BC
	//	length:			1
	//	cycles:			12
	//	flags:			/
	//	description:	Pop 2 bytes off the stack into BC, increse Stack Pointer twice
	case 0xc1:
		#ifdef DEBUG
		printf("POP BC");
		#endif // DEBUG
		registers.C = memory[sp];
		sp++;
		registers.B = memory[sp];
		sp++;
		pc += 1;
		break;

	//	mnemonic:		POP DE
	//	length:			1
	//	cycles:			12
	//	flags:			/
	//	description:	Pop 2 bytes off the stack into DE, increse Stack Pointer twice
	case 0xd1:
		#ifdef DEBUG
		printf("POP DE");
		#endif // DEBUG
		registers.E = memory[sp];
		sp++;
		registers.D = memory[sp];
		sp++;
		pc += 1;
		break;

	//	mnemonic:		POP HL
	//	length:			1
	//	cycles:			12
	//	flags:			/
	//	description:	Pop 2 bytes off the stack into HL, increse Stack Pointer twice
	case 0xe1:
		#ifdef DEBUG
		printf("POP HL");
		#endif // DEBUG
		registers.L = memory[sp];
		sp++;
		registers.H = memory[sp];
		sp++;
		pc += 1;
		break;

	//	mnemonic:		POP AF
	//	length:			1
	//	cycles:			12
	//	flags:			<all flags will be loaded back>
	//	description:	Pop 2 bytes off the stack into A, and FLAGS, increse Stack Pointer twice
	case 0xf1:
		#ifdef DEBUG
		printf("POP AF");
		#endif // DEBUG
		flags.Z = memory[sp] >> 7 & 0x1;
		flags.N = memory[sp] >> 6 & 0x1;
		flags.H = memory[sp] >> 5 & 0x1;
		flags.C = memory[sp] >> 4 & 0x1;
		sp++;
		registers.A = memory[sp];
		sp++;
		pc += 1;
		break;

	//	mnemonic:		PUSH BC
	//	length:			1
	//	cycles:			16
	//	flags:			/
	//	description:	Push BC to stack, decrement stack pointer twice (two values)
	case 0xc5:
		#ifdef DEBUG
		printf("PUSH BC");
		#endif // DEBUG
		sp--;
		memory[sp] = registers.B;
		sp--;
		memory[sp] = registers.C;
		pc += 1;
		break;

	//	mnemonic:		PUSH DE
	//	length:			1
	//	cycles:			16
	//	flags:			/
	//	description:	Push DE to stack, decrement stack pointer twice (two values)
	case 0xd5:
		#ifdef DEBUG
		printf("PUSH DE");
		#endif // DEBUG
		sp--;
		memory[sp] = registers.D;
		sp--;
		memory[sp] = registers.E;
		pc += 1;
		break;

	//	mnemonic:		PUSH HL
	//	length:			1
	//	cycles:			16
	//	flags:			/
	//	description:	Push HL to stack, decrement stack pointer twice (two values)
	case 0xe5:
		#ifdef DEBUG
		printf("PUSH HL");
		#endif // DEBUG
		sp--;
		memory[sp] = registers.H;
		sp--;
		memory[sp] = registers.L;
		pc += 1;
		break;

	//	mnemonic:		PUSH AF
	//	length:			1
	//	cycles:			16
	//	flags:			/
	//	description:	Push A and FLAGS to stack, decrement stack pointer twice (two values)
	case 0xf5:
		#ifdef DEBUG
		printf("PUSH AF");
		#endif // DEBUG
		sp--;
		memory[sp] = registers.A;
		sp--;
		memory[sp] = (flags.Z << 3 | flags.N << 2 | flags.H << 1 | flags.C) << 4 ;	//	push to high nibble
		pc += 1;
		break;

	//	mnemonic:		RET
	//	length:			1
	//	cycles:			16
	//	flags:			/
	//	description:	Pop 2 bytes from stack, and jump to that adress; increase SP by two afterwards
	case 0xc9:
		#ifdef DEBUG
		printf("RET 0x%04x", (memory[sp + 1] << 8) | memory[sp]);
		#endif // DEBUG
		pc = (memory[sp + 1] << 8) | memory[sp];
		sp += 2;
		break;

	//	mnemonic:		RETI
	//	length:			1
	//	cycles:			16
	//	flags:			/
	//	description:	Pop 2 bytes from stack, and jump to that adress; increase SP by two afterwards
	case 0xd9:
#ifdef DEBUG
		printf("RET 0x%04x", (memory[sp + 1] << 8) | memory[sp]);
#endif // DEBUG
		pc = (memory[sp + 1] << 8) | memory[sp];
		sp += 2;
		interrupts_enabled = 1;
		break;

	//	mnemonic:		RET C
	//	length:			1
	//	cycles:			8 / 20
	//	flags:			/
	//	description:	Pop 2 bytes from stack, and jump to that adress if CARRY FLAG is set; increase SP by two afterwards
	case 0xd8:
		#ifdef DEBUG
		printf("RET C 0x%04x", (memory[sp + 1] << 8) | memory[sp]);
		#endif // DEBUG
		if (flags.C == 1) {
			pc = (memory[sp + 1] << 8) | memory[sp];
			sp += 2;
		}
		else
		{
			pc += 1;
		}
		break;

	//	mnemonic:		RET NC
	//	length:			1
	//	cycles:			8 / 20
	//	flags:			/
	//	description:	Pop 2 bytes from stack, and jump to that adress if CARRY FLAG is NOT set; increase SP by two afterwards
	case 0xd0:
		#ifdef DEBUG
		printf("RET NC 0x%04x", (memory[sp + 1] << 8) | memory[sp]);
		#endif // DEBUG
		if (flags.C == 0) {
			pc = (memory[sp + 1] << 8) | memory[sp];
			sp += 2;
		}
		else
		{
			pc += 1;
		}
		break;

	//	mnemonic:		RET NZ
	//	length:			1
	//	cycles:			20 / 8
	//	flags:			/
	//	description:	Pop 2 bytes from stack, and jump to that adress if ZERO FLAG is set; increase SP by two afterwards
	case 0xc0:
		#ifdef DEBUG
		printf("RET NZ 0x%04x", (memory[sp + 1] << 8) | memory[sp]);
		#endif // DEBUG
		if (flags.Z == 0) {
			pc = (memory[sp + 1] << 8) | memory[sp];
			sp += 2;
		}
		else
		{
			pc += 1;
		}
		break;

	//	mnemonic:		RET Z
	//	length:			1
	//	cycles:			20 / 8
	//	flags:			/
	//	description:	Pop 2 bytes from stack, and jump to that adress if ZERO FLAG is NOT set; increase SP by two afterwards
	case 0xc8:
		#ifdef DEBUG
		printf("RET Z 0x%04x", (memory[sp + 1] << 8) | memory[sp]);
		#endif // DEBUG
		if (flags.Z == 1) {
			pc = (memory[sp + 1] << 8) | memory[sp];
			sp += 2;
		}
		else
		{
			pc += 1;
		}
		break;



	//////////////////////////////////////////
	////////	CALL ops
	//////////////////////////////////////////

	//	mnemonic:		CALL nn
	//	length:			3
	//	cycles:			24
	//	flags:			/
	//	description:	Push adress of next instruction (pc+3) onto the stack, and jump to nn
	case 0xcd:
		#ifdef DEBUG
		printf("CALL 0x%04x", ((memory[pc + 2] << 8) | memory[pc + 1]));
		#endif // DEBUG
		//	the following pushes the full 2 byte adress of the next instruction on the stack; e.g. 0x01fa -> sp[0] = 0x01, sp[1] = 0xfa
		sp--;
		memory[sp] = (pc + 3) >> 8;
		sp--;
		memory[sp] = (pc + 3) & 0xff;
		pc = ((memory[pc + 2] << 8) | memory[pc + 1]);
		break;

	//	mnemonic:		CALL NZ nn
	//	length:			3
	//	cycles:			24 / 12
	//	flags:			/
	//	description:	Push adress of next instruction (pc+3) onto the stack, and jump to nn, if NOT ZERO
	case 0xc4:
		#ifdef DEBUG
		printf("CALL NZ 0x%04x", ((memory[pc + 2] << 8) | memory[pc + 1]));
		#endif // DEBUG
		//	the following pushes the full 2 byte adress of the next instruction on the stack; e.g. 0x01fa -> sp[0] = 0x01, sp[1] = 0xfa
		if (flags.Z == 0) {
			sp--;
			memory[sp] = (pc + 3) >> 8;
			sp--;
			memory[sp] = (pc + 3) & 0xff;
			pc = ((memory[pc + 2] << 8) | memory[pc + 1]);
		}
		else
		{
			pc += 3;
		}
		break;

	//	mnemonic:		CALL Z nn
	//	length:			3
	//	cycles:			24 / 12
	//	flags:			/
	//	description:	Push adress of next instruction (pc+3) onto the stack, and jump to nn, if ZERO
	case 0xcc:
		#ifdef DEBUG
		printf("CALL Z 0x%04x", ((memory[pc + 2] << 8) | memory[pc + 1]));
		#endif // DEBUG
		//	the following pushes the full 2 byte adress of the next instruction on the stack; e.g. 0x01fa -> sp[0] = 0x01, sp[1] = 0xfa
		if (flags.Z == 1) {
			sp--;
			memory[sp] = (pc + 3) >> 8;
			sp--;
			memory[sp] = (pc + 3) & 0xff;
			pc = ((memory[pc + 2] << 8) | memory[pc + 1]);
		}
		else
		{
			pc += 3;
		}
		break;

	//	mnemonic:		CALL NC nn
	//	length:			3
	//	cycles:			24 / 12
	//	flags:			/
	//	description:	Push adress of next instruction (pc+3) onto the stack, and jump to nn, if CF not set
	case 0xd4:
		#ifdef DEBUG
		printf("CALL NC 0x%04x", ((memory[pc + 2] << 8) | memory[pc + 1]));
		#endif // DEBUG
		//	the following pushes the full 2 byte adress of the next instruction on the stack; e.g. 0x01fa -> sp[0] = 0x01, sp[1] = 0xfa
		if (flags.C == 0) {
			sp--;
			memory[sp] = (pc + 3) >> 8;
			sp--;
			memory[sp] = (pc + 3) & 0xff;
			pc = ((memory[pc + 2] << 8) | memory[pc + 1]);
		}
		else
		{
			pc += 3;
		}
		break;

	//	mnemonic:		CALL C nn
	//	length:			3
	//	cycles:			24 / 12
	//	flags:			/
	//	description:	Push adress of next instruction (pc+3) onto the stack, and jump to nn, if CF is set
	case 0xdc:
		#ifdef DEBUG
		printf("CALL NC 0x%04x", ((memory[pc + 2] << 8) | memory[pc + 1]));
		#endif // DEBUG
		//	the following pushes the full 2 byte adress of the next instruction on the stack; e.g. 0x01fa -> sp[0] = 0x01, sp[1] = 0xfa
		if (flags.C == 1) {
			sp--;
			memory[sp] = (pc + 3) >> 8;
			sp--;
			memory[sp] = (pc + 3) & 0xff;
			pc = ((memory[pc + 2] << 8) | memory[pc + 1]);
		}
		else
		{
			pc += 3;
		}
		break;




	//////////////////////////////////////////
	////////	INC / DEC ops
	//////////////////////////////////////////

	//	mnemonic:		DEC A
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 1
	//					H - if no borrow from bit 4 (left nibble)
	//					C - /
	//	description:	Decrements B by one
	case 0x3d:
		#ifdef DEBUG
		printf("DEC A");
		#endif // DEBUG
		flags.N = 1;
		flags.H = ((registers.A & 0xf) == 0x0) ? 1 : 0;
		registers.A -= 1;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		DEC B
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 1
	//					H - if no borrow from bit 4 (left nibble)
	//					C - /
	//	description:	Decrements B by one
	case 0x05:
		#ifdef DEBUG
		printf("DEC B");
		#endif // DEBUG
		flags.N = 1;
		flags.H = ((registers.B & 0xf) == 0x0) ? 1 : 0;
		registers.B -= 1;
		flags.Z = (registers.B == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		DEC C
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 1
	//					H - if no borrow from bit 4 (left nibble)
	//					C - /
	//	description:	Decrements C by one
	case 0x0d:
		#ifdef DEBUG
		printf("DEC C");
		#endif // DEBUG
		flags.N = 1;
		flags.H = ((registers.C & 0xf) == 0x0) ? 1 : 0;
		registers.C -= 1;
		flags.Z = (registers.C == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		DEC D
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 1
	//					H - if no borrow from bit 4 (left nibble)
	//					C - /
	//	description:	Decrements D by one
	case 0x15:
		#ifdef DEBUG
		printf("DEC D");
		#endif // DEBUG
		flags.N = 1;
		flags.H = ((registers.D & 0xf) == 0x0) ? 1 : 0;
		registers.D -= 1;
		flags.Z = (registers.D == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		DEC E
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 1
	//					H - if no borrow from bit 4 (left nibble)
	//					C - /
	//	description:	Decrements E by one
	case 0x1d:
		#ifdef DEBUG
		printf("DEC E");
		#endif // DEBUG
		flags.N = 1;
		flags.H = ((registers.E & 0xf) == 0x0) ? 1 : 0;
		registers.E -= 1;
		flags.Z = (registers.E == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		DEC H
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 1
	//					H - if no borrow from bit 4 (left nibble)
	//					C - /
	//	description:	Decrements H by one
	case 0x25:
		#ifdef DEBUG
		printf("DEC H");
		#endif // DEBUG
		flags.N = 1;
		flags.H = ((registers.H & 0xf) == 0x0) ? 1 : 0;
		registers.H -= 1;
		flags.Z = (registers.H == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		DEC L
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 1
	//					H - if no borrow from bit 4 (left nibble)
	//					C - /
	//	description:	Decrements L by one
	case 0x2d:
		#ifdef DEBUG
		printf("DEC L");
		#endif // DEBUG
		flags.N = 1;
		flags.H = ((registers.L & 0xf) == 0x0) ? 1 : 0;
		registers.L -= 1;
		flags.Z = (registers.L == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		DEC BC
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Decrements BC by one
	case 0x0b:
	{
		#ifdef DEBUG
		printf("DEC BC");
		#endif // DEBUG
		int val = (registers.B << 8) | registers.C;
		val--;
		registers.B = val >> 8 & 0xff;
		registers.C = val & 0xff;
		pc += 1;
		break;
	}

	//	mnemonic:		DEC DE
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Decrements DE by one
	case 0x1b:
	{
		#ifdef DEBUG
		printf("DEC DE");
		#endif // DEBUG
		int val = (registers.D << 8) | registers.E;
		val--;
		registers.D = val >> 8 & 0xff;
		registers.E = val & 0xff;
		pc += 1;
		break;
	}

	//	mnemonic:		DEC HL
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Decrements HL by one
	case 0x2b:
	{
		#ifdef DEBUG
		printf("DEC HL");
		#endif // DEBUG
		int val = (registers.H << 8) | registers.L;
		val--;
		registers.H = val >> 8 & 0xff;
		registers.L = val & 0xff;
		pc += 1;
		break;
	}

	//	mnemonic:		DEC SP
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Decrements SP by one
	case 0x3b:
	{
#ifdef DEBUG
		printf("DEC SP");
#endif // DEBUG
		sp--;
		pc += 1;
		break;
	}

	//	mnemonic:		DEC (HL)
	//	length:			1
	//	cycles:			12
	//	flags: 
	//					Z - if zero
	//					N - 1
	//					H - if no borrow from bit 4 (left nibble) [basically check if lower nibble == 0x0]
	//					C - unaffected
	//	description:	Decrements what's stored at HL
	case 0x35:
	{
		#ifdef DEBUG
		printf("DEC (HL)");
		#endif // DEBUG
		flags.H = ((memory[(registers.H << 8) | registers.L] & 0xf) == 0x0) ? 1 : 0;
		memory[(registers.H << 8) | registers.L] = memory[(registers.H << 8) | registers.L] - 1;
		flags.Z = (memory[(registers.H << 8) | registers.L] == 0) ? 1 : 0;
		flags.N = 1;
		pc += 1;
		break;
	}

	//	mnemonic:		INC (HL)
	//	length:			1
	//	cycles:			12
	//	flags: 
	//					Z - if zero
	//					N - 0
	//					H - if no borrow from bit 4 (left nibble) [basically check if lower nibble == 0x0]
	//					C - unaffected
	//	description:	Increments what's stored at HL
	case 0x34:
	{
#ifdef DEBUG
		printf("INC (HL)");
#endif // DEBUG
		flags.H = (memory[(registers.H << 8) | registers.L] & 0xf) == 0xf;
		memory[(registers.H << 8) | registers.L] = memory[(registers.H << 8) | registers.L] + 1;
		flags.Z = memory[(registers.H << 8) | registers.L] == 0;
		flags.N = 0;
		pc += 1;
		break;
	}

	//	mnemonic:		INC SP
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Increments SP by one
	case 0x33:
		#ifdef DEBUG
		printf("INC SP");
		#endif // DEBUG
		sp++;
		pc += 1;
		break;

	//	mnemonic:		INC A
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 0
	//					H - if carry bit from bit 3 (right nibble)
	//					C - /
	//	description:	Increments A by one
	case 0x3c:
		#ifdef DEBUG
		printf("INC A");
		#endif // DEBUG
		flags.N = 0;
		flags.H = ((registers.A & 0xf) == 0xf) ? 1 : 0;
		registers.A += 1;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		INC B
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 0
	//					H - if carry bit from bit 3 (right nibble)
	//					C - /
	//	description:	Increments B by one
	case 0x04:
		#ifdef DEBUG
		printf("INC B");
		#endif // DEBUG
		flags.N = 0;
		flags.H = ((registers.B & 0xf) == 0xf) ? 1 : 0;
		registers.B += 1;
		flags.Z = (registers.B == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		INC C
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 0
	//					H - if carry bit from bit 3 (right nibble)
	//					C - /
	//	description:	Increments C by one
	case 0x0c:
		#ifdef DEBUG
		printf("INC C");
		#endif // DEBUG
		flags.N = 0;
		flags.H = ((registers.C & 0xf) == 0xf) ? 1 : 0;
		registers.C += 1;
		flags.Z = (registers.C == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		INC D
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 0
	//					H - if carry bit from bit 3 (right nibble)
	//					C - /
	//	description:	Increments D by one
	case 0x14:
		#ifdef DEBUG
		printf("INC D");
		#endif // DEBUG
		flags.N = 0;
		flags.H = ((registers.D & 0xf) == 0xf) ? 1 : 0;
		registers.D += 1;
		flags.Z = (registers.D == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		INC E
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 0
	//					H - if carry bit from bit 3 (right nibble)
	//					C - /
	//	description:	Increments E by one
	case 0x1c:
		#ifdef DEBUG
		printf("INC E");
		#endif // DEBUG
		flags.N = 0;
		flags.H = ((registers.E & 0xf) == 0xf) ? 1 : 0;
		registers.E += 1;
		flags.Z = (registers.E == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		INC H
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 0
	//					H - if carry bit from bit 3 (right nibble)
	//					C - /
	//	description:	Increments H by one
	case 0x24:
		#ifdef DEBUG
		printf("INC H");
		#endif // DEBUG
		flags.N = 0;
		flags.H = ((registers.H & 0xf) == 0xf) ? 1 : 0;
		registers.H += 1;
		flags.Z = (registers.H == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		INC L
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - 0
	//					H - if carry bit from bit 3 (right nibble)
	//					C - /
	//	description:	Increments L by one
	case 0x2c:
		#ifdef DEBUG
		printf("INC L");
		#endif // DEBUG
		flags.N = 0;
		flags.H = ((registers.L & 0xf) == 0xf) ? 1 : 0;
		registers.L += 1;
		flags.Z = (registers.L == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		INC BC
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Increments BC by one
	case 0x03:
		{
			#ifdef DEBUG
			printf("INC BC");
			#endif // DEBUG
			int val = (registers.B << 8) | registers.C;
			val++;
			registers.B = val >> 8;
			registers.C = val & 0xFF;
			pc += 1;
			break;
		}

	//	mnemonic:		INC DE
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Increments DE by one
	case 0x13:
	{
		#ifdef DEBUG
		printf("INC DE");
		#endif // DEBUG
		int val = (registers.D << 8) | registers.E;
		val++;
		registers.D = val >> 8;
		registers.E = val & 0xFF;
		pc += 1;
		break;
	}

	//	mnemonic:		INC HL
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Increments HL by one
	case 0x23:
	{
		#ifdef DEBUG
		printf("INC HL");
		#endif // DEBUG
		int val = (registers.H << 8) | registers.L;
		val++;
		registers.H = val >> 8;
		registers.L = val & 0xFF;
		pc += 1;
		break;
	}

	//	mnemonic:		CPL
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - untouched
	//					N - set
	//					H - set
	//					C - untouched
	//	description:	Complements A (switch all bits)
	case 0x2f:
		#ifdef DEBUG
		printf("CPL");
		#endif // DEBUG
		registers.A = ~registers.A;
		flags.N = 1;
		flags.H = 1;
		pc += 1;
		break;

	//	mnemonic:		DAA
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - /
	//					H - reset
	//					C - if carry (?)
	//	description:	converts A into packed BCD (round each hex-position, back to 0-9 range)
	case 0x27:
	{
#ifdef DEBUG
		printf("DAA");
#endif // DEBUG
		//	addition before
		if (!flags.N) {
			if (flags.C || registers.A > 0x99) {
				registers.A += 0x60;
				flags.C = 1;
			}
			if (flags.H || (registers.A & 0x0f) > 0x09) {
				registers.A += 0x06;
			}
		}
		//	subtraction before
		else {
			if (flags.C)
				registers.A -= 0x60;
			if (flags.H)
				registers.A -= 0x06;
		}
		flags.Z = (registers.A == 0);
		flags.H = 0;
		pc += 1;
		break;
	}




	//////////////////////////////////////////
	////////	LD ops
	//////////////////////////////////////////

	//	mnemonic:		LD HL, SP + n 
	//	length:			2
	//	cycles:			12
	//	flags:			00HC
	//	description:	Put SP+n into HL
	case 0xf8:
		#ifdef DEBUG
		printf("LD HL, SP + n");
		#endif // DEBUG
		flags.H = ((sp & 0xf) + (memory[pc + 1] & 0xf)) > 0xf;
		flags.C = ((sp & 0xff) + memory[pc + 1]) > 0xff;
		registers.H = (sp + ((int8_t)memory[pc + 1])) >> 8;
		registers.L = (sp + ((int8_t)memory[pc + 1])) & 0xff;
		flags.Z = 0;
		flags.N = 0;
		pc += 2;
		break;

	//	mnemonic:		LDH (n), A
	//	length:			2
	//	cycles:			12
	//	flags:			/
	//	description:	Put A into the adress 0xFF00 + n
	case 0xe0:
		#ifdef DEBUG
		printf("LDH (n)[0x%04x], A", (0xFF00 + memory[pc + 1]));
		#endif // DEBUG
		memory[0xFF00 + memory[pc + 1]] = registers.A;
		pc += 2;
		break;

	//	mnemonic:		LDH A, (n)
	//	length:			2
	//	cycles:			12
	//	flags:			/
	//	description:	Put the adress 0xFF00 + n int A
	case 0xf0:
		#ifdef DEBUG
		printf("LDH A, (n)[0x%04x]", (0xFF00 + memory[pc + 1]));
		#endif // DEBUG
		registers.A = memory[0xFF00 + memory[pc + 1]];
		pc += 2;
		break;

	//	mnemonic:		LD (nn), A
	//	length:			3
	//	cycles:			16
	//	flags:			/
	//	description:	Put A into the adress nn
	case 0xea:
		#ifdef DEBUG
		printf("LD (nn)[0x%04x], A", (memory[pc + 2] << 8) | memory[pc + 1]);
		#endif // DEBUG
		memory[(memory[pc + 2] << 8) | memory[pc + 1]] = registers.A;
		pc += 3;
		break;

	//	mnemonic:		LD A, (nn)
	//	length:			3
	//	cycles:			16
	//	flags:			/
	//	description:	Put whats at the adress of nn into A
	case 0xfa:
		#ifdef DEBUG
		printf("LD A, (nn) [0x%04x]", memory[memory[pc + 2] << 8 | memory[pc + 1]]);
		#endif // DEBUG
		registers.A = memory[(memory[pc + 2] << 8) | memory[pc + 1]];
		pc += 3;
		break;

	//	mnemonic:		LD A, (HL)
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put whats at the adress of HL into A
	case 0x7e:
		#ifdef DEBUG
		printf("LD A, (HL)");
		#endif // DEBUG
		registers.A = memory[(registers.H << 8) | registers.L];
		pc += 1;
		break;

	//	mnemonic:		LD (HL), n
	//	length:			2
	//	cycles:			12
	//	flags:			/
	//	description:	Put n into the adress at HL
	case 0x36:
		#ifdef DEBUG
		printf("LD (HL), n [0x%02x]", pc + 1);
		#endif // DEBUG
		memory[(registers.H << 8) | registers.L] = memory[pc + 1];
		pc += 2;
		break;

	//	mnemonic:		LD A, (BC)
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put what is stored at BC's adress into A
	case 0x0a:
#ifdef DEBUG
		printf("LD A, (BC)");
#endif // DEBUG
		registers.A = memory[(registers.B << 8) | registers.C];
		pc += 1;
		break;

	//	mnemonic:		LD A, (DE)
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put what is stored at DE's adress into A
	case 0x1a:
		#ifdef DEBUG
		printf("LD A, (DE)");
		#endif // DEBUG
		registers.A = memory[(registers.D << 8) | registers.E];
		pc += 1;
		break;

	//	mnemonic:		LD A, (FF00+C)
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put what is stored at 0xFF00 + C adress into A
	case 0xf2:
#ifdef DEBUG
		printf("LD A, (FF00 + C)");
#endif // DEBUG
		registers.A = memory[0xff00 + registers.C];
		pc += 1;
		break;

	//	mnemonic:		LDI (HL), A
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put A into the adress stored at HL. Then increment HL
	case 0x22:
	{
		#ifdef DEBUG
		printf("LDI (HL), A");
		#endif // DEBUG
		memory[(registers.H << 8) | registers.L] = registers.A;
		uint16_t val = (registers.H << 8) | registers.L;
		val++;
		registers.H = val >> 8;
		registers.L = val & 0xFF;
		pc += 1;
		break;
	}

	//	mnemonic:		LD (FF00 + C), A
	//	length:			2 (should be 1? no following byte needed)
	//	cycles:			8
	//	flags:			/
	//	description:	Put A into the adress stored at 0xff00 + C
	case 0xe2:
		#ifdef DEBUG
		printf("LD (FF00 + C), A");
		#endif // DEBUG
		memory[0xff00 + registers.C] = registers.A;
		pc += 1;
		break;

	//	mnemonic:		LD B, n
	//	length:			2
	//	cycles:			8
	//	flags:			/
	//	description:	Put n into B
	case 0x06:
		#ifdef DEBUG
		printf("LD B, 0x%02x", memory[pc + 1]);
		#endif // DEBUG
		registers.B = memory[pc + 1];
		pc += 2;
		break;

	//	mnemonic:		LD B, (HL)
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put n into B
	case 0x46:
		#ifdef DEBUG
		printf("LD B, (HL)");
		#endif // DEBUG
		registers.B = memory[(registers.H << 8) | registers.L];
		pc += 1;
		break;

	//	mnemonic:		LD B
	case 0x47: op_ld(registers.B, registers.A, flags, registers, pc); break;
	case 0x40: op_ld(registers.B, registers.B, flags, registers, pc); break;
	case 0x41: op_ld(registers.B, registers.C, flags, registers, pc); break;
	case 0x42: op_ld(registers.B, registers.D, flags, registers, pc); break;
	case 0x43: op_ld(registers.B, registers.E, flags, registers, pc); break;
	case 0x44: op_ld(registers.B, registers.H, flags, registers, pc); break;
	case 0x45: op_ld(registers.B, registers.L, flags, registers, pc); break;

	//	mnemonic:		LD E, n
	//	length:			2
	//	cycles:			8
	//	flags:			/
	//	description:	Put n into E
	case 0x1e:
		#ifdef DEBUG
		printf("LD E, 0x%02x", memory[pc + 1]);
		#endif // DEBUG
		registers.E = memory[pc + 1];
		pc += 2;
		break;

	//	mnemonic:		LD E
	case 0x5f: op_ld(registers.E, registers.A, flags, registers, pc); break;
	case 0x58: op_ld(registers.E, registers.B, flags, registers, pc); break;
	case 0x59: op_ld(registers.E, registers.C, flags, registers, pc); break;
	case 0x5a: op_ld(registers.E, registers.D, flags, registers, pc); break;
	case 0x5b: op_ld(registers.E, registers.E, flags, registers, pc); break;
	case 0x5c: op_ld(registers.E, registers.H, flags, registers, pc); break;
	case 0x5d: op_ld(registers.E, registers.L, flags, registers, pc); break;

	//	mnemonic:		LD E, (HL)
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put what's at the adress of HL into E
	case 0x5e:
		#ifdef DEBUG
		printf("LD E, (HL)");
		#endif // DEBUG
		registers.E = memory[(registers.H << 8) | registers.L];
		pc += 1;
		break;

	//	mnemonic:		LD H, n
	//	length:			2
	//	cycles:			8
	//	flags:			/
	//	description:	Put n into H
	case 0x26:
		#ifdef DEBUG
		printf("LD H, 0x%02x", memory[pc + 1]);
		#endif // DEBUG
		registers.H = memory[pc + 1];
		pc += 2;
		break;

	//	mnemonic:		LD L, n
	//	length:			2
	//	cycles:			8
	//	flags:			/
	//	description:	Put n into L
	case 0x2e:
		#ifdef DEBUG
		printf("LD L, 0x%02x", memory[pc + 1]);
		#endif // DEBUG
		registers.L = memory[pc + 1];
		pc += 2;
		break;

	//	mnemonic:		LD L, A
	case 0x6f: op_ld(registers.L, registers.A, flags, registers, pc); break;
	case 0x68: op_ld(registers.L, registers.B, flags, registers, pc); break;
	case 0x69: op_ld(registers.L, registers.C, flags, registers, pc); break;
	case 0x6a: op_ld(registers.L, registers.D, flags, registers, pc); break;
	case 0x6b: op_ld(registers.L, registers.E, flags, registers, pc); break;
	case 0x6c: op_ld(registers.L, registers.H, flags, registers, pc); break;
	case 0x6d: op_ld(registers.L, registers.L, flags, registers, pc); break;

	//	mnemonic:		LD L, (HL)
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put whats stored at HL into L
	case 0x6e:
		#ifdef DEBUG
		printf("LD L, (HL)");
		#endif // DEBUG
		registers.L = memory[(registers.H << 8) | registers.L];
		pc += 1;
		break;

	//	mnemonic:		LD C
	case 0x4f: op_ld(registers.C, registers.A, flags, registers, pc); break;
	case 0x48: op_ld(registers.C, registers.B, flags, registers, pc); break;
	case 0x49: op_ld(registers.C, registers.C, flags, registers, pc); break;
	case 0x4a: op_ld(registers.C, registers.D, flags, registers, pc); break;
	case 0x4b: op_ld(registers.C, registers.E, flags, registers, pc); break;
	case 0x4c: op_ld(registers.C, registers.H, flags, registers, pc); break;
	case 0x4d: op_ld(registers.C, registers.L, flags, registers, pc); break;

	//	mnemonic:		LD C, (HL)
	//	length:			1
	//	cycles:			4
	//	flags:			/
	//	description:	Put what's stored in HL into C
	case 0x4e:
		#ifdef DEBUG
		printf("LD C, (HL)");
		#endif // DEBUG
		registers.C = memory[(registers.H << 8) | registers.L];
		pc += 1;
		break;

	//	mnemonic:		LD D
	case 0x57: op_ld(registers.D, registers.A, flags, registers, pc); break;
	case 0x50: op_ld(registers.D, registers.B, flags, registers, pc); break;
	case 0x51: op_ld(registers.D, registers.C, flags, registers, pc); break;
	case 0x52: op_ld(registers.D, registers.D, flags, registers, pc); break;
	case 0x53: op_ld(registers.D, registers.E, flags, registers, pc); break;
	case 0x54: op_ld(registers.D, registers.H, flags, registers, pc); break;
	case 0x55: op_ld(registers.D, registers.L, flags, registers, pc); break;

	//	mnemonic:		LD D, n
	//	length:			2
	//	cycles:			8
	//	flags:			/
	//	description:	Put n into D
	case 0x16:
		#ifdef DEBUG
		printf("LD D, n [0x%02x]", memory[pc + 1]);
		#endif // DEBUG
		registers.D = memory[pc + 1];
		pc += 2;
		break;

	//	mnemonic:		LD D, (HL)
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put what's at the adress of HL into D
	case 0x56:
		#ifdef DEBUG
		printf("LD D, (HL)");
		#endif // DEBUG
		registers.D = memory[(registers.H << 8) | registers.L];
		pc += 1;
		break;

	//	mnemonic:		LD H
	case 0x67: op_ld(registers.H, registers.A, flags, registers, pc); break;
	case 0x60: op_ld(registers.H, registers.B, flags, registers, pc); break;
	case 0x61: op_ld(registers.H, registers.C, flags, registers, pc); break;
	case 0x62: op_ld(registers.H, registers.D, flags, registers, pc); break;
	case 0x63: op_ld(registers.H, registers.E, flags, registers, pc); break;
	case 0x64: op_ld(registers.H, registers.H, flags, registers, pc); break;
	case 0x65: op_ld(registers.H, registers.L, flags, registers, pc); break;

	//	mnemonic:		LD H, (HL)
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put what's at the adress of HL into H
	case 0x66:
		#ifdef DEBUG
		printf("LD H, (HL)");
		#endif // DEBUG
		registers.H = memory[(registers.H << 8) | registers.L];
		pc += 1;
		break;

	//	mnemonic:		LD (HL), A
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put A into the adress stored at H+L
	case 0x77:
		#ifdef DEBUG
		printf("LD (HL), A");
		#endif // DEBUG
		memory[(registers.H << 8) | registers.L] = registers.A;
		pc += 1;
		break;

	//	mnemonic:		LD (HL), B
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put B into the adress stored at H+L
	case 0x70:
		#ifdef DEBUG
		printf("LD (HL), B");
		#endif // DEBUG
		memory[(registers.H << 8) | registers.L] = registers.B;
		pc += 1;
		break;

	//	mnemonic:		LD (HL), C
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put C into the adress stored at H+L
	case 0x71:
		#ifdef DEBUG
		printf("LD (HL), C");
		#endif // DEBUG
		memory[(registers.H << 8) | registers.L] = registers.C;
		pc += 1;
		break;

	//	mnemonic:		LD (HL), D
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put D into the adress stored at H+L
	case 0x72:
		#ifdef DEBUG
		printf("LD (HL), D");
		#endif // DEBUG
		memory[(registers.H << 8) | registers.L] = registers.D;
		pc += 1;
		break;

	//	mnemonic:		LD (HL), E
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put E into the adress stored at H+L
	case 0x73:
		#ifdef DEBUG
		printf("LD (HL), E");
		#endif // DEBUG
		memory[(registers.H << 8) | registers.L] = registers.E;
		pc += 1;
		break;

	//	mnemonic:		LD (HL), H
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put H into the adress stored at H+L
	case 0x74:
		#ifdef DEBUG
		printf("LD (HL), H");
		#endif // DEBUG
		memory[(registers.H << 8) | registers.L] = registers.H;
		pc += 1;
		break;

	//	mnemonic:		LD (HL), L
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Put L into the adress stored at H+L
	case 0x75:
		#ifdef DEBUG
		printf("LD (HL), L");
		#endif // DEBUG
		memory[(registers.H << 8) | registers.L] = registers.L;
		pc += 1;
		break;

	//	mnemonic:		LD A
	case 0x7f: op_ld(registers.A, registers.A, flags, registers, pc); break;
	case 0x78: op_ld(registers.A, registers.B, flags, registers, pc); break;
	case 0x79: op_ld(registers.A, registers.C, flags, registers, pc); break;
	case 0x7a: op_ld(registers.A, registers.D, flags, registers, pc); break;
	case 0x7b: op_ld(registers.A, registers.E, flags, registers, pc); break;
	case 0x7c: op_ld(registers.A, registers.H, flags, registers, pc); break;
	case 0x7d: op_ld(registers.A, registers.L, flags, registers, pc); break;

	//	mnemonic:		LD A, n
	//	length:			2
	//	cycles:			8
	//	flags:			/
	//	description:	Put n into A
	case 0x3e:
		#ifdef DEBUG
		printf("LD A, 0x%02x", memory[pc + 1]);
		#endif // DEBUG
		registers.A = memory[pc + 1];
		pc += 2;
		break;

	//	mnemonic:		LD C, n
	//	length:			2
	//	cycles:			8
	//	flags:			/
	//	description:	Put n into C
	case 0x0e:
		#ifdef DEBUG
		printf("LD C, 0x%02x", memory[pc + 1]);
		#endif // DEBUG
		registers.C = memory[pc + 1];
		pc += 2;
		break;

	//	mnemonic:		LD BC, nn
	//	length:			3
	//	cycles:			12
	//	flags:			/
	//	description:	Loads the next two bytes into BC
	case 0x01:
		#ifdef DEBUG
		printf("LD BC,0x%04x", (memory[pc + 2] << 8) | memory[pc + 1]);
		#endif // DEBUG
		registers.B = memory[pc + 2];
		registers.C = memory[pc + 1];
		pc += 3;
		break;

	//	mnemonic:		LD DE, nn
	//	length:			3
	//	cycles:			12
	//	flags:			/
	//	description:	Loads the next two bytes into DE
	case 0x11:
		#ifdef DEBUG
		printf("LD DE, %04x", (memory[pc + 2] << 8 | memory[pc + 1]));
		#endif // DEBUG
		registers.D = memory[pc + 2];
		registers.E = memory[pc + 1];
		pc += 3;
		break;

	//	mnemonic:		LD (BC), A
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Loads A into the adress of BC
	case 0x02:
#ifdef DEBUG
		printf("LD (BC), A");
#endif // DEBUG
		memory[(registers.B << 8) | registers.C] = registers.A;
		pc += 1;
		break;

	//	mnemonic:		LD (DE), A
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Loads A into the adress of DE
	case 0x12:
		#ifdef DEBUG
		printf("LD (DE), A");
		#endif // DEBUG
		memory[(registers.D << 8) | registers.E] = registers.A;
		pc += 1;
		break;

	//	mnemonic:		LD HL, nn
	//	length:			3
	//	cycles:			12
	//	flags:			/
	//	description:	Loads the next two bytes into HL
	case 0x21:
		#ifdef DEBUG
		printf("LD HL,0x%04x", (memory[pc + 2] << 8) | memory[pc + 1]);
		#endif // DEBUG
		registers.H = memory[pc + 2];
		registers.L = memory[pc + 1];
		pc += 3;
		break;

	//	mnemonic:		LD SP, nn
	//	length:			3
	//	cycles:			12
	//	flags:			/
	//	description:	Loads the next two bytes into SP (stackpointer) register
	case 0x31:
		#ifdef DEBUG
		printf("LD SP,0x%02x", (memory[pc + 2] << 8) | memory[pc + 1]);
		#endif // DEBUG
		sp = (memory[pc + 2] << 8) | memory[pc + 1];
		pc += 3;
		break;

	//	mnemonic:		LD SP, HL
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Loads HL into SP (stackpointer) register
	case 0xf9:
		#ifdef DEBUG
		printf("LD SP, HL");
		#endif // DEBUG
		sp = (registers.H << 8) | registers.L;
		pc += 1;
		break;

	//	mnemonic:		LDD (HL), A
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Loads A into the adress stored in HL, then decrements HL
	case 0x32:
	{
		#ifdef DEBUG
		printf("LDD (HL), A");
		#endif // DEBUG
		memory[(registers.H << 8) | registers.L] = registers.A;
		int val = (registers.H << 8) | registers.L;
		val--;
		registers.H = val >> 8;
		registers.L = val & 0xFF;
		pc += 1;
		break;
	}

	//	mnemonic:		LD A, (HL+)
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Puts value at adress HL into A, increments HL afterwards
	case 0x2a:
	{
		#ifdef DEBUG
		printf("LD A, (HL+)");
		#endif // DEBUG
		registers.A = memory[(registers.H << 8) | registers.L];
		int val = (registers.H << 8) | registers.L;
		val++;
		registers.H = val >> 8 & 0xff;
		registers.L = val & 0xff;
		pc += 1;
		break;
	}

	//	mnemonic:		LD A, (HL-)
	//	length:			1
	//	cycles:			8
	//	flags:			/
	//	description:	Puts value at adress HL into A, decrements HL afterwards
	case 0x3a:
	{
#ifdef DEBUG
		printf("LD A, (HL-)");
#endif // DEBUG
		registers.A = memory[(registers.H << 8) | registers.L];
		int val = (registers.H << 8) | registers.L;
		val--;
		registers.H = val >> 8 & 0xff;
		registers.L = val & 0xff;
		pc += 1;
		break;
	}

	//	mnemonic:		LD (nn), SP
	//	length:			3
	//	cycles:			20
	//	flags:			/
	//	description:	Loads SP into the adress stored in nn
	case 0x08:
	{
		#ifdef DEBUG
		printf("LD (nn)[0x%04x], SP", ((memory[pc + 2] << 8) | memory[pc + 1]));
		#endif // DEBUG
		memory[((memory[pc + 2] << 8) | memory[pc + 1])] = sp;
		memory[((memory[pc + 2] << 8) | memory[pc + 1]) + 1] = sp >> 8;
		pc += 3;
		break;
	}



	//////////////////////////////////////////
	////////	Set Flag instructions
	//////////////////////////////////////////

	//	mnemonic:		SCF
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - unmodified
	//					N - Unset
	//					H - Unset
	//					C - Set
	//	description:	Sets carry flag
	case 0x37:
		#ifdef DEBUG
		printf("SCF");
		#endif // DEBUG
		flags.N = 0;
		flags.H = 0;
		flags.C = 1;
		pc += 1;
		break;

	//	mnemonic:		CCF
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - unmodified
	//					N - Unset
	//					H - Unset
	//					C - Depends
	//	description:	Complements carry flag
	case 0x3f:
		#ifdef DEBUG
		printf("CCF");
		#endif // DEBUG
		flags.N = 0;
		flags.H = 0;
		flags.C = ~flags.C & 0x1;
		pc += 1;
		break;


	//////////////////////////////////////////
	////////	OR, XOR, AND, CP instructions
	//////////////////////////////////////////

	//	mnemonic:		CP n
	//	length:			2
	//	cycles:			8
	//	flags: 
	//					Z - if zero (A == n)
	//					N - Set
	//					H - Set if no borrow bit from bit 4
	//					C - Set if A < n
	//	description:	Compares A to n, result is thrown away (purely for flags)
	case 0xfe:
		#ifdef DEBUG
		printf("CP n [0x%02x]", memory[pc + 1]);
		#endif // DEBUG
		flags.Z = (registers.A == memory[pc + 1]) ? 1 : 0;
		flags.N = 1;
		flags.H = (registers.A & 0xF) < (memory[pc + 1] & 0xF);
		flags.C = (registers.A < memory[pc + 1]) ? 1 : 0;
		pc += 2;
		break;

	//	mnemonic:		CP
	case 0xbf: return op_cp(registers.A, registers.A, flags, registers, pc); break;
	case 0xb8: return op_cp(registers.A, registers.B, flags, registers, pc); break;
	case 0xb9: return op_cp(registers.A, registers.C, flags, registers, pc); break;
	case 0xba: return op_cp(registers.A, registers.D, flags, registers, pc); break;
	case 0xbb: return op_cp(registers.A, registers.E, flags, registers, pc); break;
	case 0xbc: return op_cp(registers.A, registers.H, flags, registers, pc); break;
	case 0xbd: return op_cp(registers.A, registers.L, flags, registers, pc); break;

	//	mnemonic:		CP (HL)
	//	length:			1
	//	cycles:			8
	//	flags: 
	//					Z - if zero (A == n)
	//					N - Set
	//					H - Set if no borrow bit from bit 4
	//					C - Set if A < n
	//	description:	Compares A to what's at the adress of HL, result is thrown away (purely for flags)
	case 0xbe:
		#ifdef DEBUG
		printf("CP (HL)");
		#endif // DEBUG
		flags.Z = (registers.A == memory[(registers.H << 8) | registers.L]) ? 1 : 0;
		flags.N = 1;
		flags.H = (registers.A & 0xF) < (memory[(registers.H << 8) | registers.L] & 0xF);
		flags.C = (registers.A < memory[(registers.H << 8) | registers.L]) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		XOR n
	//	length:			2
	//	cycles:			8
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Reset
	//					C - Reset
	//	description:	XORs register A with n, result in A
	case 0xee:
		#ifdef DEBUG
		printf("XOR n");
		#endif // DEBUG
		registers.A = registers.A ^ memory[pc + 1];
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 0;
		flags.C = 0;
		pc += 2;
		break;

	//	mnemonic:		XOR
	case 0xaf: return op_xor(registers.A, flags, registers, pc); break;
	case 0xa8: return op_xor(registers.B, flags, registers, pc); break;
	case 0xa9: return op_xor(registers.C, flags, registers, pc); break;
	case 0xaa: return op_xor(registers.D, flags, registers, pc); break;
	case 0xab: return op_xor(registers.E, flags, registers, pc); break;
	case 0xac: return op_xor(registers.H, flags, registers, pc); break;
	case 0xad: return op_xor(registers.L, flags, registers, pc); break;
	case 0xae: return op_xor(memory[(registers.H << 8) | registers.L], flags, registers, pc, 2);

	//	mnemonic:		OR (HL)
	//	length:			1
	//	cycles:			8
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Reset
	//					C - Reset
	//	description:	ORs register A with register A, result in A
	case 0xb6:
		#ifdef DEBUG
		printf("OR (HL)");
		#endif // DEBUG
		registers.A = registers.A | memory[(registers.H << 8) | registers.L];
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 0;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		OR n
	//	length:			2
	//	cycles:			8
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Reset
	//					C - Reset
	//	description:	ORs n with register A, result in A
	case 0xf6:
		#ifdef DEBUG
		printf("OR n (with A)");
		#endif // DEBUG
		registers.A = registers.A | memory[pc + 1];
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 0;
		flags.C = 0;
		pc += 2;
		break;

	//	mnemonic:		OR A
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Reset
	//					C - Reset
	//	description:	ORs register A with register A, result in A
	case 0xb7:
		#ifdef DEBUG
		printf("OR A (with A)");
		#endif // DEBUG
		registers.A = registers.A | registers.A;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 0;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		OR B
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Reset
	//					C - Reset
	//	description:	ORs register A with register B, result in A
	case 0xb0:
		#ifdef DEBUG
		printf("OR B (with A)");
		#endif // DEBUG
		registers.A = registers.A | registers.B;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 0;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		OR C
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Reset
	//					C - Reset
	//	description:	ORs register A with register C, result in A
	case 0xb1:
		#ifdef DEBUG
		printf("OR C (with A)");
		#endif // DEBUG
		registers.A = registers.A | registers.C;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 0;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		OR D
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Reset
	//					C - Reset
	//	description:	ORs register A with register D, result in A
	case 0xb2:
		#ifdef DEBUG
		printf("OR D (with A)");
		#endif // DEBUG
		registers.A = registers.A | registers.D;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 0;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		OR E
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Reset
	//					C - Reset
	//	description:	ORs register A with register E, result in A
	case 0xb3:
		#ifdef DEBUG
		printf("OR E (with A)");
		#endif // DEBUG
		registers.A = registers.A | registers.E;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 0;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		OR H
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Reset
	//					C - Reset
	//	description:	ORs register A with register H, result in A
	case 0xb4:
		#ifdef DEBUG
		printf("OR H (with A)");
		#endif // DEBUG
		registers.A = registers.A | registers.H;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 0;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		OR L
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Reset
	//					C - Reset
	//	description:	ORs register A with register L, result in A
	case 0xb5:
		#ifdef DEBUG
		printf("OR L (with A)");
		#endif // DEBUG
		registers.A = registers.A | registers.L;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 0;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		AND A
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Set
	//					C - Reset
	//	description:	ANDs register A with register A
	case 0xa7:
		#ifdef DEBUG
		printf("AND A");
		#endif // DEBUG
		registers.A = registers.A & registers.A;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 1;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		AND B
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Set
	//					C - Reset
	//	description:	ANDs register A with register B
	case 0xa0:
#ifdef DEBUG
		printf("AND B");
#endif // DEBUG
		registers.A = registers.A & registers.B;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 1;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		AND C
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Set
	//					C - Reset
	//	description:	ANDs register A with register C
	case 0xa1:
		#ifdef DEBUG
		printf("AND C");
		#endif // DEBUG
		registers.A = registers.A & registers.C;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 1;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		AND D
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Set
	//					C - Reset
	//	description:	ANDs register A with register D
	case 0xa2:
		#ifdef DEBUG
		printf("AND D");
		#endif // DEBUG
		registers.A = registers.A & registers.D;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 1;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		AND E
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Set
	//					C - Reset
	//	description:	ANDs register A with register E
	case 0xa3:
		#ifdef DEBUG
		printf("AND E");
		#endif // DEBUG
		registers.A = registers.A & registers.E;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 1;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		AND H
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Set
	//					C - Reset
	//	description:	ANDs register A with register H
	case 0xa4:
		#ifdef DEBUG
		printf("AND H");
		#endif // DEBUG
		registers.A = registers.A & registers.H;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 1;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		AND L
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Set
	//					C - Reset
	//	description:	ANDs register A with register L
	case 0xa5:
		#ifdef DEBUG
		printf("AND L");
		#endif // DEBUG
		registers.A = registers.A & registers.L;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 1;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		AND (HL)
	//	length:			1
	//	cycles:			4
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Set
	//					C - Reset
	//	description:	ANDs register A with (HL)
	case 0xa6:
#ifdef DEBUG
		printf("AND (HL)");
#endif // DEBUG
		registers.A = registers.A & memory[(registers.H << 8) | registers.L];
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 1;
		flags.C = 0;
		pc += 1;
		break;

	//	mnemonic:		AND n
	//	length:			2
	//	cycles:			8
	//	flags: 
	//					Z - if zero
	//					N - Reset
	//					H - Set
	//					C - Reset
	//	description:	ANDs register A with n
	case 0xe6:
		#ifdef DEBUG
		printf("AND 0x%02x", memory[pc + 1]);
		#endif // DEBUG
		registers.A = registers.A & memory[pc + 1];
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 0;
		flags.H = 1;
		flags.C = 0;
		pc += 2;
		break;



	//////////////////////////////////////////
	////////	ADD, SUB instructions
	//////////////////////////////////////////

	//	mnemonic:		ADD A, A
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds A to A
	case 0x87:
		#ifdef DEBUG
		printf("ADD A, A");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.A & 0xf) + (registers.A & 0xf)) > 0xf);
		flags.C = ((registers.A + registers.A) > 0xff);
		registers.A += registers.A;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		ADD A, B
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds B to A
	case 0x80:
		#ifdef DEBUG
		printf("ADD A, B");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.A & 0xf) + (registers.B & 0xf)) > 0xf);
		flags.C = ((registers.A + registers.B) > 0xff);
		registers.A += registers.B;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		ADD A, C
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds C to A
	case 0x81:
		#ifdef DEBUG
		printf("ADD A, C");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.A & 0xf) + (registers.C & 0xf)) > 0xf);
		flags.C = ((registers.A + registers.C) > 0xff);
		registers.A += registers.C;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		ADD A, D
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds D to A
	case 0x82:
		#ifdef DEBUG
		printf("ADD A, D");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.A & 0xf) + (registers.D & 0xf)) > 0xf);
		flags.C = ((registers.A + registers.D) > 0xff);
		registers.A += registers.D;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		ADD A, E
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds E to A
	case 0x83:
		#ifdef DEBUG
		printf("ADD A, E");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.A & 0xf) + (registers.E & 0xf)) > 0xf);
		flags.C = ((registers.A + registers.E) > 0xff);
		registers.A += registers.E;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		ADD A, H
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds H to A
	case 0x84:
		#ifdef DEBUG
		printf("ADD A, H");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.A & 0xf) + (registers.H & 0xf)) > 0xf);
		flags.C = ((registers.A + registers.H) > 0xff);
		registers.A += registers.H;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		ADD A, L
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds L to A
	case 0x85:
		#ifdef DEBUG
		printf("ADD A, L");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.A & 0xf) + (registers.L & 0xf)) > 0xf);
		flags.C = ((registers.A + registers.L) > 0xff);
		registers.A += registers.L;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;

	//	mnemonic:		ADD A, (HL)
	//	length:			1
	//	cycles:			8
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds what is stored at the adress of (HL) to A
	case 0x86:
	{
		#ifdef DEBUG
		printf("ADD A, (HL)");
		#endif // DEBUG
		int val = memory[(registers.H << 8) | registers.L];
		flags.N = 0;
		flags.H = (((registers.A & 0xf) + (memory[(registers.H << 8) | registers.L] & 0xf)) > 0xf);
		flags.C = ((registers.A + memory[(registers.H << 8) | registers.L]) > 0xff);
		registers.A += val;
		flags.Z = (registers.A == 0x00) ? 1 : 0;	//	TODO - THIS is the right approach. There are binary additions, that still end in 0x00, so always check for 0x00 IN THE END!
													//	TODO - ^ Apply for every other opcode case which uses the Z-flag
		pc += 1;
		break;
	}

	//	mnemonic:		ADD A, n
	//	length:			2
	//	cycles:			8
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds n to A
	case 0xc6:
		#ifdef DEBUG
		printf("ADD A, n");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.A & 0xf) + (memory[pc + 1] & 0xf)) > 0xf);
		flags.C = ((registers.A + memory[pc + 1]) > 0xff);
		registers.A += memory[pc + 1];
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 2;
		break;

	//	mnemonic:		ADD HL, BC
	//	length:			1
	//	cycles:			8
	//	flags:			
	//					Z - unaffected
	//					N - 0
	//					H - set if carry from bit 11 (MSB of lower nibble of high byte)
	//					C - set if carry from bit 15 (MSB)
	//	description:	Adds BC to HL
	case 0x09:
	{
		#ifdef DEBUG
		printf("ADD HL, BC");
		#endif // DEBUG
		int val = registers.B << 8 | registers.C;
		int tar = registers.H << 8 | registers.L;
		flags.N = 0;
		flags.H = ((((val & 0xfff) + (tar & 0xfff)) & 0x1000) == 0x1000) ? 1 : 0;	//	mask both values up to bit 11, add, mask the (half-carry) Uebertrag, check if set
		flags.C = ((val + tar) > 0xffff) ? 1 : 0;
		registers.H = (val + tar) >> 8;
		registers.L = (val + tar) & 0xff;
		pc += 1;
		break;
	}

	//	mnemonic:		ADD HL, DE
	//	length:			1
	//	cycles:			8
	//	flags:			
	//					Z - unaffected
	//					N - 0
	//					H - set if carry from bit 11 (MSB of lower nibble of high byte)
	//					C - set if carry from bit 15 (MSB)
	//	description:	Adds DE to HL
	case 0x19:
	{
		#ifdef DEBUG
		printf("ADD HL, DE");
		#endif // DEBUG
		int val = registers.D << 8 | registers.E;
		int tar = registers.H << 8 | registers.L;
		flags.N = 0;
		flags.H = ((((val & 0xfff) + (tar & 0xfff)) & 0x1000) == 0x1000) ? 1 : 0;	//	mask both values up to bit 11, add, mask the (half-carry) Uebertrag, check if set
		flags.C = ((val + tar) > 0xffff) ? 1 : 0;
		registers.H = (val + tar) >> 8;
		registers.L = (val + tar) & 0xff;
		pc += 1;
		break;
	}

	//	mnemonic:		ADD HL, HL
	//	length:			1
	//	cycles:			8
	//	flags:			
	//					Z - unaffected
	//					N - 0
	//					H - set if carry from bit 11 (MSB of lower nibble of high byte)
	//					C - set if carry from bit 15 (MSB)
	//	description:	Adds HL to HL
	case 0x29:
	{
		#ifdef DEBUG
		printf("ADD HL, HL");
		#endif // DEBUG
		int val = registers.H << 8 | registers.L;
		int tar = registers.H << 8 | registers.L;
		flags.N = 0;
		flags.H = ((((val & 0xfff) + (tar & 0xfff)) & 0x1000) == 0x1000) ? 1 : 0;	//	mask both values up to bit 11, add, mask the (half-carry) Uebertrag, check if set
		flags.C = ((val + tar) > 0xffff) ? 1 : 0;
		registers.H = (val + tar) >> 8;
		registers.L = (val + tar) & 0xff;
		pc += 1;
		break;
	}

	//	mnemonic:		ADD HL, SP
	//	length:			1
	//	cycles:			8
	//	flags:			
	//					Z - unaffected
	//					N - 0
	//					H - set if carry from bit 11 (MSB of lower nibble of high byte)
	//					C - set if carry from bit 15 (MSB)
	//	description:	Adds SP to HL
	case 0x39:
	{
#ifdef DEBUG
		printf("ADD HL, SP");
#endif // DEBUG
		int val = sp;
		int tar = registers.H << 8 | registers.L;
		flags.N = 0;
		flags.H = ((((val & 0xfff) + (tar & 0xfff)) & 0x1000) == 0x1000) ? 1 : 0;	//	mask both values up to bit 11, add, mask the (half-carry) Uebertrag, check if set
		flags.C = ((val + tar) > 0xffff) ? 1 : 0;
		registers.H = (val + tar) >> 8;
		registers.L = (val + tar) & 0xff;
		pc += 1;
		break;
	}

	//	mnemonic:		ADD SP, n
	//	length:			1
	//	cycles:			8
	//	flags:			
	//					Z - 0
	//					N - 0
	//					H - set if appropriate operation
	//					C - set if appropriate operation
	//	description:	Adds n to SP (n is SIGNED)
	case 0xe8:
	{
#ifdef DEBUG
		printf("ADD SP, n");
#endif // DEBUG
		flags.Z = 0;
		flags.N = 0;
		int8_t offset = (int8_t)memory[pc + 1];
		flags.H = ((sp & 0xf) + (memory[pc + 1] & 0xf)) > 0xf;
		flags.C = ((sp & 0xff) + memory[pc + 1]) > 0xff;
		sp += offset;
		pc += 2;
		break;
	}

	//	mnemonic:		ADC A, n
	//	length:			2
	//	cycles:			8
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds n to A plus carry
	case 0xce:
	{
		#ifdef DEBUG
		printf("ADC A, n");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((memory[pc + 1] & 0xf) + (registers.A & 0xf) + flags.C) > 0xf) ? 1 : 0;
		int oldcarry = ((memory[pc + 1] + registers.A + flags.C) > 0xff) ? 1 : 0;
		registers.A += (memory[pc + 1] + flags.C);
		flags.C = oldcarry;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 2;
		break;
	}

	//	mnemonic:		ADC A, A
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds A to A plus carry
	case 0x8f:
	{
		#ifdef DEBUG
		printf("ADC A, A");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.A & 0xf) + (registers.A & 0xf) + flags.C) > 0xf) ? 1 : 0;
		int oldcarry = ((registers.A + registers.A + flags.C) > 0xff) ? 1 : 0;
		registers.A += registers.A + flags.C;
		flags.C = oldcarry;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;
	}

	//	mnemonic:		ADC A, B
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds B to A plus carry
	case 0x88:
	{
		#ifdef DEBUG
		printf("ADC A, B");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.B & 0xf) + (registers.A & 0xf) + flags.C) > 0xf) ? 1 : 0;
		int oldcarry = ((registers.B + registers.A + flags.C) > 0xff) ? 1 : 0;
		registers.A += registers.B + flags.C;
		flags.C = oldcarry;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;
	}

	//	mnemonic:		ADC A, C
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds C to A plus carry
	case 0x89:
	{
		#ifdef DEBUG
		printf("ADC A, C");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.C & 0xf) + (registers.A & 0xf) + flags.C) > 0xf) ? 1 : 0;
		int oldcarry = ((registers.C + registers.A + flags.C) > 0xff) ? 1 : 0;
		registers.A += registers.C + flags.C;
		flags.C = oldcarry;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;
	}


	//	mnemonic:		ADC A, D
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds D to A plus carry
	case 0x8a:
	{
		#ifdef DEBUG
		printf("ADC A, D");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.D & 0xf) + (registers.A & 0xf) + flags.C) > 0xf) ? 1 : 0;
		int oldcarry = ((registers.D + registers.A + flags.C) > 0xff) ? 1 : 0;
		registers.A += registers.D + flags.C;
		flags.C = oldcarry;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;
	}

	//	mnemonic:		ADC A, E
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds E to A plus carry
	case 0x8b:
	{
		#ifdef DEBUG
		printf("ADC A, E");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.E & 0xf) + (registers.A & 0xf) + flags.C) > 0xf) ? 1 : 0;
		int oldcarry = ((registers.E + registers.A + flags.C) > 0xff) ? 1 : 0;
		registers.A += registers.E + flags.C;
		flags.C = oldcarry;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;
	}

	//	mnemonic:		ADC A, H
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds H to A plus carry
	case 0x8c:
	{
		#ifdef DEBUG
		printf("ADC A, H");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.H & 0xf) + (registers.A & 0xf) + flags.C) > 0xf) ? 1 : 0;
		int oldcarry = ((registers.H + registers.A + flags.C) > 0xff) ? 1 : 0;
		registers.A += registers.H + flags.C;
		flags.C = oldcarry;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;
	}

	//	mnemonic:		ADC A, L
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds L to A plus carry
	case 0x8d:
	{
		#ifdef DEBUG
		printf("ADC A, L");
		#endif // DEBUG
		flags.N = 0;
		flags.H = (((registers.L & 0xf) + (registers.A & 0xf) + flags.C) > 0xf) ? 1 : 0;
		int oldcarry = ((registers.L + registers.A + flags.C) > 0xff) ? 1 : 0;
		registers.A += registers.L + flags.C;
		flags.C = oldcarry;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;
	}

	//	mnemonic:		ADC A, (HL)
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - 0
	//					H - set if carry from bit 3
	//					C - set if carry from bit 7
	//	description:	Adds L to A plus carry
	case 0x8e:
	{
#ifdef DEBUG
		printf("ADC A, (HL)");
#endif // DEBUG
		flags.N = 0;
		flags.H = (((memory[(registers.H << 8) | registers.L] & 0xf) + (registers.A & 0xf) + flags.C) > 0xf) ? 1 : 0;
		int oldcarry = ((memory[(registers.H << 8) | registers.L] + registers.A + flags.C) > 0xff) ? 1 : 0;
		registers.A += memory[(registers.H << 8) | registers.L] + flags.C;
		flags.C = oldcarry;
		flags.Z = (registers.A == 0) ? 1 : 0;
		pc += 1;
		break;
	}

	//	mnemonic:		SUB B
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - Set
	//					H - if borrow bit from bit 4
	//					C - if borrow
	//	description:	Subtracts B from A
	case 0x90:
		#ifdef DEBUG
		printf("SUB B");
		#endif // DEBUG
		flags.C = (registers.B > registers.A) ? 1 : 0;
		flags.H = (registers.B & 0xf) > (registers.A & 0xf);
		registers.A = registers.A - registers.B;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 1;
		pc += 1;
		break;

	//	mnemonic:		SUB C
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - Set
	//					H - if borrow bit from bit 4
	//					C - if borrow
	//	description:	Subtracts C from A
	case 0x91:
		#ifdef DEBUG
		printf("SUB C");
		#endif // DEBUG
		flags.C = (registers.C > registers.A) ? 1 : 0;
		flags.H = (registers.C & 0xf) > (registers.A & 0xf);
		registers.A = registers.A - registers.C;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 1;
		pc += 1;
		break;

	//	mnemonic:		SUB D
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - Set
	//					H - if borrow bit from bit 4
	//					C - if borrow
	//	description:	Subtracts D from A
	case 0x92:
		#ifdef DEBUG
		printf("SUB D");
		#endif // DEBUG
		flags.C = (registers.D > registers.A) ? 1 : 0;
		flags.H = (registers.D & 0xf) > (registers.A & 0xf);
		registers.A = registers.A - registers.D;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 1;
		pc += 1;
		break;

	//	mnemonic:		SUB E
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - Set
	//					H - if borrow bit from bit 4
	//					C - if borrow
	//	description:	Subtracts E from A
	case 0x93:
		#ifdef DEBUG
		printf("SUB E");
		#endif // DEBUG
		flags.C = (registers.E > registers.A) ? 1 : 0;
		flags.H = (registers.E & 0xf) > (registers.A & 0xf);
		registers.A = registers.A - registers.E;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 1;
		pc += 1;
		break;

	//	mnemonic:		SUB H
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - Set
	//					H - if borrow bit from bit 4
	//					C - if borrow
	//	description:	Subtracts H from A
	case 0x94:
		#ifdef DEBUG
		printf("SUB H");
		#endif // DEBUG
		flags.C = (registers.H > registers.A) ? 1 : 0;
		flags.H = (registers.H & 0xf) > (registers.A & 0xf);
		registers.A = registers.A - registers.H;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 1;
		pc += 1;
		break;

	//	mnemonic:		SUB L
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - Set
	//					H - if borrow bit from bit 4
	//					C - if borrow
	//	description:	Subtracts L from A
	case 0x95:
		#ifdef DEBUG
		printf("SUB L");
		#endif // DEBUG
		flags.C = (registers.L > registers.A) ? 1 : 0;
		flags.H = (registers.L & 0xf) > (registers.A & 0xf);
		registers.A = registers.A - registers.L;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 1;
		pc += 1;
		break;

	//	mnemonic:		SUB (HL)
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - Set
	//					H - if borrow bit from bit 4
	//					C - if borrow
	//	description:	Subtracts (HL) from A
	case 0x96:
#ifdef DEBUG
		printf("SUB (HL)");
#endif // DEBUG
		flags.C = (memory[(registers.H << 8) | registers.L] > registers.A) ? 1 : 0;
		flags.H = (memory[(registers.H << 8) | registers.L] & 0xf) > (registers.A & 0xf);
		registers.A = registers.A - memory[(registers.H << 8) | registers.L];
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 1;
		pc += 1;
		break;

	//	mnemonic:		SUB A
	//	length:			1
	//	cycles:			4
	//	flags:			
	//					Z - if result is zero
	//					N - Set
	//					H - if borrow bit from bit 4
	//					C - if borrow
	//	description:	Subtracts A from A
	case 0x97:
		#ifdef DEBUG
		printf("SUB A");
		#endif // DEBUG
		flags.C = (registers.A > registers.A) ? 1 : 0;
		flags.H = (registers.A & 0xf) > (registers.A & 0xf);
		registers.A = registers.A - registers.A;
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 1;
		pc += 1;
		break;

	//	mnemonic:		SUB n
	//	length:			2
	//	cycles:			8
	//	flags:			
	//					Z - if result is zero
	//					N - Set
	//					H - if borrow bit from bit 4
	//					C - if borrow
	//	description:	Subtracts n from A
	case 0xd6:
		#ifdef DEBUG
		printf("SUB n");
		#endif // DEBUG
		flags.C = (memory[pc + 1] > registers.A) ? 1 : 0;
		flags.H = (memory[pc + 1] & 0xf) > (registers.A & 0xf);
		registers.A = registers.A - memory[pc + 1];
		flags.Z = (registers.A == 0) ? 1 : 0;
		flags.N = 1;
		pc += 2;
		break;

	//	mnemonic:		SBC B - L1 - T4 - Z1HC
	//	description:	Subtracts B and carry flag from A
	case 0x98: op_sbc(registers.B, flags, registers, pc); break;
	case 0x99: op_sbc(registers.C, flags, registers, pc); break;
	case 0x9a: op_sbc(registers.D, flags, registers, pc); break;
	case 0x9b: op_sbc(registers.E, flags, registers, pc); break;
	case 0x9c: op_sbc(registers.H, flags, registers, pc); break;
	case 0x9d: op_sbc(registers.L, flags, registers, pc); break;
	case 0x9f: op_sbc(registers.A, flags, registers, pc); break;
	case 0x9e: op_sbc(memory[(registers.H << 8) | registers.L], flags, registers, pc); break;
	case 0xde: op_sbc_u8(memory[pc + 1], flags, registers, pc); break;




	//////////////////////////////////////////
	////////	JMP instructions
	//////////////////////////////////////////

	//	mnemonic:		JP nn
	//	length:			3
	//	cycles:			16
	//	flags:			/
	//	description:	Jump to adress nn
	case 0xc3:
		#ifdef DEBUG
		printf("JP 0x%04x", ((memory[pc+2] << 8) | memory[pc + 1] ));
		#endif // DEBUG
		pc = ((memory[pc + 2] << 8) | (memory[pc + 1] & 0xff));
		break;

	//	mnemonic:		JR r
	//	length:			2
	//	cycles:			12
	//	flags:			/
	//	description:	add r to current adress, and jump to it; r is a SIGNED number! so positive and negative jumps are possible
	case 0x18:
	{
		#ifdef DEBUG
		printf("JR r");
		#endif // DEBUG
		pc += 2 + (int8_t)memory[pc + 1];
		break;
	}

	//	mnemonic:		JR NZ, r
	//	length:			2
	//	cycles:			12/8
	//	flags:			/
	//	description:	if Z-Flag is reset / 0, add r to the current adress, and jump to it; r is a SIGNED number! so positive and negative jumps are possible
	case 0x20:
		#ifdef DEBUG
		printf("JR NZ, 0x%02x", memory[pc + 1]);
		#endif // DEBUG
		if (flags.Z == 0)
			pc += 2 + (int8_t)(memory[pc + 1]);	//	Signed number
		else
			pc += 2;
		break;

	//	mnemonic:		JR NC, r
	//	length:			2
	//	cycles:			12/8
	//	flags:			/
	//	description:	if C-Flag is reset / 0, add r to the current adress, and jump to it; r is a SIGNED number! so positive and negative jumps are possible
	case 0x30:
		#ifdef DEBUG
		printf("JR NC, 0x%02x", memory[pc + 1]);
		#endif // DEBUG
		if (flags.C == 0)
			pc += 2 + (int8_t)(memory[pc + 1]);	//	Signed number
		else
			pc += 2;
		break;

	//	mnemonic:		JR C, r
	//	length:			2
	//	cycles:			12/8
	//	flags:			/
	//	description:	if C-Flag is set / 1, add r to the current adress, and jump to it; r is a SIGNED number! so positive and negative jumps are possible
	case 0x38:
		#ifdef DEBUG
		printf("JR C, 0x%02x", memory[pc + 1]);
		#endif // DEBUG
		if (flags.C == 1)
			pc += 2 + (int8_t)(memory[pc + 1]);	//	Signed number
		else
			pc += 2;
		break;

	//	mnemonic:		JR Z, r
	//	length:			2
	//	cycles:			12/8
	//	flags:			/
	//	description:	if Z-Flag is set / 1, add r to the current adress, and jump to it; r is a SIGNED number! so positive and negative jumps are possible
	case 0x28:
		#ifdef DEBUG
		printf("JR Z, 0x%02x", memory[pc + 1]);
		#endif // DEBUG
		if (flags.Z == 1)
			pc += 2 + (int8_t)(memory[pc + 1]);	//	Signed number
		else
			pc += 2;
		break;

	//	mnemonic:		JP Z, nn
	//	length:			3
	//	cycles:			16/12
	//	flags:			/
	//	description:	if Z-Flag is set / 1, jump to nn
	case 0xca:
		#ifdef DEBUG
		printf("JP Z, 0x%04x", memory[pc + 2] << 8 | memory[pc + 1]);
		#endif // DEBUG
		if (flags.Z == 1)
			pc = (memory[pc + 2] << 8) | memory[pc + 1];
		else
			pc += 3;
		break;

	//	mnemonic:		JP NZ, nn
	//	length:			3
	//	cycles:			16/12
	//	flags:			/
	//	description:	if Z-Flag is unset / 0, jump to nn
	case 0xc2:
		#ifdef DEBUG
		printf("JP NZ, 0x%04x", memory[pc + 2] << 8 | memory[pc + 1]);
		#endif // DEBUG
		if (flags.Z == 0)
			pc = (memory[pc + 2] << 8) | memory[pc + 1];
		else
			pc += 3;
		break;

	//	mnemonic:		JP NC, nn
	//	length:			3
	//	cycles:			16/12
	//	flags:			/
	//	description:	if C-Flag is reset / 0, jump to nn
	case 0xd2:
		#ifdef DEBUG
		printf("JP NC, 0x%04x", memory[pc + 2] << 8 | memory[pc + 1]);
		#endif // DEBUG
		if (flags.C == 0)
			pc = (memory[pc + 2] << 8) | memory[pc + 1];
		else
			pc += 3;
		break;

	//	mnemonic:		JP C, nn
	//	length:			3
	//	cycles:			16/12
	//	flags:			/
	//	description:	if C-Flag is set / 1, jump to nn
	case 0xda:
		#ifdef DEBUG
		printf("JP C, 0x%04x", memory[pc + 2] << 8 | memory[pc + 1]);
		#endif // DEBUG
		if (flags.C == 1)
			pc = (memory[pc + 2] << 8) | memory[pc + 1];
		else
			pc += 3;
		break;

	//	mnemonic:		JP HL
	//	length:			1
	//	cycles:			4
	//	flags:			/
	//	description:	jump to HL
	case 0xe9:
		#ifdef DEBUG
		printf("JP HL [0x%04x]", registers.H << 8 | registers.L);
		#endif // DEBUG
		pc = (registers.H << 8) | registers.L;
		break;


	//
	//	From here on CB-Prefix OpCodes
	//
	case 0xcb:
		switch (memory[pc + 1])
		{

		//	RLC - L2 - T8 - Z00C
		//	Rotate left TO carry bit
		case 0x00: op_rlc(registers.B, flags, registers, pc); break;
		case 0x01: op_rlc(registers.C, flags, registers, pc); break;
		case 0x02: op_rlc(registers.D, flags, registers, pc); break;
		case 0x03: op_rlc(registers.E, flags, registers, pc); break;
		case 0x04: op_rlc(registers.H, flags, registers, pc); break;
		case 0x05: op_rlc(registers.L, flags, registers, pc); break;
		case 0x06: op_rlc(memory[(registers.H << 8) | registers.L], flags, registers, pc); break;
		case 0x07: op_rlc(registers.A, flags, registers, pc); break;

		//	RRC - L2 - T8 - Z00C
		//	Rotate right TO carry bit
		case 0x08: op_rrc(registers.B, flags, registers, pc); break;
		case 0x09: op_rrc(registers.C, flags, registers, pc); break;
		case 0x0a: op_rrc(registers.D, flags, registers, pc); break;
		case 0x0b: op_rrc(registers.E, flags, registers, pc); break;
		case 0x0c: op_rrc(registers.H, flags, registers, pc); break;
		case 0x0d: op_rrc(registers.L, flags, registers, pc); break;
		case 0x0e: op_rrc(memory[(registers.H << 8) | registers.L], flags, registers, pc); break;
		case 0x0f: op_rrc(registers.A, flags, registers, pc); break;

		//	RL - L2 - T8 - Z00C
		//	Rotate left THROUGH carry flag
		case 0x10: op_rl(registers.B, flags, registers, pc); break;
		case 0x11: op_rl(registers.C, flags, registers, pc); break;
		case 0x12: op_rl(registers.D, flags, registers, pc); break;
		case 0x13: op_rl(registers.E, flags, registers, pc); break;
		case 0x14: op_rl(registers.H, flags, registers, pc); break;
		case 0x15: op_rl(registers.L, flags, registers, pc); break;
		case 0x16: op_rl(memory[(registers.H << 8) | registers.L], flags, registers, pc); break;
		case 0x17: op_rl(registers.A, flags, registers, pc); break;

		//	RR - L2 - T8 - Z00C
		//	Rotate right THROUGH carry flag
		case 0x18: op_rr(registers.B, flags, registers, pc); break;
		case 0x19: op_rr(registers.C, flags, registers, pc); break;
		case 0x1a: op_rr(registers.D, flags, registers, pc); break;
		case 0x1b: op_rr(registers.E, flags, registers, pc); break;
		case 0x1c: op_rr(registers.H, flags, registers, pc); break;
		case 0x1d: op_rr(registers.L, flags, registers, pc); break;
		case 0x1e: op_rr(memory[(registers.H << 8) | registers.L], flags, registers, pc); break;
		case 0x1f: op_rr(registers.A, flags, registers, pc); break;

		//	SLA - L2 - T8 - Z00C
		//	Shift left into CF
		case 0x20: op_sla(registers.B, flags, registers, pc); break;
		case 0x21: op_sla(registers.C, flags, registers, pc); break;
		case 0x22: op_sla(registers.D, flags, registers, pc); break;
		case 0x23: op_sla(registers.E, flags, registers, pc); break;
		case 0x24: op_sla(registers.H, flags, registers, pc); break;
		case 0x25: op_sla(registers.L, flags, registers, pc); break;
		case 0x26: op_sla(memory[(registers.H << 8) | registers.L], flags, registers, pc); break;
		case 0x27: op_sla(registers.A, flags, registers, pc); break;

		//	SRA - L2 - T8 - Z00C
		//	Shift right into CF, keep MSB
		case 0x28: op_sra(registers.B, flags, registers, pc); break;
		case 0x29: op_sra(registers.C, flags, registers, pc); break;
		case 0x2a: op_sra(registers.D, flags, registers, pc); break;
		case 0x2b: op_sra(registers.E, flags, registers, pc); break;
		case 0x2c: op_sra(registers.H, flags, registers, pc); break;
		case 0x2d: op_sra(registers.L, flags, registers, pc); break;
		case 0x2e: op_sra(memory[(registers.H << 8) | registers.L], flags, registers, pc); break;
		case 0x2f: op_sra(registers.A, flags, registers, pc); break;

		//	SWAP - L2 - T8 - Z000
		//	Swap high and low nibble
		case 0x30: op_swap(registers.B, flags, registers, pc); break;
		case 0x31: op_swap(registers.C, flags, registers, pc); break;
		case 0x32: op_swap(registers.D, flags, registers, pc); break;
		case 0x33: op_swap(registers.E, flags, registers, pc); break;
		case 0x34: op_swap(registers.H, flags, registers, pc); break;
		case 0x35: op_swap(registers.L, flags, registers, pc); break;
		case 0x36: op_swap(memory[(registers.H << 8) | registers.L], flags, registers, pc); break;
		case 0x37: op_swap(registers.A, flags, registers, pc); break;

		//	SRL - L2 - T8 - Z00C
		//	Shift right into CF
		case 0x38: op_srl(registers.B, flags, registers, pc); break;
		case 0x39: op_srl(registers.C, flags, registers, pc); break;
		case 0x3a: op_srl(registers.D, flags, registers, pc); break;
		case 0x3b: op_srl(registers.E, flags, registers, pc); break;
		case 0x3c: op_srl(registers.H, flags, registers, pc); break;
		case 0x3d: op_srl(registers.L, flags, registers, pc); break;
		case 0x3e: op_srl(memory[(registers.H << 8) | registers.L], flags, registers, pc); break;
		case 0x3f: op_srl(registers.A, flags, registers, pc); break;

		//	BIT - L2 - T8 - Z01/
		//	Test Bit
		case 0x40: return op_bit(registers.B, 0, flags, registers, pc); break;
		case 0x41: return op_bit(registers.C, 0, flags, registers, pc); break;
		case 0x42: return op_bit(registers.D, 0, flags, registers, pc); break;
		case 0x43: return op_bit(registers.E, 0, flags, registers, pc); break;
		case 0x44: return op_bit(registers.H, 0, flags, registers, pc); break;
		case 0x45: return op_bit(registers.L, 0, flags, registers, pc); break;
		case 0x46: return op_bit(memory[(registers.H << 8) | registers.L], 0, flags, registers, pc, 3); break;
		case 0x47: return op_bit(registers.A, 0, flags, registers, pc); break;
		case 0x48: return op_bit(registers.B, 1, flags, registers, pc); break;
		case 0x49: return op_bit(registers.C, 1, flags, registers, pc); break;
		case 0x4a: return op_bit(registers.D, 1, flags, registers, pc); break;
		case 0x4b: return op_bit(registers.E, 1, flags, registers, pc); break;
		case 0x4c: return op_bit(registers.H, 1, flags, registers, pc); break;
		case 0x4d: return op_bit(registers.L, 1, flags, registers, pc); break;
		case 0x4e: return op_bit(memory[(registers.H << 8) | registers.L], 1, flags, registers, pc, 3); break;
		case 0x4f: return op_bit(registers.A, 1, flags, registers, pc); break;
		case 0x50: return op_bit(registers.B, 2, flags, registers, pc); break;
		case 0x51: return op_bit(registers.C, 2, flags, registers, pc); break;
		case 0x52: return op_bit(registers.D, 2, flags, registers, pc); break;
		case 0x53: return op_bit(registers.E, 2, flags, registers, pc); break;
		case 0x54: return op_bit(registers.H, 2, flags, registers, pc); break;
		case 0x55: return op_bit(registers.L, 2, flags, registers, pc); break;
		case 0x56: return op_bit(memory[(registers.H << 8) | registers.L], 2, flags, registers, pc, 3); break;
		case 0x57: return op_bit(registers.A, 2, flags, registers, pc); break;
		case 0x58: return op_bit(registers.B, 3, flags, registers, pc); break;
		case 0x59: return op_bit(registers.C, 3, flags, registers, pc); break;
		case 0x5a: return op_bit(registers.D, 3, flags, registers, pc); break;
		case 0x5b: return op_bit(registers.E, 3, flags, registers, pc); break;
		case 0x5c: return op_bit(registers.H, 3, flags, registers, pc); break;
		case 0x5d: return op_bit(registers.L, 3, flags, registers, pc); break;
		case 0x5e: return op_bit(memory[(registers.H << 8) | registers.L], 3, flags, registers, pc, 3); break;
		case 0x5f: return op_bit(registers.A, 3, flags, registers, pc); break;
		case 0x60: return op_bit(registers.B, 4, flags, registers, pc); break;
		case 0x61: return op_bit(registers.C, 4, flags, registers, pc); break;
		case 0x62: return op_bit(registers.D, 4, flags, registers, pc); break;
		case 0x63: return op_bit(registers.E, 4, flags, registers, pc); break;
		case 0x64: return op_bit(registers.H, 4, flags, registers, pc); break;
		case 0x65: return op_bit(registers.L, 4, flags, registers, pc); break;
		case 0x66: return op_bit(memory[(registers.H << 8) | registers.L], 4, flags, registers, pc, 3); break;
		case 0x67: return op_bit(registers.A, 4, flags, registers, pc); break;
		case 0x68: return op_bit(registers.B, 5, flags, registers, pc); break;
		case 0x69: return op_bit(registers.C, 5, flags, registers, pc); break;
		case 0x6a: return op_bit(registers.D, 5, flags, registers, pc); break;
		case 0x6b: return op_bit(registers.E, 5, flags, registers, pc); break;
		case 0x6c: return op_bit(registers.H, 5, flags, registers, pc); break;
		case 0x6d: return op_bit(registers.L, 5, flags, registers, pc); break;
		case 0x6e: return op_bit(memory[(registers.H << 8) | registers.L], 5, flags, registers, pc, 3); break;
		case 0x6f: return op_bit(registers.A, 5, flags, registers, pc); break;
		case 0x70: return op_bit(registers.B, 6, flags, registers, pc); break;
		case 0x71: return op_bit(registers.C, 6, flags, registers, pc); break;
		case 0x72: return op_bit(registers.D, 6, flags, registers, pc); break;
		case 0x73: return op_bit(registers.E, 6, flags, registers, pc); break;
		case 0x74: return op_bit(registers.H, 6, flags, registers, pc); break;
		case 0x75: return op_bit(registers.L, 6, flags, registers, pc); break;
		case 0x76: return op_bit(memory[(registers.H << 8) | registers.L], 6, flags, registers, pc, 3); break;
		case 0x77: return op_bit(registers.A, 6, flags, registers, pc); break;
		case 0x78: return op_bit(registers.B, 7, flags, registers, pc); break;
		case 0x79: return op_bit(registers.C, 7, flags, registers, pc); break;
		case 0x7a: return op_bit(registers.D, 7, flags, registers, pc); break;
		case 0x7b: return op_bit(registers.E, 7, flags, registers, pc); break;
		case 0x7c: return op_bit(registers.H, 7, flags, registers, pc); break;
		case 0x7d: return op_bit(registers.L, 7, flags, registers, pc); break;
		case 0x7e: return op_bit(memory[(registers.H << 8) | registers.L], 7, flags, registers, pc, 3); break;
		case 0x7f: return op_bit(registers.A, 7, flags, registers, pc); break;

		//	RES - L2 - T8 - ////
		//	Reset Bit
		case 0x80: return op_res(registers.B, 0, flags, registers, pc); break;
		case 0x81: return op_res(registers.C, 0, flags, registers, pc); break;
		case 0x82: return op_res(registers.D, 0, flags, registers, pc); break;
		case 0x83: return op_res(registers.E, 0, flags, registers, pc); break;
		case 0x84: return op_res(registers.H, 0, flags, registers, pc); break;
		case 0x85: return op_res(registers.L, 0, flags, registers, pc); break;
		case 0x86: return op_res(memory[(registers.H << 8) | registers.L], 0, flags, registers, pc, 4); break;
		case 0x87: return op_res(registers.A, 0, flags, registers, pc); break;
		case 0x88: return op_res(registers.B, 1, flags, registers, pc); break;
		case 0x89: return op_res(registers.C, 1, flags, registers, pc); break;
		case 0x8a: return op_res(registers.D, 1, flags, registers, pc); break;
		case 0x8b: return op_res(registers.E, 1, flags, registers, pc); break;
		case 0x8c: return op_res(registers.H, 1, flags, registers, pc); break;
		case 0x8d: return op_res(registers.L, 1, flags, registers, pc); break;
		case 0x8e: return op_res(memory[(registers.H << 8) | registers.L], 1, flags, registers, pc, 4); break;
		case 0x8f: return op_res(registers.A, 1, flags, registers, pc); break;
		case 0x90: return op_res(registers.B, 2, flags, registers, pc); break;
		case 0x91: return op_res(registers.C, 2, flags, registers, pc); break;
		case 0x92: return op_res(registers.D, 2, flags, registers, pc); break;
		case 0x93: return op_res(registers.E, 2, flags, registers, pc); break;
		case 0x94: return op_res(registers.H, 2, flags, registers, pc); break;
		case 0x95: return op_res(registers.L, 2, flags, registers, pc); break;
		case 0x96: return op_res(memory[(registers.H << 8) | registers.L], 2, flags, registers, pc, 4); break;
		case 0x97: return op_res(registers.A, 2, flags, registers, pc); break;
		case 0x98: return op_res(registers.B, 3, flags, registers, pc); break;
		case 0x99: return op_res(registers.C, 3, flags, registers, pc); break;
		case 0x9a: return op_res(registers.D, 3, flags, registers, pc); break;
		case 0x9b: return op_res(registers.E, 3, flags, registers, pc); break;
		case 0x9c: return op_res(registers.H, 3, flags, registers, pc); break;
		case 0x9d: return op_res(registers.L, 3, flags, registers, pc); break;
		case 0x9e: return op_res(memory[(registers.H << 8) | registers.L], 3, flags, registers, pc, 4); break;
		case 0x9f: return op_res(registers.A, 3, flags, registers, pc); break;
		case 0xa0: return op_res(registers.B, 4, flags, registers, pc); break;
		case 0xa1: return op_res(registers.C, 4, flags, registers, pc); break;
		case 0xa2: return op_res(registers.D, 4, flags, registers, pc); break;
		case 0xa3: return op_res(registers.E, 4, flags, registers, pc); break;
		case 0xa4: return op_res(registers.H, 4, flags, registers, pc); break;
		case 0xa5: return op_res(registers.L, 4, flags, registers, pc); break;
		case 0xa6: return op_res(memory[(registers.H << 8) | registers.L], 4, flags, registers, pc, 4); break;
		case 0xa7: return op_res(registers.A, 4, flags, registers, pc); break;
		case 0xa8: return op_res(registers.B, 5, flags, registers, pc); break;
		case 0xa9: return op_res(registers.C, 5, flags, registers, pc); break;
		case 0xaa: return op_res(registers.D, 5, flags, registers, pc); break;
		case 0xab: return op_res(registers.E, 5, flags, registers, pc); break;
		case 0xac: return op_res(registers.H, 5, flags, registers, pc); break;
		case 0xad: return op_res(registers.L, 5, flags, registers, pc); break;
		case 0xae: return op_res(memory[(registers.H << 8) | registers.L], 5, flags, registers, pc, 4); break;
		case 0xaf: return op_res(registers.A, 5, flags, registers, pc); break;
		case 0xb0: return op_res(registers.B, 6, flags, registers, pc); break;
		case 0xb1: return op_res(registers.C, 6, flags, registers, pc); break;
		case 0xb2: return op_res(registers.D, 6, flags, registers, pc); break;
		case 0xb3: return op_res(registers.E, 6, flags, registers, pc); break;
		case 0xb4: return op_res(registers.H, 6, flags, registers, pc); break;
		case 0xb5: return op_res(registers.L, 6, flags, registers, pc); break;
		case 0xb6: return op_res(memory[(registers.H << 8) | registers.L], 6, flags, registers, pc, 4); break;
		case 0xb7: return op_res(registers.A, 6, flags, registers, pc); break;
		case 0xb8: return op_res(registers.B, 7, flags, registers, pc); break;
		case 0xb9: return op_res(registers.C, 7, flags, registers, pc); break;
		case 0xba: return op_res(registers.D, 7, flags, registers, pc); break;
		case 0xbb: return op_res(registers.E, 7, flags, registers, pc); break;
		case 0xbc: return op_res(registers.H, 7, flags, registers, pc); break;
		case 0xbd: return op_res(registers.L, 7, flags, registers, pc); break;
		case 0xbe: return op_res(memory[(registers.H << 8) | registers.L], 7, flags, registers, pc, 4); break;
		case 0xbf: return op_res(registers.A, 7, flags, registers, pc); break;

		//	SET - L2 - T8 - ////
		//	Set Bit
		case 0xc0: return op_set(registers.B, 0, flags, registers, pc); break;
		case 0xc1: return op_set(registers.C, 0, flags, registers, pc); break;
		case 0xc2: return op_set(registers.D, 0, flags, registers, pc); break;
		case 0xc3: return op_set(registers.E, 0, flags, registers, pc); break;
		case 0xc4: return op_set(registers.H, 0, flags, registers, pc); break;
		case 0xc5: return op_set(registers.L, 0, flags, registers, pc); break;
		case 0xc7: return op_set(registers.A, 0, flags, registers, pc); break;
		case 0xc8: return op_set(registers.B, 1, flags, registers, pc); break;
		case 0xc9: return op_set(registers.C, 1, flags, registers, pc); break;
		case 0xca: return op_set(registers.D, 1, flags, registers, pc); break;
		case 0xcb: return op_set(registers.E, 1, flags, registers, pc); break;
		case 0xcc: return op_set(registers.H, 1, flags, registers, pc); break;
		case 0xcd: return op_set(registers.L, 1, flags, registers, pc); break;
		case 0xcf: return op_set(registers.A, 1, flags, registers, pc); break;
		case 0xd0: return op_set(registers.B, 2, flags, registers, pc); break;
		case 0xd1: return op_set(registers.C, 2, flags, registers, pc); break;
		case 0xd2: return op_set(registers.D, 2, flags, registers, pc); break;
		case 0xd3: return op_set(registers.E, 2, flags, registers, pc); break;
		case 0xd4: return op_set(registers.H, 2, flags, registers, pc); break;
		case 0xd5: return op_set(registers.L, 2, flags, registers, pc); break;
		case 0xd7: return op_set(registers.A, 2, flags, registers, pc); break;
		case 0xd8: return op_set(registers.B, 3, flags, registers, pc); break;
		case 0xd9: return op_set(registers.C, 3, flags, registers, pc); break;
		case 0xda: return op_set(registers.D, 3, flags, registers, pc); break;
		case 0xdb: return op_set(registers.E, 3, flags, registers, pc); break;
		case 0xdc: return op_set(registers.H, 3, flags, registers, pc); break;
		case 0xdd: return op_set(registers.L, 3, flags, registers, pc); break;
		case 0xdf: return op_set(registers.A, 3, flags, registers, pc); break;
		case 0xe0: return op_set(registers.B, 4, flags, registers, pc); break;
		case 0xe1: return op_set(registers.C, 4, flags, registers, pc); break;
		case 0xe2: return op_set(registers.D, 4, flags, registers, pc); break;
		case 0xe3: return op_set(registers.E, 4, flags, registers, pc); break;
		case 0xe4: return op_set(registers.H, 4, flags, registers, pc); break;
		case 0xe5: return op_set(registers.L, 4, flags, registers, pc); break;
		case 0xe7: return op_set(registers.A, 4, flags, registers, pc); break;
		case 0xe8: return op_set(registers.B, 5, flags, registers, pc); break;
		case 0xe9: return op_set(registers.C, 5, flags, registers, pc); break;
		case 0xea: return op_set(registers.D, 5, flags, registers, pc); break;
		case 0xeb: return op_set(registers.E, 5, flags, registers, pc); break;
		case 0xec: return op_set(registers.H, 5, flags, registers, pc); break;
		case 0xed: return op_set(registers.L, 5, flags, registers, pc); break;
		case 0xef: return op_set(registers.A, 5, flags, registers, pc); break;
		case 0xf0: return op_set(registers.B, 6, flags, registers, pc); break;
		case 0xf1: return op_set(registers.C, 6, flags, registers, pc); break;
		case 0xf2: return op_set(registers.D, 6, flags, registers, pc); break;
		case 0xf3: return op_set(registers.E, 6, flags, registers, pc); break;
		case 0xf4: return op_set(registers.H, 6, flags, registers, pc); break;
		case 0xf5: return op_set(registers.L, 6, flags, registers, pc); break;
		case 0xf7: return op_set(registers.A, 6, flags, registers, pc); break;
		case 0xf8: return op_set(registers.B, 7, flags, registers, pc); break;
		case 0xf9: return op_set(registers.C, 7, flags, registers, pc); break;
		case 0xfa: return op_set(registers.D, 7, flags, registers, pc); break;
		case 0xfb: return op_set(registers.E, 7, flags, registers, pc); break;
		case 0xfc: return op_set(registers.H, 7, flags, registers, pc); break;
		case 0xfd: return op_set(registers.L, 7, flags, registers, pc); break;
		case 0xff: return op_set(registers.A, 7, flags, registers, pc); break;

		//	SET nr, (HL) - L2 - T16 - ////
		case 0xc6: return op_set(memory[(registers.H << 8) | registers.L], 0, flags, registers, pc, 4); break;
		case 0xce: return op_set(memory[(registers.H << 8) | registers.L], 1, flags, registers, pc, 4); break;
		case 0xd6: return op_set(memory[(registers.H << 8) | registers.L], 2, flags, registers, pc, 4); break;
		case 0xde: return op_set(memory[(registers.H << 8) | registers.L], 3, flags, registers, pc, 4); break;
		case 0xe6: return op_set(memory[(registers.H << 8) | registers.L], 4, flags, registers, pc, 4); break;
		case 0xee: return op_set(memory[(registers.H << 8) | registers.L], 5, flags, registers, pc, 4); break;
		case 0xf6: return op_set(memory[(registers.H << 8) | registers.L], 6, flags, registers, pc, 4); break;
		case 0xfe: return op_set(memory[(registers.H << 8) | registers.L], 7, flags, registers, pc, 4); break;

		default:
			printf("Unsupported CB-prefixed opcode: 0x%02x at 0x%04x\n\n\n", memory[pc + 1], pc);
			std::exit(EXIT_FAILURE);
			break;
		}
		break;

	default:
		printf("Unsupported opcode: 0x%02x at 0x%04x\n\n\n", opcode, pc);
		std::exit(EXIT_FAILURE);
		break;
	}

	//	formatting for disassembly output
	#ifdef DEBUG
	printf("\n");
	#endif
}

