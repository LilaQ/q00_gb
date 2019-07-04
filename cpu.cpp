#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstdint>
#include "mmu.h"
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
int op_cp(unsigned char& parameter, unsigned char& val, Flags& flags, Registers& registers, uint16_t& pc, int m = 1, int steps = 1) {
#ifdef DEBUG
	printf("CP");
#endif // DEBUG
	flags.Z = (parameter == val) ? 1 : 0;
	flags.N = 1;
	flags.H = (parameter & 0xF) < (val & 0xF);
	flags.C = (parameter < val) ? 1 : 0;
	pc += steps;

	//	return m-cycle
	return m;
}

// LD X, Y
// Load Y into X
int op_ld(unsigned char& parameter, unsigned char& val, Flags& flags, Registers& registers, uint16_t& pc, int m = 1, int steps = 1) {
#ifdef DEBUG
	printf("LD");
#endif // DEBUG
	parameter = val;
	pc += steps;

	//	return m-cycle
	return m;
}

//	LD (XY), Z
//	Load Z to adress (XY)
int op_ld_to_adr(uint16_t adr, unsigned char& val, uint16_t& pc, int m = 2, int steps = 1) {
	writeToMem(adr, val);
	pc += steps;
	//	return m-cycles;
	return m;
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
int op_rlc(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc, int m = 2) {
#ifdef DEBUG
	printf("RLC");
#endif // DEBUG
	flags.C = parameter >> 7 & 0x01;
	flags.N = 0;
	flags.H = 0;
	parameter = parameter << 1 | flags.C;
	flags.Z = (parameter == 0);
	pc += 2;

	//	return m-cycles
	return m;
}

//	RRC - L2 - T8 - Z00C
//	Rotate right TO carry bit (bit 0 to CF, and bit 0 to bit 7)
int op_rrc(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc, int m = 2) {
#ifdef DEBUG
	printf("RRC");
#endif // DEBUG
	flags.C = parameter & 0x01;
	flags.N = 0;
	flags.H = 0;
	parameter = parameter >> 1 | (flags.C << 7);
	flags.Z = (parameter == 0);
	pc += 2;

	//	return m-cycles
	return m;
}

//	RL - L2 - T8 - Z00C
//	Rotate left THROUGH carry flag (CF to bit 0, bit 7 to CF)
int op_rl(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc, int m = 2) {
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

	//	return m-cycles
	return m;
}

//	RR - L2 - T8 - Z00C
//	Rotate right THROUGH carry flag (CF to bit 7, bit 0 to CF)
int op_rr(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc, int m = 2) {
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

	//	return m-cycles
	return m;
}

//	RST 
//	RST
int op_rst(uint8_t add, uint16_t& pc, uint16_t &sp) {
#ifdef DEBUG
	printf("RST");
#endif // DEBUG
	sp--;
	writeToMem(sp, (pc + 1) >> 8);
	sp--;
	writeToMem(sp, (pc + 1) & 0xff);
	pc = add;

	//	return m-cycles
	return 4;
}

//	SBC - L1 - T4 - Z1HC
//	Subtract (x + CF) from register A
int op_sbc(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc, int m = 1) {
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

	//	return m-cycles
	return m;
}

//	SBC - L2- T8 - Z1HC
//	Subtract (u8 + CF) from register A
int op_sbc_u8(unsigned char arg, Flags& flags, Registers& registers, uint16_t& pc) {
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

	//	return m-cycles
	return 2;
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
int op_sla(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc, int m = 2) {
#ifdef DEBUG
	printf("SLA");
#endif // DEBUG
	flags.C = (parameter >> 7) & 0x01;
	flags.N = 0;
	flags.H = 0;
	parameter = parameter << 1;
	flags.Z = (parameter == 0);
	pc += 2;

	//	return m-cycles
	return m;
}

//	SRA - L2 - T8 - Z00C
//	Shift left into carry (bit 0 to CF, bit 7 to bit 7 [MSB remains same!])
int op_sra(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc, int m = 2) {
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

	//	return m-cycles
	return m;
}

//	SRL - L2 - T8 - Z00C
//	Shift left into carry (bit 0 to CF, 0 to bit 7)
int op_srl(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc, int m = 2) {
#ifdef DEBUG
	printf("SRL");
#endif // DEBUG
	flags.C = parameter & 0x01;
	flags.N = 0;
	flags.H = 0;
	parameter = parameter >> 1;
	flags.Z = (parameter == 0);
	pc += 2;

	//	return m-cycles
	return m;
}

//	SWAP - L2 - T8 - Z000
//	Swap high and low nibble
int op_swap(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc, int m = 2) {
#ifdef DEBUG
	printf("SWAP");
#endif // DEBUG
	flags.C = 0;
	flags.N = 0;
	flags.H = 0;
	parameter = ((parameter >> 4) & 0x0f) | ((parameter << 4) & 0xf0);
	flags.Z = (parameter == 0);
	pc += 2;

	//	return m-cycles
	return m;
}

//	XOR
//	XOR
int op_xor(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc, int m = 1, int step = 1) {
#ifdef DEBUG
	printf("XOR A");
#endif // DEBUG
	registers.A = registers.A ^ parameter;
	flags.Z = (registers.A == 0) ? 1 : 0;
	flags.N = 0;
	flags.H = 0;
	flags.C = 0;
	pc += step;

	//	return m-cycles
	return m;
}

//	POP XY
//	Pops 2 bytes from stack into paired registers
int op_pop(unsigned char& hi, unsigned char& lo, uint16_t& pc, uint16_t& sp) {
#ifdef DEBUG
	printf("POP");
#endif // DEBUG
	lo = readFromMem(sp);
	sp++;
	hi = readFromMem(sp);
	sp++;
	pc += 1;

	//	return m-cycles
	return 3;
}

//	PUSH XY
//	Pushes 2 bytes to the stack
int op_push(unsigned char& hi, unsigned char& lo, uint16_t& pc, uint16_t& sp) {
#ifdef DEBUG
	printf("PUSH BC");
#endif // DEBUG
	sp--;
	writeToMem(sp, hi);
	sp--;
	writeToMem(sp, lo);
	pc += 1;

	//	return m-cycles 
	return 4;
}

//	INC X
//	Increases register X
int op_inc(unsigned char& arg, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("INC");
#endif // DEBUG
	flags.N = 0;
	flags.H = ((arg & 0xf) == 0xf);
	arg += 1;
	flags.Z = (arg == 0);
	pc += 1;

	//	return m-cycles
	return 1;
}

//	INC XY
//	Increase register pair XY
int op_inc_u16(unsigned char& hi, unsigned char& lo, uint16_t& pc) {
#ifdef DEBUG
	printf("INC BC");
#endif // DEBUG
	int val = (hi << 8) | lo;
	val++;
	hi = val >> 8;
	lo = val & 0xFF;
	pc += 1;

	//	return m-cycles;
	return 2;
}

//	DEC X
//	Decresese register X
int op_dec(unsigned char& arg, Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("DEC");
#endif // DEBUG
	flags.N = 1;
	flags.H = ((arg & 0xf) == 0x0) ? 1 : 0;
	arg -= 1;
	flags.Z = (arg == 0) ? 1 : 0;
	pc += 1;

	//	return m-cycles
	return 1;
}

//	DEC XY
//	Decrease register pair XY
int op_dec_u16(unsigned char& hi, unsigned char& lo, uint16_t& pc) {
	#ifdef DEBUG
	printf("DEC XY");
	#endif // DEBUG
	int val = (hi << 8) | lo;
	val--;
	hi = val >> 8 & 0xff;
	lo = val & 0xff;
	pc += 1;

	//	return m-cycles
	return 2;
}

//	AND X
//	AND with register X with register A
int op_and(unsigned char& arg, Flags& flags, Registers& registers, uint16_t& pc, int m = 1, int step = 1) {
#ifdef DEBUG
	printf("AND B");
#endif // DEBUG
	registers.A = registers.A & arg;
	flags.Z = (registers.A == 0);
	flags.N = 0;
	flags.H = 1;
	flags.C = 0;
	pc += step;

	//	return m-cycles
	return m;
}

//	ADD X
//	Adds X to A
int op_add(unsigned char& arg, Flags& flags, Registers& registers, uint16_t& pc, int m = 1, int step = 1) {
#ifdef DEBUG
	printf("ADD A, B");
#endif // DEBUG
	flags.N = 0;
	flags.H = (((registers.A & 0xf) + (arg & 0xf)) > 0xf);
	flags.C = ((registers.A + arg) > 0xff);
	registers.A += arg;
	flags.Z = (registers.A == 0);
	pc += step;

	//	return m-cycles
	return m;
}

//	SUB X
//	Subtracts X from A
int op_sub(unsigned char& arg, Flags& flags, Registers& registers, uint16_t& pc, int m = 1, int step = 1) {
#ifdef DEBUG
	printf("SUB B");
#endif // DEBUG
	flags.C = (arg > registers.A);
	flags.H = (arg & 0xf) > (registers.A & 0xf);
	registers.A = registers.A - arg;
	flags.Z = (registers.A == 0);
	flags.N = 1;
	pc += step;

	//	return m-cycles
	return m;
}

//	OR X
//	ORs X with A
int op_or(unsigned char& arg, Flags& flags, Registers& registers, uint16_t& pc, int m = 1, int step = 1) {
#ifdef DEBUG
	printf("OR");
#endif // DEBUG
	registers.A = registers.A | arg;
	flags.Z = (registers.A == 0);
	flags.N = 0;
	flags.H = 0;
	flags.C = 0;
	pc += step;

	//	return m-cycles
	return m;
}

//	INC / DEC SP
//	Increase or decrease SP
int op_add_to_sp(uint16_t& sp, uint16_t& pc, int arg) {
	sp += arg;
	pc += 1;
	//	return m-cycles;
	return 2;
}

//	ENABLE / DISABLE IME
//	Enables or disables Interrupt Master Entry (main interrupt switch)
int op_set_ime(int& ime, int state, uint16_t& pc) {
	ime = state;
	pc += 1;
	//	return m-cycles
	return 1;
}

//	RLA / RLCA
//	rotates left to or through carry 
int op_rla(Flags& flags, Registers& registers, uint16_t& pc, int keepoldflag = 0) {
	int oldcarry = flags.C;
	flags.C = registers.A >> 7 & 0x01;
	flags.Z = 0;
	flags.N = 0;
	flags.H = 0;
	if(keepoldflag)
		registers.A = registers.A << 1 | oldcarry;
	else
		registers.A = registers.A << 1 | flags.C;
	pc += 1;

	//	return m-cycles
	return 1;
}

//	RRA / RRCA
//	rotates right to or through carry
int op_rra(Flags& flags, Registers& registers, uint16_t& pc, int keepoldflag = 0) {
	int oldcarry = flags.C;
	flags.C = registers.A & 0x01;
	flags.Z = 0;
	flags.N = 0;
	flags.H = 0;
	if(keepoldflag)
		registers.A = registers.A >> 1 | (oldcarry << 7);
	else
		registers.A = registers.A >> 1 | (flags.C << 7);
	pc += 1;

	//	return m-cycles
	return 1;
}

//	RET / RETI / RET Z / RET NZ / RET C / RET NC
//	return, with options (enable interrupts, on zero / not zero, on carry / not carry)
int op_ret(int& ime, int state, int condition_state, uint16_t& pc, uint16_t& sp, unsigned char condition, int enable_condition = 0) {
#ifdef DEBUG
	printf("RET/I/NZ/Z/NC/C", (memory[sp + 1] << 8) | memory[sp]);
#endif // DEBUG
	if (!enable_condition || (enable_condition && condition == condition_state)) {
		pc = (readFromMem(sp + 1) << 8) | readFromMem(sp);
		sp += 2;
	}
	else {
		pc += 1;
	}
	//	set IME
	if(state)
		ime = 1;
	//	return m-cycles
	if (!enable_condition) return 4;
	else if (enable_condition && (condition == condition_state)) return 5;
	else return 2;
}

//	JP / JP C / JP NC / JP Z / JP NZ
int op_jp(uint16_t& pc, uint16_t adr, int enable_condition, unsigned char condition, int condition_state) {
	if (!enable_condition || (enable_condition && (condition == condition_state))) {
		pc = adr;
		return 4;
	}
	else {
		pc += 3;
		return 3;
	}
}

//	JR / JR C / JR NC / JR Z / JR NZ
int op_jr(uint16_t& pc, int8_t adr, int enable_condition, unsigned char condition, int condition_state) {
	if (!enable_condition || (enable_condition && (condition == condition_state))) {
		if (pc == 0x2000)
			int i = 0;
		pc += 2 + adr;
		return 3;
	}
	else {
		pc += 2;
		return 2;
	}
}

//	ADC
int op_adc(unsigned char& arg, Flags& flags, Registers& registers, uint16_t& pc, int m = 1, int step = 1) {
#ifdef DEBUG
	printf("ADC");
#endif // DEBUG
	flags.N = 0;
	flags.H = (((arg & 0xf) + (registers.A & 0xf) + flags.C) > 0xf);
	int oldcarry = ((arg + registers.A + flags.C) > 0xff);
	registers.A += arg + flags.C;
	flags.C = oldcarry;
	flags.Z = (registers.A == 0);
	pc += step;
	//	return m-cycles
	return m;
}

//	CPL
//	complements register A
int op_cpl(Flags& flags, Registers& registers, uint16_t& pc) {
#ifdef DEBUG
	printf("CPL");
#endif // DEBUG
	registers.A = ~registers.A;
	flags.N = 1;
	flags.H = 1;
	pc += 1;
	//	return m-cycles
	return 1;
}

//	SCF
//	Sets carry flags
int op_scf(Flags& flags, uint16_t& pc) {
#ifdef DEBUG
	printf("SCF");
#endif // DEBUG
	flags.N = 0;
	flags.H = 0;
	flags.C = 1;
	pc += 1;
	return 1;
}

//	CCF
//	Complement carry flag
int op_ccf(Flags& flags, uint16_t& pc) {
#ifdef DEBUG
	printf("CCF");
#endif // DEBUG
	flags.N = 0;
	flags.H = 0;
	flags.C = ~flags.C & 0x1;
	pc += 1;
	return 1;
}

//	DAA
//	converts A into packed BCD(round each hex - position, back to 0 - 9 range)
int op_daa(Flags& flags, Registers& registers, uint16_t& pc) {
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
		return 1;
	}
}

//	CALL / CALL Z / CALL NZ / CALL C / CALL NC
//	pushes next instruction adress to the stack, and jumps to the new adress
int op_call(uint16_t& pc, uint16_t& sp, int enable_condition, unsigned char condition, int condition_state) {
	if (!enable_condition || (condition == condition_state)) {
		sp--;
		writeToMem(sp, (pc + 3) >> 8);
		sp--;
		writeToMem(sp, (pc + 3) & 0xff);
		pc = ((readFromMem(pc + 2) << 8) | readFromMem(pc + 1));
	}
	else
	{
		pc += 3;
	}
	//	return m-cycles
	if (!enable_condition) return 6;
	else if (enable_condition && (condition == condition_state)) return 6;
	else return 3;
}

//	LD RR, u16
//	loads u16 into a paired register
int op_ld_rr_u16(uint16_t& pc, unsigned char& hi, unsigned char& lo, unsigned char shi, unsigned slo) {
	hi = shi;
	lo = slo;
	pc += 3;
	//	return m-cycles
	return 3;
}

//	LD SP u16
//	loads the next two bytes into the stackpointer (SP)
int op_ld_sp_u16(uint16_t& pc, uint16_t& sp, unsigned char shi, unsigned slo, int m = 3, int step = 3) {
	sp = (shi << 8) | slo;
	pc += step;
	//	return m-cycles
	return m;
}

//	LD (HL+/-) A / LD A (HL+/-)
//	Load to / from HL and increase or decrese HL after
int op_ld_hl_a(uint16_t& pc, Registers& registers, int action) {
#ifdef DEBUG
	printf("LD A, (HL+) - or similar");
#endif // DEBUG
	if((action & HL_ACTION::READ_FROM_HL) == HL_ACTION::READ_FROM_HL)
		registers.A = readFromMem((registers.H << 8) | registers.L);
	if((action & HL_ACTION::WRITE_TO_HL) == HL_ACTION::WRITE_TO_HL)
		writeToMem((registers.H << 8) | registers.L, registers.A);
	uint16_t val = (registers.H << 8) | registers.L;
	if ((action & HL_ACTION::INCREMENT) == HL_ACTION::INCREMENT) { val++; }
	if ((action & HL_ACTION::DECREMENT) == HL_ACTION::DECREMENT) { val--; }
	registers.H = val >> 8 & 0xff;
	registers.L = val & 0xff;
	pc += 1;
	//	return m-cycles
	return 2;
}

//	ADD HL RR
//	ADD u16 (register pair or SP) to HL
int op_add_hl_u16(uint16_t& pc, Flags& flags, uint16_t val, Registers& registers) {
#ifdef DEBUG
	printf("ADD HL, u16");
#endif // DEBUG
	uint16_t tar = (registers.H << 8) | registers.L;
	flags.N = 0;
	flags.H = ((((val & 0xfff) + (tar & 0xfff)) & 0x1000) == 0x1000) ? 1 : 0;	//	mask both values up to bit 11, add, mask the (half-carry) Uebertrag, check if set
	flags.C = ((val + tar) > 0xffff) ? 1 : 0;
	registers.H = (val + tar) >> 8;
	registers.L = (val + tar) & 0xff;
	pc += 1;
	//	return m-cycles
	return 2;
}

//	LD HL, SP + n
//	loads SP + n into HL
int op_ld_hl_sp(uint16_t& pc, uint16_t& sp, Flags& flags, Registers& registers) {
#ifdef DEBUG
	printf("LD HL, SP + n");
#endif // DEBUG
	flags.H = ((sp & 0xf) + (readFromMem(pc + 1) & 0xf)) > 0xf;
	flags.C = ((sp & 0xff) + readFromMem(pc + 1)) > 0xff;
	registers.H = (sp + ((int8_t)readFromMem(pc + 1))) >> 8;
	registers.L = (sp + ((int8_t)readFromMem(pc + 1))) & 0xff;
	flags.Z = 0;
	flags.N = 0;
	pc += 2;
	//	return m-cycles
	return 3;
}

//	ADD SP, n
//	adds n to SP
int op_add_sp(uint16_t& pc, uint16_t& sp, Flags& flags, Registers& registers) {
#ifdef DEBUG
	printf("ADD SP, n");
#endif // DEBUG
	flags.Z = 0;
	flags.N = 0;
	flags.H = ((sp & 0xf) + (readFromMem(pc + 1) & 0xf)) > 0xf;
	flags.C = ((sp & 0xff) + readFromMem(pc + 1)) > 0xff;
	sp += (int8_t)readFromMem(pc + 1);
	pc += 2;
	//	return m-cycles
	return 4;
}


int processOpcode(uint16_t& pc, uint16_t& sp, Registers& registers, Flags& flags, int& interrupts_enabled) {
	opcode = readFromMem(pc);

	switch (opcode) {

	//	STOP (TO DO)
	case 0x10: pc++; return 1; break;

	//	NOP
	case 0x00: pc++; return 1; break;

	//	HALT
	case 0x76: flags.HALT = 1; pc += 1; return 1; break;

	//	EI / DI
	case 0xf3: return op_set_ime(interrupts_enabled, 0, pc); break;
	case 0xfb: return op_set_ime(interrupts_enabled, 1, pc); break;

	//	RST
	case 0xc7: return op_rst(0x00, pc, sp); break;
	case 0xd7: return op_rst(0x10, pc, sp); break;
	case 0xe7: return op_rst(0x20, pc, sp); break;
	case 0xf7: return op_rst(0x30, pc, sp); break;
	case 0xcf: return op_rst(0x08, pc, sp); break;
	case 0xdf: return op_rst(0x18, pc, sp); break;
	case 0xef: return op_rst(0x28, pc, sp); break;
	case 0xff: return op_rst(0x38, pc, sp); break;

	//////////////////////////////////////////
	////////	ROT ops
	//////////////////////////////////////////

	//	RLA / RLCA
	case 0x17: return op_rla(flags, registers, pc, 1); break;
	case 0x07: return op_rla(flags, registers, pc, 0); break;

	//	RRA / RRCA
	case 0x1f: return op_rra(flags, registers, pc, 1); break;
	case 0x0f: return op_rra(flags, registers, pc, 0); break;

	//////////////////////////////////////////
	////////	PUSH / POP / RET ops
	//////////////////////////////////////////

	//	mnemonic:		POP XY
	case 0xc1: return op_pop(registers.B, registers.C, pc, sp); break;
	case 0xd1: return op_pop(registers.D, registers.E, pc, sp); break;
	case 0xe1: return op_pop(registers.H, registers.L, pc, sp); break;
	case 0xf1:
		#ifdef DEBUG
		printf("POP AF");
		#endif // DEBUG
		flags.Z = readFromMem(sp) >> 7 & 0x1;
		flags.N = readFromMem(sp) >> 6 & 0x1;
		flags.H = readFromMem(sp) >> 5 & 0x1;
		flags.C = readFromMem(sp) >> 4 & 0x1;
		sp++;
		registers.A = readFromMem(sp);
		sp++;
		pc += 1;

		//	return m-cycles
		return 3;
		break;

	//	mnemonic:		PUSH BC
	case 0xc5: return op_push(registers.B, registers.C, pc, sp); break;
	case 0xd5: return op_push(registers.D, registers.E, pc, sp); break;
	case 0xe5: return op_push(registers.H, registers.L, pc, sp); break;
	case 0xf5:
		#ifdef DEBUG
		printf("PUSH AF");
		#endif // DEBUG
		sp--;
		writeToMem(sp, registers.A);
		sp--;
		writeToMem(sp, (flags.Z << 3 | flags.N << 2 | flags.H << 1 | flags.C) << 4);	//	push to high nibble
		pc += 1;

		//	return m-cycles
		return 4;
		break;

	//	RET / RETI / RET C / RET NC / RET NZ / RET Z
	case 0xc9: return op_ret(interrupts_enabled, 0, 0, pc, sp, flags.C, 0); break;
	case 0xd9: return op_ret(interrupts_enabled, 1, 0, pc, sp, flags.C, 0); break;
	case 0xd8: return op_ret(interrupts_enabled, 0, 1, pc, sp, flags.C, 1); break;
	case 0xd0: return op_ret(interrupts_enabled, 0, 0, pc, sp, flags.C, 1); break;
	case 0xc0: return op_ret(interrupts_enabled, 0, 0, pc, sp, flags.Z, 1); break;
	case 0xc8: return op_ret(interrupts_enabled, 0, 1, pc, sp, flags.Z, 1); break;

	//////////////////////////////////////////
	////////	CALL ops
	//////////////////////////////////////////

	//	mnemonic:		CALL nn
	case 0xcd: return op_call(pc, sp, 0, 0, 0); break;
	case 0xc4: return op_call(pc, sp, 1, flags.Z, 0); break;
	case 0xcc: return op_call(pc, sp, 1, flags.Z, 1); break;
	case 0xd4: return op_call(pc, sp, 1, flags.C, 0); break;
	case 0xdc: return op_call(pc, sp, 1, flags.C, 1); break;

	//////////////////////////////////////////
	////////	INC / DEC ops
	//////////////////////////////////////////

	//	mnemonic:		DEC X
	case 0x3d: return op_dec(registers.A, flags, registers, pc);  break;
	case 0x05: return op_dec(registers.B, flags, registers, pc);  break;
	case 0x0d: return op_dec(registers.C, flags, registers, pc);  break;
	case 0x15: return op_dec(registers.D, flags, registers, pc);  break;
	case 0x1d: return op_dec(registers.E, flags, registers, pc);  break;
	case 0x25: return op_dec(registers.H, flags, registers, pc);  break;
	case 0x2d: return op_dec(registers.L, flags, registers, pc);  break;

	//	mnemonic:		DEC XY
	case 0x0b: return op_dec_u16(registers.B, registers.C, pc); break;
	case 0x1b: return op_dec_u16(registers.D, registers.E, pc); break;
	case 0x2b: return op_dec_u16(registers.H, registers.L, pc); break;

	//	mnemonic:		DEC (HL)
	case 0x35:
	{
		#ifdef DEBUG
		printf("DEC (HL)");
		#endif // DEBUG
		flags.H = ((readFromMem((registers.H << 8) | registers.L) & 0xf) == 0x0);
		writeToMem((registers.H << 8) | registers.L, readFromMem((registers.H << 8) | registers.L) - 1);
		flags.Z = (readFromMem((registers.H << 8) | registers.L) == 0);
		flags.N = 1;
		pc += 1;
		//	return m-cycles
		return 3;
		break;
	}

	//	mnemonic:		INC (HL)
	case 0x34:
	{
#ifdef DEBUG
		printf("INC (HL)");
#endif // DEBUG
		flags.H = (readFromMem((registers.H << 8) | registers.L) & 0xf) == 0xf;
		writeToMem((registers.H << 8) | registers.L, readFromMem((registers.H << 8) | registers.L) + 1);
		flags.Z = readFromMem((registers.H << 8) | registers.L) == 0;
		flags.N = 0;
		pc += 1;
		//	return m-cycles
		return 3;
		break;
	}

	//	mnemonic:		INC / DEC SP
	case 0x33: return op_add_to_sp(sp, pc, 1); break;
	case 0x3b: return op_add_to_sp(sp, pc, -1); break;

	//	mnemonic:		INC X
	case 0x3c: return op_inc(registers.A, flags, registers, pc); break;
	case 0x04: return op_inc(registers.B, flags, registers, pc); break;
	case 0x0c: return op_inc(registers.C, flags, registers, pc); break;
	case 0x14: return op_inc(registers.D, flags, registers, pc); break;
	case 0x1c: return op_inc(registers.E, flags, registers, pc); break;
	case 0x24: return op_inc(registers.H, flags, registers, pc); break;
	case 0x2c: return op_inc(registers.L, flags, registers, pc); break;

	//	mnemonic:		INC BC
	case 0x03: return op_inc_u16(registers.B, registers.C, pc); break;
	case 0x13: return op_inc_u16(registers.D, registers.E, pc); break;
	case 0x23: return op_inc_u16(registers.H, registers.L, pc); break;

	//	mnemonic:		CPL
	case 0x2f: return op_cpl(flags, registers, pc); break;

	//	mnemonic:		DAA
	case 0x27: return op_daa(flags, registers, pc); break;
	

	//////////////////////////////////////////
	////////	LD ops
	//////////////////////////////////////////

	//	LD (nn), A
	case 0xe0: return op_ld_to_adr(0xFF00 + readFromMem(pc + 1), registers.A, pc, 3, 2); break;
	case 0xea: return op_ld_to_adr((readFromMem(pc + 2) << 8) | readFromMem(pc + 1), registers.A, pc, 4, 3); break;

	//	LD A, (XY)
	case 0x0a: return op_ld(registers.A, readFromMem((registers.B << 8) | registers.C), flags, registers, pc, 2); break;
	case 0x1a: return op_ld(registers.A, readFromMem((registers.D << 8) | registers.E), flags, registers, pc, 2); break;
	case 0xf0: return op_ld(registers.A, readFromMem(0xff00 + readFromMem(pc + 1)), flags, registers, pc, 3, 2); break;
	case 0xf2: return op_ld(registers.A, readFromMem(0xff00 + registers.C), flags, registers, pc, 2); break;
	case 0xfa: return op_ld(registers.A, readFromMem(readFromMem(pc + 2) << 8 | readFromMem(pc + 1)), flags, registers, pc, 4, 3); break;

	//	LD A
	case 0x7f: return op_ld(registers.A, registers.A, flags, registers, pc); break;
	case 0x78: return op_ld(registers.A, registers.B, flags, registers, pc); break;
	case 0x79: return op_ld(registers.A, registers.C, flags, registers, pc); break;
	case 0x7a: return op_ld(registers.A, registers.D, flags, registers, pc); break;
	case 0x7b: return op_ld(registers.A, registers.E, flags, registers, pc); break;
	case 0x7c: return op_ld(registers.A, registers.H, flags, registers, pc); break;
	case 0x7d: return op_ld(registers.A, registers.L, flags, registers, pc); break;
	case 0x7e: return op_ld(registers.A, readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0x3e: return op_ld(registers.A, readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	LD B
	case 0x47: return op_ld(registers.B, registers.A, flags, registers, pc); break;
	case 0x40: return op_ld(registers.B, registers.B, flags, registers, pc); break;
	case 0x41: return op_ld(registers.B, registers.C, flags, registers, pc); break;
	case 0x42: return op_ld(registers.B, registers.D, flags, registers, pc); break;
	case 0x43: return op_ld(registers.B, registers.E, flags, registers, pc); break;
	case 0x44: return op_ld(registers.B, registers.H, flags, registers, pc); break;
	case 0x45: return op_ld(registers.B, registers.L, flags, registers, pc); break;
	case 0x46: return op_ld(registers.B, readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0x06: return op_ld(registers.B, readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	LD C
	case 0x4f: return op_ld(registers.C, registers.A, flags, registers, pc); break;
	case 0x48: return op_ld(registers.C, registers.B, flags, registers, pc); break;
	case 0x49: return op_ld(registers.C, registers.C, flags, registers, pc); break;
	case 0x4a: return op_ld(registers.C, registers.D, flags, registers, pc); break;
	case 0x4b: return op_ld(registers.C, registers.E, flags, registers, pc); break;
	case 0x4c: return op_ld(registers.C, registers.H, flags, registers, pc); break;
	case 0x4d: return op_ld(registers.C, registers.L, flags, registers, pc); break;
	case 0x4e: return op_ld(registers.C, readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0x0e: return op_ld(registers.C, readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	LD D
	case 0x57: return op_ld(registers.D, registers.A, flags, registers, pc); break;
	case 0x50: return op_ld(registers.D, registers.B, flags, registers, pc); break;
	case 0x51: return op_ld(registers.D, registers.C, flags, registers, pc); break;
	case 0x52: return op_ld(registers.D, registers.D, flags, registers, pc); break;
	case 0x53: return op_ld(registers.D, registers.E, flags, registers, pc); break;
	case 0x54: return op_ld(registers.D, registers.H, flags, registers, pc); break;
	case 0x55: return op_ld(registers.D, registers.L, flags, registers, pc); break;
	case 0x56: return op_ld(registers.D, readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0x16: return op_ld(registers.D, readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	LD E
	case 0x5f: return op_ld(registers.E, registers.A, flags, registers, pc); break;
	case 0x58: return op_ld(registers.E, registers.B, flags, registers, pc); break;
	case 0x59: return op_ld(registers.E, registers.C, flags, registers, pc); break;
	case 0x5a: return op_ld(registers.E, registers.D, flags, registers, pc); break;
	case 0x5b: return op_ld(registers.E, registers.E, flags, registers, pc); break;
	case 0x5c: return op_ld(registers.E, registers.H, flags, registers, pc); break;
	case 0x5d: return op_ld(registers.E, registers.L, flags, registers, pc); break;
	case 0x5e: return op_ld(registers.E, readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0x1e: return op_ld(registers.E, readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	LD H
	case 0x67: return op_ld(registers.H, registers.A, flags, registers, pc); break;
	case 0x60: return op_ld(registers.H, registers.B, flags, registers, pc); break;
	case 0x61: return op_ld(registers.H, registers.C, flags, registers, pc); break;
	case 0x62: return op_ld(registers.H, registers.D, flags, registers, pc); break;
	case 0x63: return op_ld(registers.H, registers.E, flags, registers, pc); break;
	case 0x64: return op_ld(registers.H, registers.H, flags, registers, pc); break;
	case 0x65: return op_ld(registers.H, registers.L, flags, registers, pc); break;
	case 0x66: return op_ld(registers.H, readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0x26: return op_ld(registers.H, readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	LD L
	case 0x6f: return op_ld(registers.L, registers.A, flags, registers, pc); break;
	case 0x68: return op_ld(registers.L, registers.B, flags, registers, pc); break;
	case 0x69: return op_ld(registers.L, registers.C, flags, registers, pc); break;
	case 0x6a: return op_ld(registers.L, registers.D, flags, registers, pc); break;
	case 0x6b: return op_ld(registers.L, registers.E, flags, registers, pc); break;
	case 0x6c: return op_ld(registers.L, registers.H, flags, registers, pc); break;
	case 0x6d: return op_ld(registers.L, registers.L, flags, registers, pc); break;
	case 0x6e: return op_ld(registers.L, readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0x2e: return op_ld(registers.L, readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	LD (XY), Z
	case 0x70: return op_ld_to_adr((registers.H << 8) | registers.L, registers.B, pc); break;
	case 0x71: return op_ld_to_adr((registers.H << 8) | registers.L, registers.C, pc); break;
	case 0x72: return op_ld_to_adr((registers.H << 8) | registers.L, registers.D, pc); break;
	case 0x73: return op_ld_to_adr((registers.H << 8) | registers.L, registers.E, pc); break;
	case 0x74: return op_ld_to_adr((registers.H << 8) | registers.L, registers.H, pc); break;
	case 0x75: return op_ld_to_adr((registers.H << 8) | registers.L, registers.L, pc); break;
	case 0x77: return op_ld_to_adr((registers.H << 8) | registers.L, registers.A, pc); break;
	case 0xe2: return op_ld_to_adr(0xff00 + registers.C, registers.A, pc); break;
	case 0x36: return op_ld_to_adr((registers.H << 8) | registers.L, readFromMem(pc + 1), pc, 3, 2); break;

	//	LD RR, nn
	case 0x01: return op_ld_rr_u16(pc, registers.B, registers.C, readFromMem(pc + 2), readFromMem(pc + 1)); break;
	case 0x11: return op_ld_rr_u16(pc, registers.D, registers.E, readFromMem(pc + 2), readFromMem(pc + 1)); break;
	case 0x21: return op_ld_rr_u16(pc, registers.H, registers.L, readFromMem(pc + 2), readFromMem(pc + 1)); break;
	case 0x31: return op_ld_sp_u16(pc, sp, readFromMem(pc + 2), readFromMem(pc + 1)); break;
	case 0xf9: return op_ld_sp_u16(pc, sp, registers.H, registers.L, 2, 1);

	//	LD (XX), A
	case 0x02: return op_ld_to_adr((registers.B << 8) | registers.C, registers.A, pc, 2, 1); break;
	case 0x12: return op_ld_to_adr((registers.D << 8) | registers.E, registers.A, pc, 2, 1); break;

	//	LD HL, SP + n 
	case 0xf8: return op_ld_hl_sp(pc, sp, flags, registers); break;

	//	LD A, (HL+ / -) / LD (HL + / -) A
	case 0x2a: return op_ld_hl_a(pc, registers, HL_ACTION::READ_FROM_HL | HL_ACTION::INCREMENT); break;
	case 0x3a: return op_ld_hl_a(pc, registers, HL_ACTION::READ_FROM_HL | HL_ACTION::DECREMENT); break;
	case 0x32: return op_ld_hl_a(pc, registers, HL_ACTION::WRITE_TO_HL | HL_ACTION::DECREMENT); break;
	case 0x22: return op_ld_hl_a(pc, registers, HL_ACTION::WRITE_TO_HL | HL_ACTION::INCREMENT); break;

	//	mnemonic:		LD (nn), SP
	case 0x08:
	{
		#ifdef DEBUG
		printf("LD (nn)[0x%04x], SP", ((memory[pc + 2] << 8) | memory[pc + 1]));
		#endif // DEBUG
		writeToMem(((readFromMem(pc + 2) << 8) | readFromMem(pc + 1)), sp & 0xff);
		writeToMem(((readFromMem(pc + 2) << 8) | readFromMem(pc + 1)) + 1, sp >> 8);
		pc += 3;
		return 5;
		break;
	}



	//////////////////////////////////////////
	////////	Set Flag instructions
	//////////////////////////////////////////

	//	mnemonic:		SCF
	case 0x37: return op_scf(flags, pc); break;

	//	mnemonic:		CCF
	case 0x3f: return op_ccf(flags, pc); break;


	//////////////////////////////////////////
	////////	OR, XOR, AND, CP instructions
	//////////////////////////////////////////

	//	mnemonic:		CP
	case 0xbf: return op_cp(registers.A, registers.A, flags, registers, pc); break;
	case 0xb8: return op_cp(registers.A, registers.B, flags, registers, pc); break;
	case 0xb9: return op_cp(registers.A, registers.C, flags, registers, pc); break;
	case 0xba: return op_cp(registers.A, registers.D, flags, registers, pc); break;
	case 0xbb: return op_cp(registers.A, registers.E, flags, registers, pc); break;
	case 0xbc: return op_cp(registers.A, registers.H, flags, registers, pc); break;
	case 0xbd: return op_cp(registers.A, registers.L, flags, registers, pc); break;
	case 0xbe: return op_cp(registers.A, readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0xfe: return op_cp(registers.A, readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	mnemonic:		XOR
	case 0xaf: return op_xor(registers.A, flags, registers, pc); break;
	case 0xa8: return op_xor(registers.B, flags, registers, pc); break;
	case 0xa9: return op_xor(registers.C, flags, registers, pc); break;
	case 0xaa: return op_xor(registers.D, flags, registers, pc); break;
	case 0xab: return op_xor(registers.E, flags, registers, pc); break;
	case 0xac: return op_xor(registers.H, flags, registers, pc); break;
	case 0xad: return op_xor(registers.L, flags, registers, pc); break;
	case 0xae: return op_xor(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0xee: return op_xor(readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	mnemonic:		OR B
	case 0xb0: return op_or(registers.B, flags, registers, pc); break;
	case 0xb1: return op_or(registers.C, flags, registers, pc); break;
	case 0xb2: return op_or(registers.D, flags, registers, pc); break;
	case 0xb3: return op_or(registers.E, flags, registers, pc); break;
	case 0xb4: return op_or(registers.H, flags, registers, pc); break;
	case 0xb5: return op_or(registers.L, flags, registers, pc); break;
	case 0xb6: return op_or(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0xb7: return op_or(registers.A, flags, registers, pc); break;
	case 0xf6: return op_or(readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	mnemonic:		AND X
	case 0xa7: return op_and(registers.A, flags, registers, pc); break;
	case 0xa0: return op_and(registers.B, flags, registers, pc); break;
	case 0xa1: return op_and(registers.C, flags, registers, pc); break;
	case 0xa2: return op_and(registers.D, flags, registers, pc); break;
	case 0xa3: return op_and(registers.E, flags, registers, pc); break;
	case 0xa4: return op_and(registers.H, flags, registers, pc); break;
	case 0xa5: return op_and(registers.L, flags, registers, pc); break;
	case 0xa6: return op_and(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0xe6: return op_and(readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	////////	ADD, SUB instructions

	//	mnemonic:		ADD X
	case 0x87: return op_add(registers.A, flags, registers, pc); break;
	case 0x80: return op_add(registers.B, flags, registers, pc); break;
	case 0x81: return op_add(registers.C, flags, registers, pc); break;
	case 0x82: return op_add(registers.D, flags, registers, pc); break;
	case 0x83: return op_add(registers.E, flags, registers, pc); break;
	case 0x84: return op_add(registers.H, flags, registers, pc); break;
	case 0x85: return op_add(registers.L, flags, registers, pc); break;
	case 0x86: return op_add(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0xc6: return op_add(readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	mnemonic:		ADD HL, BC
	//	length:			1
	//	cycles:			8
	//	flags:			
	//					Z - unaffected
	//					N - 0
	//					H - set if carry from bit 11 (MSB of lower nibble of high byte)
	//					C - set if carry from bit 15 (MSB)
	//	description:	Adds BC to HL
	case 0x09: return op_add_hl_u16(pc, flags, registers.B << 8 | registers.C, registers); break;
	case 0x19: return op_add_hl_u16(pc, flags, registers.D << 8 | registers.E, registers); break;
	case 0x29: return op_add_hl_u16(pc, flags, registers.H << 8 | registers.L, registers); break;
	case 0x39: return op_add_hl_u16(pc, flags, sp, registers); break;
	/*{
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
	}*/

	//	mnemonic:		ADD SP, n
	case 0xe8: return op_add_sp(pc, sp, flags, registers); break;

	//	mnemonic:		ADC A
	case 0x8f: return op_adc(registers.A, flags, registers, pc); break;
	case 0x88: return op_adc(registers.B, flags, registers, pc); break;
	case 0x89: return op_adc(registers.C, flags, registers, pc); break;
	case 0x8a: return op_adc(registers.D, flags, registers, pc); break;
	case 0x8b: return op_adc(registers.E, flags, registers, pc); break;
	case 0x8c: return op_adc(registers.H, flags, registers, pc); break;
	case 0x8d: return op_adc(registers.L, flags, registers, pc); break;
	case 0x8e: return op_adc(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0xce: return op_adc(readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	SUB X
	case 0x90: return op_sub(registers.B, flags, registers, pc); break;
	case 0x91: return op_sub(registers.C, flags, registers, pc); break;
	case 0x92: return op_sub(registers.D, flags, registers, pc); break;
	case 0x93: return op_sub(registers.E, flags, registers, pc); break;
	case 0x94: return op_sub(registers.H, flags, registers, pc); break;
	case 0x95: return op_sub(registers.L, flags, registers, pc); break;
	case 0x96: return op_sub(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0x97: return op_sub(registers.A, flags, registers, pc); break;
	case 0xd6: return op_sub(readFromMem(pc + 1), flags, registers, pc, 2, 2); break;

	//	SBC X
	case 0x98: return op_sbc(registers.B, flags, registers, pc); break;
	case 0x99: return op_sbc(registers.C, flags, registers, pc); break;
	case 0x9a: return op_sbc(registers.D, flags, registers, pc); break;
	case 0x9b: return op_sbc(registers.E, flags, registers, pc); break;
	case 0x9c: return op_sbc(registers.H, flags, registers, pc); break;
	case 0x9d: return op_sbc(registers.L, flags, registers, pc); break;
	case 0x9f: return op_sbc(registers.A, flags, registers, pc); break;
	case 0x9e: return op_sbc(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 2); break;
	case 0xde: return op_sbc_u8(readFromMem(pc + 1), flags, registers, pc); break;


	////////	JMP instructions

	//	JP nn
	case 0xc3: return op_jp(pc, ((readFromMem(pc + 2) << 8) | (readFromMem(pc + 1) & 0xff)), 0, 0, 0); break;
	case 0xca: return op_jp(pc, ((readFromMem(pc + 2) << 8) | (readFromMem(pc + 1) & 0xff)), 1, flags.Z, 1); break;
	case 0xc2: return op_jp(pc, ((readFromMem(pc + 2) << 8) | (readFromMem(pc + 1) & 0xff)), 1, flags.Z, 0); break;
	case 0xd2: return op_jp(pc, ((readFromMem(pc + 2) << 8) | (readFromMem(pc + 1) & 0xff)), 1, flags.C, 0); break;
	case 0xda: return op_jp(pc, ((readFromMem(pc + 2) << 8) | (readFromMem(pc + 1) & 0xff)), 1, flags.C, 1); break;
	case 0xe9: 
		pc =(registers.H << 8) | registers.L;
		//	return m-cycles
		return 1;
		break;

	//	JR r
	case 0x18: return op_jr(pc, (int8_t)readFromMem(pc + 1), 0, 0, 0); break;
	case 0x20: return op_jr(pc, (int8_t)readFromMem(pc + 1), 1, flags.Z, 0); break;
	case 0x30: return op_jr(pc, (int8_t)readFromMem(pc + 1), 1, flags.C, 0); break;
	case 0x38: return op_jr(pc, (int8_t)readFromMem(pc + 1), 1, flags.C, 1); break;
	case 0x28: return op_jr(pc, (int8_t)readFromMem(pc + 1), 1, flags.Z, 1); break;

	//
	//	From here on CB-Prefix OpCodes
	//
	case 0xcb:
		switch (readFromMem(pc + 1))
		{

		//	RLC - L2 - T8 - Z00C
		//	Rotate left TO carry bit
		case 0x00: return op_rlc(registers.B, flags, registers, pc); break;
		case 0x01: return op_rlc(registers.C, flags, registers, pc); break;
		case 0x02: return op_rlc(registers.D, flags, registers, pc); break;
		case 0x03: return op_rlc(registers.E, flags, registers, pc); break;
		case 0x04: return op_rlc(registers.H, flags, registers, pc); break;
		case 0x05: return op_rlc(registers.L, flags, registers, pc); break;
		case 0x06: return op_rlc(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 4); break;
		case 0x07: return op_rlc(registers.A, flags, registers, pc); break;

		//	RRC - L2 - T8 - Z00C
		//	Rotate right TO carry bit
		case 0x08: return op_rrc(registers.B, flags, registers, pc); break;
		case 0x09: return op_rrc(registers.C, flags, registers, pc); break;
		case 0x0a: return op_rrc(registers.D, flags, registers, pc); break;
		case 0x0b: return op_rrc(registers.E, flags, registers, pc); break;
		case 0x0c: return op_rrc(registers.H, flags, registers, pc); break;
		case 0x0d: return op_rrc(registers.L, flags, registers, pc); break;
		case 0x0e: return op_rrc(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 4); break;
		case 0x0f: return op_rrc(registers.A, flags, registers, pc); break;

		//	RL - L2 - T8 - Z00C
		//	Rotate left THROUGH carry flag
		case 0x10: return op_rl(registers.B, flags, registers, pc); break;
		case 0x11: return op_rl(registers.C, flags, registers, pc); break;
		case 0x12: return op_rl(registers.D, flags, registers, pc); break;
		case 0x13: return op_rl(registers.E, flags, registers, pc); break;
		case 0x14: return op_rl(registers.H, flags, registers, pc); break;
		case 0x15: return op_rl(registers.L, flags, registers, pc); break;
		case 0x16: return op_rl(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 4); break;
		case 0x17: return op_rl(registers.A, flags, registers, pc); break;

		//	RR - L2 - T8 - Z00C
		//	Rotate right THROUGH carry flag
		case 0x18: return op_rr(registers.B, flags, registers, pc); break;
		case 0x19: return op_rr(registers.C, flags, registers, pc); break;
		case 0x1a: return op_rr(registers.D, flags, registers, pc); break;
		case 0x1b: return op_rr(registers.E, flags, registers, pc); break;
		case 0x1c: return op_rr(registers.H, flags, registers, pc); break;
		case 0x1d: return op_rr(registers.L, flags, registers, pc); break;
		case 0x1e: return op_rr(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 4); break;
		case 0x1f: return op_rr(registers.A, flags, registers, pc); break;

		//	SLA - L2 - T8 - Z00C
		//	Shift left into CF
		case 0x20: return op_sla(registers.B, flags, registers, pc); break;
		case 0x21: return op_sla(registers.C, flags, registers, pc); break;
		case 0x22: return op_sla(registers.D, flags, registers, pc); break;
		case 0x23: return op_sla(registers.E, flags, registers, pc); break;
		case 0x24: return op_sla(registers.H, flags, registers, pc); break;
		case 0x25: return op_sla(registers.L, flags, registers, pc); break;
		case 0x26: return op_sla(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 4); break;
		case 0x27: return op_sla(registers.A, flags, registers, pc); break;

		//	SRA - L2 - T8 - Z00C
		//	Shift right into CF, keep MSB
		case 0x28: return op_sra(registers.B, flags, registers, pc); break;
		case 0x29: return op_sra(registers.C, flags, registers, pc); break;
		case 0x2a: return op_sra(registers.D, flags, registers, pc); break;
		case 0x2b: return op_sra(registers.E, flags, registers, pc); break;
		case 0x2c: return op_sra(registers.H, flags, registers, pc); break;
		case 0x2d: return op_sra(registers.L, flags, registers, pc); break;
		case 0x2e: return op_sra(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 4); break;
		case 0x2f: return op_sra(registers.A, flags, registers, pc); break;

		//	SWAP - L2 - T8 - Z000
		//	Swap high and low nibble
		case 0x30: return op_swap(registers.B, flags, registers, pc); break;
		case 0x31: return op_swap(registers.C, flags, registers, pc); break;
		case 0x32: return op_swap(registers.D, flags, registers, pc); break;
		case 0x33: return op_swap(registers.E, flags, registers, pc); break;
		case 0x34: return op_swap(registers.H, flags, registers, pc); break;
		case 0x35: return op_swap(registers.L, flags, registers, pc); break;
		case 0x36: return op_swap(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 4); break;
		case 0x37: return op_swap(registers.A, flags, registers, pc); break;

		//	SRL - L2 - T8 - Z00C
		//	Shift right into CF
		case 0x38: return op_srl(registers.B, flags, registers, pc); break;
		case 0x39: return op_srl(registers.C, flags, registers, pc); break;
		case 0x3a: return op_srl(registers.D, flags, registers, pc); break;
		case 0x3b: return op_srl(registers.E, flags, registers, pc); break;
		case 0x3c: return op_srl(registers.H, flags, registers, pc); break;
		case 0x3d: return op_srl(registers.L, flags, registers, pc); break;
		case 0x3e: return op_srl(readFromMem((registers.H << 8) | registers.L), flags, registers, pc, 4); break;
		case 0x3f: return op_srl(registers.A, flags, registers, pc); break;

		//	BIT - L2 - T8 - Z01/
		//	Test Bit
		case 0x40: return op_bit(registers.B, 0, flags, registers, pc); break;
		case 0x41: return op_bit(registers.C, 0, flags, registers, pc); break;
		case 0x42: return op_bit(registers.D, 0, flags, registers, pc); break;
		case 0x43: return op_bit(registers.E, 0, flags, registers, pc); break;
		case 0x44: return op_bit(registers.H, 0, flags, registers, pc); break;
		case 0x45: return op_bit(registers.L, 0, flags, registers, pc); break;
		case 0x46: return op_bit(readFromMem((registers.H << 8) | registers.L), 0, flags, registers, pc, 3); break;
		case 0x47: return op_bit(registers.A, 0, flags, registers, pc); break;
		case 0x48: return op_bit(registers.B, 1, flags, registers, pc); break;
		case 0x49: return op_bit(registers.C, 1, flags, registers, pc); break;
		case 0x4a: return op_bit(registers.D, 1, flags, registers, pc); break;
		case 0x4b: return op_bit(registers.E, 1, flags, registers, pc); break;
		case 0x4c: return op_bit(registers.H, 1, flags, registers, pc); break;
		case 0x4d: return op_bit(registers.L, 1, flags, registers, pc); break;
		case 0x4e: return op_bit(readFromMem((registers.H << 8) | registers.L), 1, flags, registers, pc, 3); break;
		case 0x4f: return op_bit(registers.A, 1, flags, registers, pc); break;
		case 0x50: return op_bit(registers.B, 2, flags, registers, pc); break;
		case 0x51: return op_bit(registers.C, 2, flags, registers, pc); break;
		case 0x52: return op_bit(registers.D, 2, flags, registers, pc); break;
		case 0x53: return op_bit(registers.E, 2, flags, registers, pc); break;
		case 0x54: return op_bit(registers.H, 2, flags, registers, pc); break;
		case 0x55: return op_bit(registers.L, 2, flags, registers, pc); break;
		case 0x56: return op_bit(readFromMem((registers.H << 8) | registers.L), 2, flags, registers, pc, 3); break;
		case 0x57: return op_bit(registers.A, 2, flags, registers, pc); break;
		case 0x58: return op_bit(registers.B, 3, flags, registers, pc); break;
		case 0x59: return op_bit(registers.C, 3, flags, registers, pc); break;
		case 0x5a: return op_bit(registers.D, 3, flags, registers, pc); break;
		case 0x5b: return op_bit(registers.E, 3, flags, registers, pc); break;
		case 0x5c: return op_bit(registers.H, 3, flags, registers, pc); break;
		case 0x5d: return op_bit(registers.L, 3, flags, registers, pc); break;
		case 0x5e: return op_bit(readFromMem((registers.H << 8) | registers.L), 3, flags, registers, pc, 3); break;
		case 0x5f: return op_bit(registers.A, 3, flags, registers, pc); break;
		case 0x60: return op_bit(registers.B, 4, flags, registers, pc); break;
		case 0x61: return op_bit(registers.C, 4, flags, registers, pc); break;
		case 0x62: return op_bit(registers.D, 4, flags, registers, pc); break;
		case 0x63: return op_bit(registers.E, 4, flags, registers, pc); break;
		case 0x64: return op_bit(registers.H, 4, flags, registers, pc); break;
		case 0x65: return op_bit(registers.L, 4, flags, registers, pc); break;
		case 0x66: return op_bit(readFromMem((registers.H << 8) | registers.L), 4, flags, registers, pc, 3); break;
		case 0x67: return op_bit(registers.A, 4, flags, registers, pc); break;
		case 0x68: return op_bit(registers.B, 5, flags, registers, pc); break;
		case 0x69: return op_bit(registers.C, 5, flags, registers, pc); break;
		case 0x6a: return op_bit(registers.D, 5, flags, registers, pc); break;
		case 0x6b: return op_bit(registers.E, 5, flags, registers, pc); break;
		case 0x6c: return op_bit(registers.H, 5, flags, registers, pc); break;
		case 0x6d: return op_bit(registers.L, 5, flags, registers, pc); break;
		case 0x6e: return op_bit(readFromMem((registers.H << 8) | registers.L), 5, flags, registers, pc, 3); break;
		case 0x6f: return op_bit(registers.A, 5, flags, registers, pc); break;
		case 0x70: return op_bit(registers.B, 6, flags, registers, pc); break;
		case 0x71: return op_bit(registers.C, 6, flags, registers, pc); break;
		case 0x72: return op_bit(registers.D, 6, flags, registers, pc); break;
		case 0x73: return op_bit(registers.E, 6, flags, registers, pc); break;
		case 0x74: return op_bit(registers.H, 6, flags, registers, pc); break;
		case 0x75: return op_bit(registers.L, 6, flags, registers, pc); break;
		case 0x76: return op_bit(readFromMem((registers.H << 8) | registers.L), 6, flags, registers, pc, 3); break;
		case 0x77: return op_bit(registers.A, 6, flags, registers, pc); break;
		case 0x78: return op_bit(registers.B, 7, flags, registers, pc); break;
		case 0x79: return op_bit(registers.C, 7, flags, registers, pc); break;
		case 0x7a: return op_bit(registers.D, 7, flags, registers, pc); break;
		case 0x7b: return op_bit(registers.E, 7, flags, registers, pc); break;
		case 0x7c: return op_bit(registers.H, 7, flags, registers, pc); break;
		case 0x7d: return op_bit(registers.L, 7, flags, registers, pc); break;
		case 0x7e: return op_bit(readFromMem((registers.H << 8) | registers.L), 7, flags, registers, pc, 3); break;
		case 0x7f: return op_bit(registers.A, 7, flags, registers, pc); break;

		//	RES - L2 - T8 - ////
		//	Reset Bit
		case 0x80: return op_res(registers.B, 0, flags, registers, pc); break;
		case 0x81: return op_res(registers.C, 0, flags, registers, pc); break;
		case 0x82: return op_res(registers.D, 0, flags, registers, pc); break;
		case 0x83: return op_res(registers.E, 0, flags, registers, pc); break;
		case 0x84: return op_res(registers.H, 0, flags, registers, pc); break;
		case 0x85: return op_res(registers.L, 0, flags, registers, pc); break;
		case 0x86: return op_res(readFromMem((registers.H << 8) | registers.L), 0, flags, registers, pc, 4); break;
		case 0x87: return op_res(registers.A, 0, flags, registers, pc); break;
		case 0x88: return op_res(registers.B, 1, flags, registers, pc); break;
		case 0x89: return op_res(registers.C, 1, flags, registers, pc); break;
		case 0x8a: return op_res(registers.D, 1, flags, registers, pc); break;
		case 0x8b: return op_res(registers.E, 1, flags, registers, pc); break;
		case 0x8c: return op_res(registers.H, 1, flags, registers, pc); break;
		case 0x8d: return op_res(registers.L, 1, flags, registers, pc); break;
		case 0x8e: return op_res(readFromMem((registers.H << 8) | registers.L), 1, flags, registers, pc, 4); break;
		case 0x8f: return op_res(registers.A, 1, flags, registers, pc); break;
		case 0x90: return op_res(registers.B, 2, flags, registers, pc); break;
		case 0x91: return op_res(registers.C, 2, flags, registers, pc); break;
		case 0x92: return op_res(registers.D, 2, flags, registers, pc); break;
		case 0x93: return op_res(registers.E, 2, flags, registers, pc); break;
		case 0x94: return op_res(registers.H, 2, flags, registers, pc); break;
		case 0x95: return op_res(registers.L, 2, flags, registers, pc); break;
		case 0x96: return op_res(readFromMem((registers.H << 8) | registers.L), 2, flags, registers, pc, 4); break;
		case 0x97: return op_res(registers.A, 2, flags, registers, pc); break;
		case 0x98: return op_res(registers.B, 3, flags, registers, pc); break;
		case 0x99: return op_res(registers.C, 3, flags, registers, pc); break;
		case 0x9a: return op_res(registers.D, 3, flags, registers, pc); break;
		case 0x9b: return op_res(registers.E, 3, flags, registers, pc); break;
		case 0x9c: return op_res(registers.H, 3, flags, registers, pc); break;
		case 0x9d: return op_res(registers.L, 3, flags, registers, pc); break;
		case 0x9e: return op_res(readFromMem((registers.H << 8) | registers.L), 3, flags, registers, pc, 4); break;
		case 0x9f: return op_res(registers.A, 3, flags, registers, pc); break;
		case 0xa0: return op_res(registers.B, 4, flags, registers, pc); break;
		case 0xa1: return op_res(registers.C, 4, flags, registers, pc); break;
		case 0xa2: return op_res(registers.D, 4, flags, registers, pc); break;
		case 0xa3: return op_res(registers.E, 4, flags, registers, pc); break;
		case 0xa4: return op_res(registers.H, 4, flags, registers, pc); break;
		case 0xa5: return op_res(registers.L, 4, flags, registers, pc); break;
		case 0xa6: return op_res(readFromMem((registers.H << 8) | registers.L), 4, flags, registers, pc, 4); break;
		case 0xa7: return op_res(registers.A, 4, flags, registers, pc); break;
		case 0xa8: return op_res(registers.B, 5, flags, registers, pc); break;
		case 0xa9: return op_res(registers.C, 5, flags, registers, pc); break;
		case 0xaa: return op_res(registers.D, 5, flags, registers, pc); break;
		case 0xab: return op_res(registers.E, 5, flags, registers, pc); break;
		case 0xac: return op_res(registers.H, 5, flags, registers, pc); break;
		case 0xad: return op_res(registers.L, 5, flags, registers, pc); break;
		case 0xae: return op_res(readFromMem((registers.H << 8) | registers.L), 5, flags, registers, pc, 4); break;
		case 0xaf: return op_res(registers.A, 5, flags, registers, pc); break;
		case 0xb0: return op_res(registers.B, 6, flags, registers, pc); break;
		case 0xb1: return op_res(registers.C, 6, flags, registers, pc); break;
		case 0xb2: return op_res(registers.D, 6, flags, registers, pc); break;
		case 0xb3: return op_res(registers.E, 6, flags, registers, pc); break;
		case 0xb4: return op_res(registers.H, 6, flags, registers, pc); break;
		case 0xb5: return op_res(registers.L, 6, flags, registers, pc); break;
		case 0xb6: return op_res(readFromMem((registers.H << 8) | registers.L), 6, flags, registers, pc, 4); break;
		case 0xb7: return op_res(registers.A, 6, flags, registers, pc); break;
		case 0xb8: return op_res(registers.B, 7, flags, registers, pc); break;
		case 0xb9: return op_res(registers.C, 7, flags, registers, pc); break;
		case 0xba: return op_res(registers.D, 7, flags, registers, pc); break;
		case 0xbb: return op_res(registers.E, 7, flags, registers, pc); break;
		case 0xbc: return op_res(registers.H, 7, flags, registers, pc); break;
		case 0xbd: return op_res(registers.L, 7, flags, registers, pc); break;
		case 0xbe: return op_res(readFromMem((registers.H << 8) | registers.L), 7, flags, registers, pc, 4); break;
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
		case 0xc6: return op_set(readFromMem((registers.H << 8) | registers.L), 0, flags, registers, pc, 4); break;
		case 0xce: return op_set(readFromMem((registers.H << 8) | registers.L), 1, flags, registers, pc, 4); break;
		case 0xd6: return op_set(readFromMem((registers.H << 8) | registers.L), 2, flags, registers, pc, 4); break;
		case 0xde: return op_set(readFromMem((registers.H << 8) | registers.L), 3, flags, registers, pc, 4); break;
		case 0xe6: return op_set(readFromMem((registers.H << 8) | registers.L), 4, flags, registers, pc, 4); break;
		case 0xee: return op_set(readFromMem((registers.H << 8) | registers.L), 5, flags, registers, pc, 4); break;
		case 0xf6: return op_set(readFromMem((registers.H << 8) | registers.L), 6, flags, registers, pc, 4); break;
		case 0xfe: return op_set(readFromMem((registers.H << 8) | registers.L), 7, flags, registers, pc, 4); break;

		default:
			printf("Unsupported CB-prefixed opcode: 0x%02x at 0x%04x\n\n\n", readFromMem(pc + 1), pc);
			std::exit(EXIT_FAILURE);
			break;
		}
		break;

	default:
		//printf("Unsupported opcode: 0x%02x at 0x%04x\n\n\n", opcode, pc);
		//std::exit(EXIT_FAILURE);
		pc++;
		return 0;
		break;
	}

	//	formatting for disassembly output
	#ifdef DEBUG
	printf("\n");
	#endif
}

