#define _CRT_SECURE_NO_DEPRECATE
#include "SDL2/include/SDL.h"
#include <fstream>
#include "cpu.h"
#include "ppu.h"
#include "mmu.h"
#include "structs.h"
#include <string>
#include <string.h>
#include <iostream>
#include <chrono>					//	keep track of time / frames 
#include <thread>
#include <Windows.h>
#include <WinUser.h>
#include "SDL2/include/SDL_syswm.h"
#include "commctrl.h"
#undef main

using namespace std;

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

//	Debug Vars
unsigned char cartridge[0xFA000];	
string filename = "kirby.gb";

/*	blargg's tests filesnames:

	01 - passed (special)
	02 - passed (interrupts)
	03 - passed (op sp, hl)					| not working in 'release' build
	04 - passed (op r, imm)
	05 - passed (op rp)
	06 - passed (ld r, r)
	07 - passed (jr, jp, call, ret, rst)	| not working in 'release' build
	08 - passed (misc instructions)			| not working in 'release' build
	09 - passed (op r, r)
	10 - passed (bit ops)
	11 - passed (op a, (hl))

	inst_timing - passed
*/

//	Main Vars
SDL_Window* mainWindow;				//	Main Window
SDL_Event event;					//	Eventhandler for all SDL events
uint16_t pc = 0x0000;				//	Program Counter; our pointer that points to the current opcode; full 16 bit / 2 byte for adressing the whole memory (0x0000 - 0xffff)
uint16_t sp = 0x0000;				//	Stack Pointer; full 16 bit / 2 byte for adressing the whole memory (0x0000 - 0xffff)
uint8_t joypad = 0xff;				//	Storage for the joycon inputs
int interrupts_enabled = 0;
Registers registers;
Flags flags;
int timer_clocksum = 0;
int div_clocksum = 0;

void initWindow();
void handleTimer(int clocks);
void handleWindowEvents(SDL_Event event);
void handleInterrupts();
void handleControls(unsigned char memory[]);
void printDebug();
void showMemoryMap();
void showAbout();
int main();

//	reset Gameboy
void resetGameboy() {

	//	stop ppu
	stopPPU();

	//	reset vars
	pc = 0x0000;
	sp = 0x0000;
	joypad = 0xff;
	interrupts_enabled = 0;
	registers.A = 0x00;
	registers.B = 0x00;
	registers.C = 0x00;
	registers.D = 0x00;
	registers.E = 0x00;
	registers.H = 0x00;
	registers.L = 0x00;
	flags.C = 0x00;
	flags.H = 0x00;
	flags.N = 0x00;
	flags.Z = 0x00;
	flags.HALT = 0x00;
	timer_clocksum = 0;
	div_clocksum = 0;

	//	call main again
	main();
}

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

	//	init MMU
	initMMU();

	//	load cartridge
	FILE* file = fopen(filename.c_str(), "rb");
	int pos = 0;
	while (fread(&cartridge[pos], 1, 1, file)) {
		pos++;
	}
	fclose(file);
	loadROM(cartridge);

	//	init PPU
	initPPU();

	//	init Window & create menu
	initWindow();

	//	init timers
	auto t_start = std::chrono::high_resolution_clock::now();
	int enlog = 0;

	//	start CPU
	while (1) {
		//	reset timer
		t_start = std::chrono::high_resolution_clock::now();

		//	mix cpu and ppu together, framebased
		for (int i = 0; i < 154; i++) {

			//	set LY
			writeToMem(0xff44, i);

			//	process cpu if system is not HALTed
			int sum = 0;
			int cycle = 0;
			uint16_t last = 0;
			uint16_t lpc = 0;
			while (sum < (70224 / 4 / 154)) {
				//	run cpu if not halted
				if (!flags.HALT) {
					/*if(enlog)
						printDebug();*/

					lpc = pc;
					last = readFromMem(0xff40);
					/*if (pc == 0x2000) {
						printf("FCU");
					}*/
					/*if (pc >= 0x2000)
						printDebug();*/

					//	process the instruction
					cycle = processOpcode(pc, sp, registers, flags, interrupts_enabled);
					sum += cycle;

					if (pc == 0x1ffc)
						enlog = 1;
				}
				//	if system is halted just idle, but still commence timers and condition for while loop
				else {
					sum += 1;		//	NOP
					cycle = 1;
				}

				//	handle timer
				handleTimer(cycle);

				//	handle interrupts
				handleInterrupts();

				//	set controls
				//writeToMem(0xff00, joypad);
				writeToMem(0xff00, 0xff);

			}

			//	draw line
			if (i < 144)
				drawLine();
			else if (i == 144)
				writeToMem(0xff0f, readFromMem(0xff0f) | 1);

			//	handle window events & controls
			handleWindowEvents(event);
		}

		//	sleep for proper cpu timing (per frame)
		/*auto t_end = std::chrono::high_resolution_clock::now();
		double elapsed = 16.667 - std::chrono::duration<double, std::milli>(t_end - t_start).count();
		std::this_thread::sleep_for(std::chrono::milliseconds((int)elapsed));*/
	}

	//	stop PPU
	stopPPU();
	
	return 0;
}

void handleControls(unsigned char memory[]) {
	/*uint8_t ar[256];
	GetKeyboardState(ar);
	memory[0xff00] = 0xff;*/
	/*if (GetAsyncKeyState(VK_RIGHT)	& 0x8000)	{ memory[0xff00] |= 0x01; }
	if (GetAsyncKeyState(VK_LEFT)	& 0x8000)	{ memory[0xff00] |= 0x02; }
	if (GetAsyncKeyState(VK_UP)		& 0x8000)	{ memory[0xff00] |= 0x04; }
	if (GetAsyncKeyState(VK_DOWN)	& 0x8000)	{ memory[0xff00] |= 0x08; }
	if (GetAsyncKeyState(VK_RIGHT)	& 0x8000)	{ memory[0xff00] |= 0x01; }
	if (GetAsyncKeyState('A')		& 0x8000)	{ memory[0xff00] ^= 0x01; }
	if (GetAsyncKeyState('S')		& 0x8000)	{ memory[0xff00] ^= 0x02; }
	if (GetAsyncKeyState('Y')		& 0x8000)	{ memory[0xff00] ^= 0x04; }
	if (GetAsyncKeyState('X')		& 0x8000)	{ memory[0xff00] ^= 0x08; }*/
}

void printDebug() {
	printf(
		"PC: %04X, AF: %04X, BC: %04X, DE: %04X, HL: %04X, SP: %04X, LCDC: %04X",
		pc,
		(registers.A << 8) | (flags.Z << 7 | flags.N << 6 | flags.H << 5 | flags.C << 4),
		(registers.B << 8) | registers.C,
		(registers.D << 8) | registers.E,
		(registers.H << 8) | registers.L,
		sp,
		readFromMem(0xff40)
	);

	printf("\t(%02X %02X %02X)\n",
		readFromMem(pc),
		readFromMem(pc + 1),
		readFromMem(pc + 2)
	);
}

void handleTimer(int cycle) {
	//	set divider
	div_clocksum += cycle;
	if (div_clocksum >= 256) {
		div_clocksum -= 256;
		aluToMem(0xff04, 1);
	}

	//	check if timer is on
	if ((readFromMem(0xff07) >> 2) & 0x1) {
		//	increase helper counter
		timer_clocksum += cycle * 4;

		//	set frequency
		int freq = 4096;					//	Hz
		if ((readFromMem(0xff07) & 3) == 1)		//	mask last 2 bits
			freq = 262144;
		else if ((readFromMem(0xff07) & 3) == 2)	//	mask last 2 bits
			freq = 65536;
		else if ((readFromMem(0xff07) & 3) == 3)	//	mask last 2 bits
			freq = 16384;

		//printf("%d\n", freq);

		//	increment the timer according to the frequency (synched to the processed opcodes)
		while (timer_clocksum >= (4194304 / freq)) {
			//	increase TIMA
			aluToMem(0xff05, 1);
			//	check TIMA for overflow
			if (readFromMem(0xff05) == 0x00) {
				//	set timer interrupt request
				writeToMem(0xff0f, readFromMem(0xff0f) | 4);
				//	reset timer to timer modulo
				writeToMem(0xff05, readFromMem(0xff06));
			}
			timer_clocksum -= (4194304 / freq);
		}
	}
}

void handleInterrupts() {
	//	unHALT the system, once we have any interrupt
	if (readFromMem(0xffff) & readFromMem(0xff0f) && flags.HALT) {
		flags.HALT = 0;
	}

	//	check STAT for interrupt enables
	if (((readFromMem(0xff41) >> 2) & 0x01) & ((readFromMem(0xff41) >> 6) & 0x01))
		writeToMem(0xff0f, readFromMem(0xff0f) | 2);		//	trigger STAT interrupt

	//	handle interrupts
	if (interrupts_enabled) {
		//	some interrupt is enabled and allowed
		if (readFromMem(0xffff) & readFromMem(0xff0f)) {
			//	handle interrupts by priority (starting at bit 0 - vblank)
			
			//	v-blank interrupt
			if ((readFromMem(0xffff) & 1) & (readFromMem(0xff0f) & 1)) {
				sp--;
				writeToMem(sp, pc >> 8);
				sp--;
				writeToMem(sp, pc & 0xff);
				pc = 0x40;
				writeToMem(0xff0f, readFromMem(0xff0f) & ~1);
			}

			//	lcd stat interrupt
			if ((readFromMem(0xffff) & 2) & (readFromMem(0xff0f) & 2)) {
				sp--;
				writeToMem(sp, pc >> 8);
				sp--;
				writeToMem(sp, pc & 0xff);
				pc = 0x48;
				writeToMem(0xff0f, readFromMem(0xff0f) & ~2);
			}

			//	timer interrupt
			else if ((readFromMem(0xffff) & 4) & (readFromMem(0xff0f) & 4)) {
				sp--;
				writeToMem(sp, pc >> 8);
				sp--;
				writeToMem(sp, pc & 0xff);
				pc = 0x50;
				writeToMem(0xff0f, readFromMem(0xff0f) & ~4);
			}

			//	clear main interrupts_enable and corresponding flag
			interrupts_enabled = false;
		}
	}
}

void initWindow() {
	mainWindow = getWindow();
	char title[50];
	string rom = filename;
	if (filename.find_last_of("\\") != string::npos)
		rom = filename.substr(filename.find_last_of("\\") + 1);
	snprintf(title, sizeof title, "[ q00.gb ][ rom: %s ]", rom.c_str());
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
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, "[ main ]");
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelp, "[ help ]");
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hDebugger, "[ debugging ]");
	AppendMenu(hFile, MF_STRING, 9, "» Load ROM");
	AppendMenu(hFile, MF_STRING, 7, "» Reset");
	AppendMenu(hFile, MF_STRING, 1, "» Exit");
	AppendMenu(hFile, MF_STRING, 2, "Configure Controls");
	AppendMenu(hHelp, MF_STRING, 3, "About");
	AppendMenu(hDebugger, MF_STRING, 4, "» Tile Map");
	AppendMenu(hDebugger, MF_STRING, 5, "» BG Map");
	AppendMenu(hDebugger, MF_STRING, 10, "» Window Map");
	AppendMenu(hDebugger, MF_STRING, 6, "» Sprite Map");
	AppendMenu(hDebugger, MF_STRING, 8, "» Debugger");
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
				//	Exit
				if (LOWORD(event.syswm.msg->msg.win.wParam) == 1) {
					exit(0);
				}
				//	About
				else if (LOWORD(event.syswm.msg->msg.win.wParam) == 3) {
					showAbout();
				}
				//	Tile Map
				else if (LOWORD(event.syswm.msg->msg.win.wParam) == 4) {
					SDL_Window* tWindow;
					SDL_Renderer* tRenderer;
					SDL_CreateWindowAndRenderer(256, 256, 0, &tWindow, &tRenderer);
					SDL_SetWindowSize(tWindow, 256, 512);
					drawBGTileset(tRenderer, tWindow);
				}
				//	BG Map
				else if (LOWORD(event.syswm.msg->msg.win.wParam) == 5) {
					SDL_Window* tWindow;
					SDL_Renderer* tRenderer;
					SDL_CreateWindowAndRenderer(256, 256, 0, &tWindow, &tRenderer);
					SDL_SetWindowSize(tWindow, 512, 512);
					drawBGMap(tRenderer, tWindow);
				}
				//	Sprite Map
				else if (LOWORD(event.syswm.msg->msg.win.wParam) == 6) {
					SDL_Window* tWindow;
					SDL_Renderer* tRenderer;
					SDL_CreateWindowAndRenderer(256, 256, 0, &tWindow, &tRenderer);
					SDL_SetWindowSize(tWindow, 512, 512);
					drawSpriteMap(tRenderer, tWindow);
				}
				//	Reset
				else if (LOWORD(event.syswm.msg->msg.win.wParam) == 7) {
					resetGameboy();
				}
				//	Memory Map
				else if (LOWORD(event.syswm.msg->msg.win.wParam) == 8) {
					showMemoryMap();
				}
				//	Load ROM
				else if (LOWORD(event.syswm.msg->msg.win.wParam) == 9) {
					printf("loading ROM");
					char f[100];
					OPENFILENAME ofn;

					ZeroMemory(&f, sizeof(f));
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(ofn);
					ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
					ofn.lpstrFilter = "GameBoy Roms\0*.gb\0";
					ofn.lpstrFile = f;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrTitle = "[ rom selection ]";
					ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

					if (GetOpenFileNameA(&ofn)) {
						filename = f;
						resetGameboy();
					}

				}
				//	Window Map
				else if (LOWORD(event.syswm.msg->msg.win.wParam) == 10) {
					SDL_Window* tWindow;
					SDL_Renderer* tRenderer;
					SDL_CreateWindowAndRenderer(256, 256, 0, &tWindow, &tRenderer);
					SDL_SetWindowSize(tWindow, 512, 512);
					drawWindowMap(tRenderer, tWindow);
				}
			}
			//	close a window
			if (event.syswm.msg->msg.win.msg == WM_CLOSE) {
				DestroyWindow(event.syswm.msg->msg.win.hwnd);
				PostMessage(event.syswm.msg->msg.win.hwnd, WM_CLOSE, 0, 0);
			}
			break;
		case SDL_KEYUP:
		case SDL_KEYDOWN:
			//	reset controlls
			joypad = 0xff;
			joypad |= 0x10;
			//	left key
			if (event.key.keysym.scancode == SDL_SCANCODE_LEFT && event.key.state == SDL_PRESSED)
				joypad ^= 0x91;
			//	right key
			if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT && event.key.state == SDL_PRESSED)
				joypad ^= 0x92;
			//	up key
			if (event.key.keysym.scancode == SDL_SCANCODE_UP && event.key.state == SDL_PRESSED)
				joypad ^= 0x93;
			//	down key
			if (event.key.keysym.scancode == SDL_SCANCODE_DOWN && event.key.state == SDL_PRESSED)
				joypad ^= 0x94;
			//	A key
			if (event.key.keysym.scancode == SDL_SCANCODE_A && event.key.state == SDL_PRESSED)
				joypad ^= 0x01;
			//	B key
			if (event.key.keysym.scancode == SDL_SCANCODE_S && event.key.state == SDL_PRESSED)
				joypad ^= 0x02;
			//	Select key
			if (event.key.keysym.scancode == SDL_SCANCODE_X && event.key.state == SDL_PRESSED)
				joypad ^= 0x04;
			//	Start key
			if (event.key.keysym.scancode == SDL_SCANCODE_Y && event.key.state == SDL_PRESSED)
				joypad ^= 0x08;
			break;
	};
}

void showAbout() {
	SDL_Renderer* renderer;
	SDL_Window* window;
	SDL_Texture* texture;

	//	init and create window and renderer
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(500, 450, 0, &window, &renderer);
	SDL_SetWindowSize(window, 500, 450);
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	SDL_SetWindowTitle(window, "[ q00.gb ][ about ]");

	HWND hwnd = wmInfo.info.win.window;
	HINSTANCE hInst = wmInfo.info.win.hinstance;
	HWND hScroll = CreateWindow("EDIT", NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_MULTILINE | ES_READONLY, 10, 70, 480, 370, hwnd, NULL, hInst, NULL);
	string s = 
		
	"##(((((((((((((((((((((((((((((((((((((((((((((((#%((((((((\r\n"
	"(((((((((((((((((((((((((((((((((((((((((((((((((#%((((((((\r\n"
	"(((((((%#(((((((((((((((((((/((((((((((((((((/*,/#(((((##%&\r\n"
	"((((((#(##((((((((((((((* .  ..,,((((((((((/,,,*/#(((#&@@@@\r\n"
	"(((((((*..*//((((((*,..             *%%%&@#(/(#(#@@@@@@@&&&\r\n"
	"((((((((. .,/*(#(,         ...    ,%%%%&&&&&%#%((&@@@@@@@@@\r\n"
	"@@@@%,*,***/**/*,,,,**       .,,*,,,*,*((**,***(@@@@@@@@@@@\r\n"
	"@@@@(***&@&@@&@@@@%%,,..*/,/@@@@@@@@@@%&@***/&@@@@@@@@@@@@@\r\n"
	"%%&%@(,(@@&@@@@@@@@@@@/*////*@@@@@@@@@@@@@&%*(@@@@@@@@@@@@@\r\n"
	"@@@@@&%*@@@@@@@@@@@@@@/#,   *%@@@@@@@@@@@@@*%@@@@@@@@@@@@@@\r\n"
	"@@@@@@@&*#@@@@@@@@@@&/..,,,,.,#@@@@@@@@@@%/%@@@@@@@@@@@@@@&\r\n"
	"@@@@@&@@@@(/&@@@@@(/*,...    ..**&@@@@@//&@@@@@@@@@###(((((\r\n"
	"@@@@@@@@%#&(/*,.,.         ...,..,**/&@@@@@@@@@#(((((((((((\r\n"
	"###%%%%###  ,///////(/*((((((#######, .@@@(((((((((((((((((\r\n"
	"((((((((((/*((((((((/*(((*(//((/##((###..*&@@((((((((((((((\r\n"
	"(((((((((( //,,/((#((((#(((((###*#%%/,. *%&((((((((((((((((\r\n"
	"(((((((((/  ###*,##(#(###((###%,(%%%%#*.,  (&((((((((((((((\r\n"
	"(((((((((,  ./%%//(##########%%,%%%%@*..  #((((((((((((((((\r\n"
	"(((((((((  ..(&&%.######%###%%%,/%%&&(/,.    ((((((((((((((\r\n"
	"((((((((.  .,,/(,*##%#%#%####%%##*/((, .      /((((((((((((\r\n"
	"(((((((.    ((/,,(##%%%%#%##%%%%##/***/.     .#((((((((((((\r\n"
	"(((((((     ,*,/(#%%%%&%%@#%%&%%%%%##((,     *%#(((((((((((\r\n"
	"(((((((     .*(##%%&%&&&%%%%&%%&&&%%%#*.     ,#%(((((((((((\r\n"
	"(((((((      .*##%%&%%&&&&%&%%&&&%&%%/......*%&@(((((((((((\r\n"
	"(((((((*     ..(%&&&&&&&&&@&@&&%%#%%%%(%&@&@@@(((((((((((((\r\n"
	"((((((((#%%##(*,**/*(/***/***,,,,,,,*#%&&&@@@@,./((((((((((\r\n"
	"((((((((((((//*     ...,,**///,,*,,,(%@@@@@@@@@&..(@**,/(((\r\n"
	"(((((((((((//((*.   .,*/**/(##(##&%&%#@@@@@@@@@@@,,(@%**/((\r\n"
	"(((((((((((((/((*//*..,,,,/(&%&@@@&@@@@&@%@@@@@@@@#,(@@@%((\r\n"
		
		
		;

	const TCHAR* text = s.c_str();
	HDC wdc = GetWindowDC(hScroll);
	HFONT font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	LOGFONT lf;
	GetObject(font, sizeof(LOGFONT), &lf);
	lf.lfHeight = 10;
	HFONT boldFont = CreateFontIndirect(&lf);
	SendMessage(hScroll, WM_SETFONT, (WPARAM)boldFont, 60);
	SendMessage(hScroll, WM_SETTEXT, 60, reinterpret_cast<LPARAM>(text));


	HWND hAb = CreateWindow("EDIT", NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_MULTILINE | ES_READONLY, 10, 10, 480, 50, hwnd, NULL, hInst, NULL);
	s = "(c) LilaQ - 2019 \r\n\r\ncontact : horsepenis @ gmail.com - no spam kthx";

	text = s.c_str();
	wdc = GetWindowDC(hAb);
	font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	lf;
	GetObject(font, sizeof(LOGFONT), &lf);
	lf.lfHeight = 10;
	boldFont = CreateFontIndirect(&lf);
	SendMessage(hAb, WM_SETFONT, (WPARAM)boldFont, 60);
	SendMessage(hAb, WM_SETTEXT, 60, reinterpret_cast<LPARAM>(text));
}

void showMemoryMap() {

	SDL_Renderer* renderer;
	SDL_Window* window;
	SDL_Texture* texture;

	//	init and create window and renderer
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(550, 470, 0, &window, &renderer);
	SDL_SetWindowSize(window, 550, 470);
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);
	SDL_SetWindowTitle(window, "[ q00.gb ][ debugger ]");

	HWND hwnd = wmInfo.info.win.window;
	HINSTANCE hInst = wmInfo.info.win.hinstance;
	HWND hScroll = CreateWindow("EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_AUTOVSCROLL | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_MULTILINE | ES_READONLY, 10, 80, 530, 380, hwnd, NULL, hInst, NULL);

	//	MEMDUMP Control
	string s = "Offset      00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f\r\n\r\n";
	for (int i = 0; i < 0x10000; i+=0x10) {
		char title[70];
		snprintf(title, sizeof title, "0x%04x      %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \r\n", i,
			readFromMem(i),
			readFromMem(i + 1),
			readFromMem(i + 2),
			readFromMem(i + 3),
			readFromMem(i + 4),
			readFromMem(i + 5),
			readFromMem(i + 6),
			readFromMem(i + 7),
			readFromMem(i + 8),
			readFromMem(i + 9),
			readFromMem(i + 10),
			readFromMem(i + 11),
			readFromMem(i + 12),
			readFromMem(i + 13),
			readFromMem(i + 14),
			readFromMem(i + 15)
		);
		s.append((string)title);
	}

	const TCHAR* text = s.c_str();
	HDC wdc = GetWindowDC(hScroll);
	HFONT font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	LOGFONT lf;
	GetObject(font, sizeof(LOGFONT), &lf);
	lf.lfWeight = FW_LIGHT;
	HFONT boldFont = CreateFontIndirect(&lf);
	SendMessage(hScroll, WM_SETFONT, (WPARAM)boldFont, 60);
	SendMessage(hScroll, WM_SETTEXT, 60, reinterpret_cast<LPARAM>(text));

	//	FLAGS Control
	HWND hFlags = CreateWindow("EDIT", NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_MULTILINE | ES_READONLY, 10, 10, 40, 60, hwnd, NULL, hInst, NULL);
	s = "";
	char title[70];
	snprintf(title, sizeof title, "Z: %d\r\nN: %d\r\nH: %d\r\nC: %d\r\n", flags.Z, flags.N, flags.H, flags.C);
	s.append((string)title);
	text = s.c_str();
	wdc = GetWindowDC(hFlags);
	font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	SendMessage(hFlags, WM_SETFONT, (WPARAM)font, 60);
	SendMessage(hFlags, WM_SETTEXT, 60, reinterpret_cast<LPARAM>(text));

	//	REGISTERS Control
	HWND hRegs = CreateWindow("EDIT", NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_MULTILINE | ES_READONLY, 60, 10, 70, 60, hwnd, NULL, hInst, NULL);
	s = "";
	title[70];
	snprintf(title, sizeof title, "AF: %04x\r\nBC: %04x\r\nDE: %04x\r\nHL: %04x", (registers.A << 8) | ((flags.Z << 3) | (flags.N << 2) | (flags.H << 1) | (flags.C)), (registers.B << 8) | registers.C, (registers.D << 8) | registers.E, (registers.H << 8) | registers.L);
	s.append((string)title);
	text = s.c_str();
	wdc = GetWindowDC(hRegs);
	font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	SendMessage(hRegs, WM_SETFONT, (WPARAM)font, 60);
	SendMessage(hRegs, WM_SETTEXT, 60, reinterpret_cast<LPARAM>(text));

	//	TIMER Control
	HWND hAdrs = CreateWindow("EDIT", NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_MULTILINE | ES_READONLY, 140, 10, 150, 60, hwnd, NULL, hInst, NULL);
	s = "";
	title[70];
	snprintf(title, sizeof title, "IME        :    %d\r\n[0xFFFF] IE: 0x%02x\r\n[0xFF0F] IF: 0x%02x", interrupts_enabled, readFromMem(0xffff), readFromMem(0xff0f));
	s.append((string)title);
	text = s.c_str();
	wdc = GetWindowDC(hAdrs);
	font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	SendMessage(hAdrs, WM_SETFONT, (WPARAM)font, 60);
	SendMessage(hAdrs, WM_SETTEXT, 60, reinterpret_cast<LPARAM>(text));

	//	ADRESSES Control
	HWND hTts = CreateWindow("EDIT", NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_MULTILINE | ES_READONLY, 300, 10, 160, 60, hwnd, NULL, hInst, NULL);
	s = "";
	title[70];
	snprintf(title, sizeof title, "[0xFF40] LCDC: 0x%02x\r\n[0xFF41] STAT: 0x%02x\r\n[0xFF44] LY:   0x%02x", readFromMem(0xff40), readFromMem(0xff41), readFromMem(0xff44));
	s.append((string)title);
	text = s.c_str();
	wdc = GetWindowDC(hTts);
	font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	SendMessage(hTts, WM_SETFONT, (WPARAM)font, 60);
	SendMessage(hTts, WM_SETTEXT, 60, reinterpret_cast<LPARAM>(text));
	
	//	SP / PC Control
	HWND hPcSp = CreateWindow("EDIT", NULL, WS_VISIBLE | WS_CHILD | ES_LEFT | WS_BORDER | ES_MULTILINE | ES_READONLY | ES_MULTILINE | ES_READONLY, 470, 10, 70, 60, hwnd, NULL, hInst, NULL);
	s = "";
	title[70];
	snprintf(title, sizeof title, "SP: %04x\r\nPC: %04x", sp, pc);
	s.append((string)title);
	text = s.c_str();
	wdc = GetWindowDC(hPcSp);
	font = (HFONT)GetStockObject(ANSI_FIXED_FONT);
	SendMessage(hPcSp, WM_SETFONT, (WPARAM)font, 60);
	SendMessage(hPcSp, WM_SETTEXT, 60, reinterpret_cast<LPARAM>(text));
}