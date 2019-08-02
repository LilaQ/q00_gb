#define _CRT_SECURE_NO_DEPRECATE
#include "main.h"
#include "mmu.h"
#include "spu.h"
#include "structs.h"
#include "main.h"
#include <iostream>
#include <cstdint>

unsigned char gameboyBootROM[256] = {
	0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB,
	0x21, 0x26, 0xFF, 0x0E, 0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3,
	0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0, 0x47, 0x11, 0x04, 0x01,
	0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
	0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22,
	0x23, 0x05, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99,
	0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, 0x2E, 0x0F, 0x18,
	0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
	0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20,
	0xF7, 0x1D, 0x20, 0xF2, 0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62,
	0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06, 0x7B, 0xE2, 0x0C, 0x3E,
	0x87, 0xE2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
	0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17,
	0xC1, 0xCB, 0x11, 0x17, 0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9,
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
	0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63,
	0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
	0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C, 0x21, 0x04, 0x01, 0x11,
	0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
	0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE,
	0x3E, 0x01, 0xE0, 0x50
};

unsigned char memory[0xFA000];		//	Memory
unsigned char rom[0xFA000];			//	Cartridge
unsigned char romtype = 0x00;
unsigned char *ram[8];				//	pointer to RAM
uint8_t PPUstate = 2;				//	drawing mode the PPU currently is in

uint8_t mbc1romNumber = 1;
uint8_t mbc1romMode = 0;
uint8_t mbc1ramNumber = 0;
uint8_t mbc1ramEnabled = false;

uint8_t mbc2romNumber = 1;
uint8_t mbc2romMode = 0;
uint8_t mbc2ramNumber = 0;
uint8_t mbc2ramEnabled = false;

uint8_t mbc3romNumber = 1;
uint8_t mbc3romMode = 0;
uint8_t mbc3ramNumber = 0;
uint8_t mbc3ramEnabled = false;

void initMMU() {

	//	clear memory
	//memset(memory, 0, sizeof(memory));

	//	set up ram, default 8kb
	//ram[0] = new unsigned char[0x2000];

	//	load bootROM
	loadBootROM();
}

//	read the actual ROM type from the cartridge
void initROM() {

	//	read and set romtype (MBC0, MBC1..)
	romtype = readFromMem(0x0147);
	if (romtype < 0x04 && romtype != 0x00)
		romtype = 0x01;
	if (romtype >= 0x0f && romtype <= 0x13)
		romtype = 0x03;
	if (romtype == 0x05 || romtype == 0x06)
		romtype = 0x02;

	//	read and set RAM size accordingly to rom
	switch (memory[0x0149])
	{
	case 0x01:
		ram[0] = new unsigned char[0x800];
		break;
	case 0x02 :
		ram[0] = new unsigned char[0x2000];
		break;
	case 0x03 :
		ram[0] = new unsigned char[0x8000];
		ram[1] = new unsigned char[0x8000];
		ram[2] = new unsigned char[0x8000];
		ram[3] = new unsigned char[0x8000];
		break;
	case 0x04 :
		ram[0] = new unsigned char[0x8000];	//	prolly +mbc2 only
		ram[1] = new unsigned char[0x8000];
		ram[2] = new unsigned char[0x8000];
		ram[3] = new unsigned char[0x8000];
		ram[4] = new unsigned char[0x8000];
		ram[5] = new unsigned char[0x8000];
		ram[6] = new unsigned char[0x8000];
		ram[7] = new unsigned char[0x8000];
		break;
	case 0x05 :
		ram[0] = new unsigned char[0x8000];	//	prolly +mbc2 only
		ram[1] = new unsigned char[0x8000];
		ram[2] = new unsigned char[0x8000];
		ram[3] = new unsigned char[0x8000];
		ram[4] = new unsigned char[0x8000];
		ram[5] = new unsigned char[0x8000];
		ram[6] = new unsigned char[0x8000];
		ram[7] = new unsigned char[0x8000];
		break;
	}

	//	initial wave channel data (some games rely on this data)
	//	GB: 84 40 43 AA 2D 78 92 3C 60 59 59 B0 34 B8 2E DA
	//	CGB: 00 FF 00 FF 00 FF 00 FF 00 FF 00 FF 00 FF 00 FF
	//	TODO: CGB has different data
	memory[0xff30] = 0x84;
	memory[0xff31] = 0x40;
	memory[0xff32] = 0x43;
	memory[0xff33] = 0xaa;
	memory[0xff34] = 0x2d;
	memory[0xff35] = 0x78;
	memory[0xff36] = 0x92;
	memory[0xff37] = 0x3c;
	memory[0xff38] = 0x60;
	memory[0xff39] = 0x59;
	memory[0xff3a] = 0x59;
	memory[0xff3b] = 0xb0;
	memory[0xff3c] = 0x34;
	memory[0xff3d] = 0xb8;
	memory[0xff3e] = 0x2e;
	memory[0xff3f] = 0xda;
}

//	move boot ROM to memory
void loadBootROM() {
	
	for (int i = 0; i < sizeof(gameboyBootROM); i++) {
		memory[i] = gameboyBootROM[i];
	}
}

//	lock ROM, load first 100 bytes from cartridge onces booted
void lockBootROM() {
	for (int i = 0; i < sizeof(gameboyBootROM); i++) {
		memory[i] = rom[i];
	}
}

//	copy sprites to oam
void dmaOAMtransfer() {
	uint16_t src = readFromMem(0xff46) * 0x100;
	for (int i = 0; i < 40; i++) {
		writeToMem(0xfe00 + (i * 4), readFromMem(src + (i * 4)));
		writeToMem(0xfe00 + (i * 4) + 1, readFromMem(src + (i * 4) + 1));
		writeToMem(0xfe00 + (i * 4) + 2, readFromMem(src + (i * 4) + 2));
		writeToMem(0xfe00 + (i * 4) + 3, readFromMem(src + (i * 4) + 3));
	}
}

//	copy cartridge to memory
void loadROM(unsigned char c[]) {
	for (int i = 0; i < 0x8000; i++) {
		memory[i] = c[i];
	}
	for (int i = 0; i < 0xFA000; i++) {
		rom[i] = c[i];
	}

	//	overwrite 256 bytes with bootROM
	loadBootROM();

	//	set ROM type
	initROM();
}

//	set the mode, the PPU is currently in (important for OAM / VRAM access / denying access on reads / writes)
void setPPUstate(uint8_t state) {
	PPUstate = state;
}

//	write unsigned char to memory
void writeToMem(uint16_t adr, unsigned char val) {

	//	VRAM / OAM access regulations, depending on PPU state
	//	VRAM accessible in mode 0,1,2	|	0x8000-0x9fff
	//	OAM accessible in mode 0,1		|	0xfe00-0xfe9f
	/*if (adr >= 0x8000 && adr <= 0x9fff && PPUstate == 3) {
		return;
	}
	if (adr >= 0xfe00 && adr <= 0xfe9f && (PPUstate == 3 || PPUstate == 2)) {
		return;
	}*/

	//	[0xff26 - enable sound, all channels]
	if (adr == 0xff26) {
		if (val)
			memory[0xff26] = 0xff;
		else
			memory[0xff26] = 0x00;
		return;
	}

	//	[0xff00] - joypad input
	if (adr == 0xff00) {
		memory[adr] = readInput(val);
		return;
	}

	//	[0xff50] - lock bootrom
	else if (adr == 0xff50 && val == 1) {
		lockBootROM();
		memory[adr] = val;
	}

	//	[0xff46] - oam dma transer
	else if (adr == 0xff46) {
		dmaOAMtransfer();
		memory[adr] = val;
	}

	//	[0xff11] - SC1 trigger
	else if (adr == 0xff14 && (val >> 7)) {
		memory[adr] = val;
		resetSC1length(readFromMem(0xff11) & 0x3f);
		return;
	}
	//	[0xff21] - SC2 trigger
	else if (adr == 0xff19 && (val >> 7)) {
		memory[adr] = val;
		resetSC2length(readFromMem(0xff16) & 0x3f);
		return;
	}
	//	[0xff31] - SC3 trigger
	else if (adr == 0xff1e && (val >> 7)) {
		memory[adr] = val;
		resetSC3length(readFromMem(0xff1b));
		return;
	}
	//	[0xff41] - SC4 trigger
	else if (adr == 0xff23 && (val >> 7)) {
		memory[adr] = val;
		resetSC4length(readFromMem(0xff20) & 0x3f);
		return;
	}
	
	//	MBC0
	if (romtype == 0x00) {
		
		//	make ROM readonly
		if (adr >= 0x8000)
			memory[adr] = val;
	}
	//	MBC1
	else if (romtype == 0x01) {

		//	external RAM enable / disable
		if (adr < 0x2000) {
			mbc1ramEnabled = val > 0;
		}
		//	choose ROM bank nr (lower 5 bits, 0-4)
		else if (adr < 0x4000) {
			mbc1romNumber = val & 0x1f;
			if (val == 0x00 || val == 0x20 || val == 0x40 || val == 0x60)
				mbc1romNumber = (val & 0x1f) + 1;
		}
		//	choose RAM bank nr OR ROM bank top 2 bits (5-6)
		else if (adr < 0x6000) {
			//	mode: ROM bank 2 bits
			if (mbc1romMode == 0)
				mbc1romNumber |= (val & 3) << 5;
			//	mode: RAM bank selection
			else
				mbc1ramNumber = val & 3;
		}
		else if (adr < 0x8000) {
			mbc1romMode = val > 0;
		}
		else {
			memory[adr] = val;
		}
	}
	//	MBC2
	else if (romtype == 0x02) {
		//	external RAM enable / disable
		if (adr < 0x2000) {
			mbc2ramEnabled = val > 0;
		}
		//	choose ROM bank nr (lower 5 bits, 0-4)
		else if (adr < 0x4000) {
			mbc2romNumber = val & 0x1f;
			if (val == 0x00 || val == 0x20 || val == 0x40 || val == 0x60)
				mbc2romNumber = (val & 0x1f) + 1;
		}
		else {
			memory[adr] = val;
		}
	}
	//	MBC3
	else if (romtype == 0x03) {

		//	external RAM enable / disable
		if (adr < 0x2000) {
			mbc3ramEnabled = val > 0;
		}
		//	choose ROM bank nr
		else if (adr < 0x4000) {
			mbc3romNumber = val;
			if (val == 0x00)
				mbc3romNumber = 0x01;
		}
		//	choose RAM bank nr OR RTC register (real time clock, for ingame cycles)
		else if (adr < 0x6000) {
			mbc3ramNumber = val;
		}
		//	TODO: latch clock delay
		else if (adr < 0x8000) {
		}
		//	write to RAM
		else if (mbc3ramEnabled && mbc3ramNumber && (adr >= 0xa000 && adr < 0xc000)) {
			ram[mbc3romNumber][adr] = val;
		}
		//	any other write
		else {
			memory[adr] = val;
		}
	}
}

uint16_t getROM() {
	return mbc1romNumber;
}

//	read a byte from memory
unsigned char& readFromMem(uint16_t adr) {

	//	VRAM / OAM access regulations, depending on PPU state
	//	VRAM accessible in mode 0,1,2	|	0x8000-0x9fff
	//	OAM accessible in mode 0,1		|	0xfe00-0xfe9f
	/*unsigned char undefined_return = 0xff;
	if (adr >= 0x8000 && adr <= 0x9fff && PPUstate == 3) {
		return undefined_return;
	}
	if (adr >= 0xfe00 && adr <= 0xfe9f && (PPUstate == 3 || PPUstate == 2)) {
		return undefined_return;
	}*/

	//	MBC1 
	if (romtype == 0x01 && mbc1romNumber && (adr >= 0x4000 && adr < 0x8000)) {
		uint32_t target = (mbc1romNumber * 0x4000) + (adr - 0x4000);
		return rom[target];
	}
	//	MBC2
	else if (romtype == 0x02) {
		//	ROM banking
		if (mbc2romNumber && (adr >= 0x4000 && adr < 0x8000)) {
			uint32_t target = (mbc2romNumber * 0x4000) + (adr - 0x4000);
			return rom[target];
		}
		else
			return memory[adr];
	}
	//	MBC3
	else if (romtype == 0x03) {
		//	ROM banking
		if (mbc3romNumber && (adr >= 0x4000 && adr < 0x8000)) {
			uint32_t target = (mbc3romNumber * 0x4000) + (adr - 0x4000);
			return rom[target];
		}
		//	RAM / RTC banking
		else if (mbc3ramEnabled && mbc3ramNumber && (adr >= 0xa000 && adr < 0xc000)) {
			//	RAM bank
			if (mbc3ramNumber < 0x08) {
				return ram[mbc3ramNumber][adr];
			}
			//	RTC register
			else {
				printf("WARNING! Game tries to access unimplemented MBC3 RTC!!\n");
			}
		}
		else
			return memory[adr];
	}
	//	No-MBC
	else
		return memory[adr];
}

//	increase, decrease, add, subtract to value in memory
void aluToMem(uint16_t adr, int8_t val) {
	memory[adr] += val;
}

void saveState(char filename[], Registers& registers, Flags& flags, uint16_t pc, uint16_t sp ) {

	FILE* file = fopen(filename, "wb" );

	//	write memory
	fwrite(memory, sizeof(unsigned char), sizeof(memory), file);

	//	write registers
	unsigned char r[8] = {
		registers.A,
		registers.B,
		registers.C,
		registers.D,
		registers.E,
		registers.F,
		registers.H,
		registers.L
	};
	fwrite(r, sizeof(unsigned char), sizeof(r), file);

	//	write flags
	unsigned char f[4] = {
		flags.Z,
		flags.N,
		flags.H,
		flags.C
	};
	fwrite(f, sizeof(unsigned char), sizeof(f), file);

	//	write pointers
	uint16_t p[2] = {
		pc,
		sp
	};
	fwrite(p, sizeof(uint16_t), sizeof(p), file);

	fclose(file);

}

void loadState(char filename[], Registers& registers, Flags& flags, uint16_t pc, uint16_t sp) {

	FILE* file = fopen(filename, "rb");

	//	read memory
	int pos = 0;
	while (pos < sizeof(memory)) {
		fread(&memory[pos], sizeof(unsigned char), 1, file);
		pos++;
	}

	//	read registers
	unsigned char r[8];
	pos = 0;
	while (pos < 8) {
		fread(&r[pos], sizeof(unsigned char), 1, file);
		pos++;
	}
	registers.A = r[0];
	registers.B = r[1];
	registers.C = r[2];
	registers.D = r[3];
	registers.E = r[4];
	registers.F = r[5];
	registers.H = r[6];
	registers.L = r[7];

	//	write flags
	unsigned char f[4];
	pos = 0;
	while (pos < 4) {
		fread(&f[pos], sizeof(unsigned char), 1, file);
		pos++;
	}
	flags.Z = f[0];
	flags.N = f[1];
	flags.H = f[2];
	flags.C = f[3];

	//	read pointers
	uint16_t p[2];
	pos = 0;
	while (pos < 2) {
		fread(&p[pos], sizeof(uint16_t), 1, file);
		pos++;
	}
	pc = p[0];
	sp = p[1];

	fclose(file);
}
