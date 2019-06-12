#define _CRT_SECURE_NO_DEPRECATE
#include "SDL2/include/SDL.h"
#include <fstream>
#include "cpu.h"
#include "ppu.h"
#include "structs.h"
#include <iostream>
#include <chrono>					//	keep track of time / frames 
#include <thread>
#include <Windows.h>
#include <WinUser.h>
#include "SDL2/include/SDL_syswm.h"
#undef main

//	Main RAM: 8K Byte
//	Video RAM: 8K Byte
//	Resolution: 160x144 (20x18 tiles)
//	Max # of Sprites: 40
//	Max # of Sprites/line: 10
//	Clock speed: 4.194304 MHz
//	Horiz Sync: 9198 KHz
//	Vert Sync: 59.73 Hz
//	Sound: 4channels, stereo sound
//	1 machine cycle = 4 clock cycles
//
//	CPU arch similar to 8080 and Z80


//	General Memory Notes
//	0xF		=	1Nibble	= 4 Bit
//	0xFF	=	1Byte	= 8 Bit
//	0x1000	=	4kB
//	0x2000	=	8kB
//	0x4000	=	16kB
//	0x8000	=	32kB
//	0xFFFF	=	64kb ??

//	Memory Map
//	
//
//	0xFFFF	-	Interrupt Enable Register
//	0xFF80	-	Internal RAM
//	0xFF4C	-	Empty but unusable for I/O
//	0xFF00	-	I/O Ports
//	0xFEA0	-	Empty but unusable for I/O
//	0xFE00	-	Sprite Attrib Memory (0AM)
//	0xE000	-	Echo of 8kB internal RAM	//	Everything in the internal RAM will be mirrored, e.g. write to 0xCFF will write to 0xEFF as well
//	0xC000	-	8kB internal RAM
//	0xA000	-	8kB switchable RAM bank
//	0x8000	-	8kB video RAM
//	0x4000	-	16kB switchable ROM bank	]---	Both ROMs = 32kB Cartridge
//	0x0000	-	16kB ROM bank #0			]

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

uint8_t timings[256] = {
	1,3,2,2,1,1,2,1,5,2,2,2,1,1,2,1,
	0,3,2,2,1,1,2,1,3,2,2,2,1,1,2,1,
	2,3,2,2,1,1,2,1,2,2,2,2,1,1,2,1,
	2,3,2,2,3,3,3,1,2,2,2,2,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	2,2,2,2,2,2,0,2,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	1,1,1,1,1,1,2,1,1,1,1,1,1,1,2,1,
	2,3,3,4,3,4,2,4,2,4,3,0,3,6,2,4,
	2,3,3,0,3,4,2,4,2,4,3,0,3,0,2,4,
	3,3,2,0,0,4,2,4,4,1,4,0,0,0,2,4,
	3,3,2,1,0,4,2,4,3,2,4,1,0,0,2,4
};

uint8_t timings_cb[256] = {
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,3,2,2,2,2,2,2,2,3,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2,
	2,2,2,2,2,2,4,2,2,2,2,2,2,2,4,2
};

//	Debug Vars
unsigned char cartridge[0xFFFF];	//	64kb buffer (so max 64kb cartridges are supported for now with this buffer)
const char* filename = "instr_timing.gb";

/*	blargg's tests filesnames:

	01 - passed
	02 - passed
	03 - passed
	04 - passed
	05 - passed
	06 - passed
	07 - passed
	08 - passed
	09 - passed
	10 - passed
	11 - passed
*/

//	Main Vars
SDL_Window* mainWindow;				//	Main Window
SDL_Event event;					//	Eventhandler for all SDL events
uint16_t pc = 0x0000;				//	Program Counter; our pointer that points to the current opcode; full 16 bit / 2 byte for adressing the whole memory (0x0000 - 0xffff)
uint16_t sp = 0x0000;				//	Stack Pointer; full 16 bit / 2 byte for adressing the whole memory (0x0000 - 0xffff)
unsigned char memory[0x10000];		//	Memory
int interrupts_enabled = 0;
Registers registers;
Flags flags;
bool ROMloaded = 1;
int timer_clocksum = 0;
int div_clocksum = 0;
int tmp = 0;

void initWindow();
void handleTimer(int clocks);
void handleWindowEvents(SDL_Event event);
void handleInterrupts();

int main() {

	//	Init Registers
	registers.A = 0x0;
	registers.B = 0x0;
	registers.C = 0x0;
	registers.D = 0x0;
	registers.E = 0x0;
	registers.F = 0x0;
	registers.H = 0x0;
	registers.L = 0x0;

	//	Init Flags
	flags.Z = 0;
	flags.N = 0;
	flags.H = 0;
	flags.C = 0;

	//	load cartridge
	FILE* file = fopen(filename, "rb");
	int pos = 0;
	while (fread(&cartridge[pos], 1, 1, file)) {
		pos++;
	}
	fclose(file);

	//	copy cartridge to memory
	for (int i = 0; i < sizeof(memory); i++)
	{
		memory[i] = cartridge[i];
	}

	//	move boot ROM to memory
	for (int i = 0; i < sizeof(gameboyBootROM); i++) {
		memory[i] = gameboyBootROM[i];
	}

	//	init PPU
	initPPU();

	//	init Window & create menu
	initWindow();

	//	init timers
	auto t_start = std::chrono::high_resolution_clock::now();

	//	start CPU
	while (1) {
		//	reset timer
		t_start = std::chrono::high_resolution_clock::now();

		//	mix cpu and ppu together, framebased
		for (int i = 0; i < 154; i++) {

			//	set LY
			memory[0xff44] = i;

			//	draw line
			if (i < 144)
				drawLine(memory);
			else if (i == 144)
				memory[0xff0f] |= 1;	//	vblank interrupt

			//	process cpu if system is not HALTed
			int sum = 0;
			int cycle = 0;
			while (sum < (70224 / 4 / 154)) {
				//	run cpu if not halted
				if (!flags.HALT) {
					//	normal timings table
					if (pc != 0xcb) {
						sum += timings[memory[pc]];
						cycle = timings[memory[pc]];
					}
					//	cb-timings table
					else {
						sum += timings_cb[memory[pc + 1]];
						cycle = timings_cb[memory[pc + 1]];
					}
					//	process the instruction
					processOpcode(pc, sp, memory, registers, flags, interrupts_enabled);
				}
				//	if system is halted just idle, but still commence timers and condition for while loop
				else {
					sum += timings[0x00];		//	NOP
					cycle = timings[0x00];
				}

				//	handle timer
				handleTimer(cycle);

				//	handle interrupts
				handleInterrupts();

				//	blarggs test - serial output
				if (memory[0xff02] == 0x81) {
					char c = memory[0xff01];
					printf("%c", c);
					memory[0xff02] = 0x0;
				}
			}
			
			//	handle window events
			handleWindowEvents(event);
		}

		//	sleep for proper cpu timing (per frame)
		auto t_end = std::chrono::high_resolution_clock::now();
		double elapsed = 16.667 - std::chrono::duration<double, std::milli>(t_end - t_start).count();
		std::this_thread::sleep_for(std::chrono::milliseconds((int)elapsed));

		//	lock ROM, load first 100 bytes from cartridge onces booted
		if (memory[0xff50] == 0x01 && ROMloaded) {
			for (int i = 0; i < sizeof(gameboyBootROM); i++) {
				memory[i] = cartridge[i];
			}
			ROMloaded = 0;
		}
	}

	//	stop PPU
	stopPPU();
	
	return 0;
}

void handleTimer(int cycle) {
	//	set divider
	div_clocksum += cycle * 4;
	if (div_clocksum >= 256) {
		div_clocksum -= 256;
		memory[0xff04]++;
	}

	//	check if timer is on
	if ((memory[0xff07] >> 2) & 0x1) {
		//	increase helper counter
		timer_clocksum += cycle * 4;

		//	set frequency
		int freq = 4096;					//	Hz
		if ((memory[0xff07] & 3) == 1)		//	mask last 2 bits
			freq = 262144;
		else if ((memory[0xff07] & 3) == 2)	//	mask last 2 bits
			freq = 65536;
		else if ((memory[0xff07] & 3) == 3)	//	mask last 2 bits
			freq = 16384;

		//	increment the timer according to the frequency (synched to the processed opcodes)
		while (timer_clocksum >= (4194304 / freq)) {
			//	increase TIMA
			memory[0xff05]++;
			//	check TIMA for overflow
			if (memory[0xff05] == 0x00) {
				//	set timer interrupt request
				memory[0xff0f] |= 4;
				//	reset timer to timer modulo
				memory[0xff05] = memory[0xff06];
			}
			timer_clocksum -= (4194304 / freq);
		}
	}
}

void handleInterrupts() {
	//	unHALT the system, once we have any interrupt
	if (memory[0xffff] & memory[0xff0f] && flags.HALT) {
		flags.HALT = 0;
		printf("unHALTed");
	}

	//	handle interrupts
	if (interrupts_enabled) {
		//	some interrupt is enabled and allowed
		if (memory[0xffff] & memory[0xff0f]) {
			//	handle interrupts by priority (starting at bit 0 - vblank)
			
			//	v-blank interrupt
			if ((memory[0xffff] & 1) & (memory[0xff0f] & 1)) {
				sp--;
				memory[sp] = pc >> 8;
				sp--;
				memory[sp] = pc & 0xff;
				pc = 0x40;
				memory[0xff0f] &= 0xff & ~1;
			}

			//	timer interrupt
			else if ((memory[0xffff] & 4) & (memory[0xff0f] & 4)) {
				printf("you shouldn't be able to see this line!\n");
				sp--;
				memory[sp] = pc >> 8;
				sp--;
				memory[sp] = pc & 0xff;
				pc = 0x50;
				memory[0xff0f] &= 0xff & ~4;
			}

			//	clear main interrupts_enable and corresponding flag
			interrupts_enabled = false;
		}
	}
}

void initWindow() {
	mainWindow = getWindow();
	char title[50];
	snprintf(title, sizeof title, "[ q00.gb ][ rom: %s ]", filename);
	SDL_SetWindowTitle(mainWindow, title);
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(mainWindow, &wmInfo);
	HWND hwnd = wmInfo.info.win.window;
	HMENU hMenuBar = CreateMenu();
	HMENU hFile = CreateMenu();
	HMENU hEdit = CreateMenu();
	HMENU hHelp = CreateMenu();
	HMENU hDebugger = CreateMenu();
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, "File");
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hEdit, "Edit");
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelp, "Help");
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hDebugger, "Debugger");
	AppendMenu(hFile, MF_STRING, 0, "Load ROM");
	AppendMenu(hFile, MF_STRING, 1, "Exit");
	AppendMenu(hEdit, MF_STRING, 2, "Configure Controls");
	AppendMenu(hHelp, MF_STRING, 3, "About");
	AppendMenu(hDebugger, MF_STRING, 4, "» Tile Map");
	AppendMenu(hDebugger, MF_STRING, 5, "» BG Map");
	SetMenu(hwnd, hMenuBar);

	//	Enable WM events for SDL Window
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
}

void handleWindowEvents( SDL_Event event) {
	//	poll events from menu
	SDL_PollEvent(&event);
	switch (event.type)
	{
		case SDL_SYSWMEVENT:
			if (event.syswm.msg->msg.win.msg == WM_COMMAND) {
				//	Tile Map
				if (LOWORD(event.syswm.msg->msg.win.wParam) == 4) {
					SDL_Window* tWindow;
					SDL_Renderer* tRenderer;
					SDL_CreateWindowAndRenderer(256, 256, 0, &tWindow, &tRenderer);
					SDL_SetWindowSize(tWindow, 256, 512);
					drawBGTileset(memory, tRenderer, tWindow);
				}
				//	BG Map
				if (LOWORD(event.syswm.msg->msg.win.wParam) == 5) {
					SDL_Window* tWindow;
					SDL_Renderer* tRenderer;
					SDL_CreateWindowAndRenderer(256, 256, 0, &tWindow, &tRenderer);
					SDL_SetWindowSize(tWindow, 512, 512);
					drawBGMap(memory, tRenderer, tWindow);
				}
			}
			//	close a window
			if (event.syswm.msg->msg.win.msg == WM_CLOSE) {
				DestroyWindow(event.syswm.msg->msg.win.hwnd);
				PostMessage(event.syswm.msg->msg.win.hwnd, WM_CLOSE, 0, 0);
			}
			break;
	};
}