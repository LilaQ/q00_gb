#define _CRT_SECURE_NO_DEPRECATE
#include "SDL2/include/SDL.h"
#include <iostream>
#include "mmu.h"
#ifdef _WIN32
	#include <Windows.h>
	#include <WinUser.h>
	#include "SDL2/include/SDL_syswm.h"
#endif // _WIN32

//	init PPU
SDL_Renderer* renderer;
SDL_Window* window;
SDL_Texture* texture;

const int FB_SIZE = (256 * 256 * 3);
unsigned char framebuffer[160 * 144 * 3];	//	3 bytes per pixel, RGB24
unsigned char bgmap[FB_SIZE];				//	3 bytes per pixel, RGB24
unsigned char winmap[FB_SIZE];				//	3 bytes per pixel, RGB24
unsigned char spritemap[FB_SIZE];			//	3 bytes per pixel, RGB24

uint8_t SCY, SCX, STAT, LY, LYC;

int tilemap;
int tiledata;
int tilenr, colorval, colorfrompal;
int row, xoff, yoff, xoffS, yoffS;

int COLORS[] = {
		0xff,
		0xaa,
		0x55,
		0x00
};

void initPPU() {
	//	init and create window and renderer
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(160, 144, 0, &window, &renderer);
	SDL_SetWindowSize(window, 480, 432);
	SDL_SetWindowResizable(window, SDL_TRUE);

	//	for fast rendering, create a texture
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 160, 144);
	memset(bgmap, 0, FB_SIZE);
	memset(spritemap, 0x69, FB_SIZE);
	memset(winmap, 0, FB_SIZE);
}

SDL_Window* getWindow() {
	return window;
}

void createBGMap() {
	tilemap = (((readFromMem(0xff40) >> 3) & 1) == 1) ? 0x9c00 : 0x9800;		//	check which location was set in LCDC for BG Map
	tiledata = (((readFromMem(0xff40) >> 4) & 1) == 1) ? 0x8000 : 0x8800;	//	check which location was set in LCDC for BG / Window Tile Data (Catalog)
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			//	which tile no. is wanted from tiledata
			tilenr = readFromMem(tilemap + ((i / 8 * 32) + (j / 8)));
			//	get color value for the current pixel (00, 01, 10, 11)
			colorval = (readFromMem(tiledata + (tilenr * 0x10) + (i % 8 * 2)) >> (7 - (j % 8)) & 0x1) + (readFromMem(tiledata + (tilenr * 0x10) + (i % 8 * 2) + 1) >> (7 - (j % 8)) & 0x1) * 2;
			//	if 0x8800 we adress as signed tilenr from 0x9000 being tile 0 (overwrite the original value)
			if (tiledata == 0x8800) {
				colorval = (readFromMem(tiledata + 0x800 + ((int8_t)tilenr * 0x10) + (i % 8 * 2)) >> (7 - (j % 8)) & 0x1) + ((readFromMem(tiledata + 0x800 + ((int8_t)tilenr * 0x10) + (i % 8 * 2) + 1) >> (7 - (j % 8)) & 0x1) * 2);
			}
			//	get real color from palette
			colorfrompal = ( readFromMem(0xff47) >> (2 * colorval) ) & 3;
			bgmap[(i * 256 * 3) + (j * 3)] = COLORS[colorfrompal];
			bgmap[(i * 256 * 3) + (j * 3) + 1] = COLORS[colorfrompal];
			bgmap[(i * 256 * 3) + (j * 3) + 2] = COLORS[colorfrompal];
		}
	}
}

void createWindow() {
	/*
	LCDC Register = 0xFF40
	Bit 6 - Window Tile Map Display Select
		0: $9800-$9BFF
		1: $9C00-$9FFF
	*/
	tilemap = (((readFromMem(0xff40) >> 5) & 1) == 1) ? 0x9c00 : 0x9800;	//	check which location was set in LCDC for BG Map
	tiledata = (((readFromMem(0xff40) >> 4) & 1) == 1) ? 0x8000 : 0x8800;	//	check which location was set in LCDC for BG / Window Tile Data (Catalog)
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			//	which tile no. is wanted from tiledata
			tilenr = readFromMem(tilemap + ((i / 8 * 32) + (j / 8)));
			//	get color value for the current pixel (00, 01, 10, 11)
			colorval = (readFromMem(tiledata + (tilenr * 0x10) + (i % 8 * 2)) >> (7 - (j % 8)) & 0x1) + (readFromMem(tiledata + (tilenr * 0x10) + (i % 8 * 2) + 1) >> (7 - (j % 8)) & 0x1) * 2;
			//	if 0x8800 we adress as signed tilenr from 0x9000 being tile 0 (overwrite the original value)
			if (tiledata == 0x8800) {
				colorval = (readFromMem(tiledata + 0x800 + ((int8_t)tilenr * 0x10) + (i % 8 * 2)) >> (7 - (j % 8)) & 0x1) + ((readFromMem(tiledata + 0x800 + ((int8_t)tilenr * 0x10) + (i % 8 * 2) + 1) >> (7 - (j % 8)) & 0x1) * 2);
			}
			//	get real color from palette
			colorfrompal = (readFromMem(0xff47) >> (2 * colorval)) & 3;
			winmap[(i * 256 * 3) + (j * 3)] = COLORS[colorfrompal];
			winmap[(i * 256 * 3) + (j * 3) + 1] = COLORS[colorfrompal];
			winmap[(i * 256 * 3) + (j * 3) + 2] = COLORS[colorfrompal];
		}
	}
}

void createSpriteMap() {

	//	clear array
	memset(spritemap, 0x69, sizeof(spritemap));

	uint16_t pat = 0x8000;	//	sprite pattern table
	uint16_t oam = 0xfe00;	//	oam ( sprite attribute table ); 0xfe00 - 0xfe9f; divided into 40 blocks, 4 bytes each
	uint16_t oam_end = 0xfe9f;

	//	iterate through every object / sprite
	for (uint16_t i = oam; i <= oam_end; i += 4) {

		//	we assume for now, the origin of the sprite is in the lower right corner
		uint8_t y = readFromMem(i) - 16;
		uint8_t x = readFromMem(i + 1) - 8;
		uint8_t tilenr = readFromMem(i + 2);
		uint8_t flags = readFromMem(i + 3);
		uint8_t flip = (flags >> 5) & 0x01 | (((flags >> 6) & 0x01) << 1);
		uint8_t height = ((readFromMem(0xff40) >> 2) & 0x01) ? 16 : 8;
		for(int u = 0; u < height; u++)
			for (int v = 0; v < 8; v++) {
				switch (flags & 0x60)	//	mask flagging bits
				{
				case 0x00:	//	no flip
					colorval = (readFromMem(pat + (tilenr * 0x10) + (u * 2)) >> (7 - v) & 0x1) + (readFromMem(pat + (tilenr * 0x10) + (u * 2) + 1) >> (7 - v) & 0x1) * 2;
					break;
				case 0x20:	//	only x-flip
					colorval = (readFromMem(pat + (tilenr * 0x10) + (u * 2)) >> v & 0x1) + (readFromMem(pat + (tilenr * 0x10) + (u * 2) + 1) >> v & 0x1) * 2;
					break;
					//	TODO
				case 0x40:	//	only y-flip
					colorval = (readFromMem(pat + (tilenr * 0x10) + ((height - u) * 2) ) >> (7 - v) & 0x1) + (readFromMem(pat + (tilenr * 0x10) + ((height - u) * 2) + 1) >> (7 - v) & 0x1) * 2;
					break;
					//	TODO
				case 0x60:	//	x-flip and y-flip
					colorval = (readFromMem(pat + (tilenr * 0x10) + ((height - u) * 2) ) >> v & 0x1) + (readFromMem(pat + (tilenr * 0x10) + ((height - u) * 2) + 1) >> v & 0x1) * 2;
					break;
				default:
					break;
				}
				

				//	get real color from palette
				uint16_t pal = ((flags >> 4) & 1) ? 0xff49 : 0xff48;
				colorfrompal = (readFromMem(pal) >> (2 * colorval)) & 3;

				//	if not zero, set paint ( also, make sure we stay in the boundaries of our array)
				if (colorval != 0 && ((y+u) <= 0xff) && ((x+v) <= 0xff)) {
					spritemap[((y + u) * 256 * 3) + ((x + v) * 3)] = COLORS[colorfrompal];
					spritemap[((y + u) * 256 * 3) + ((x + v) * 3) + 1] = COLORS[colorfrompal];
					spritemap[((y + u) * 256 * 3) + ((x + v) * 3) + 2] = COLORS[colorfrompal];
				}
			}
	}
}

void drawSpriteMap(SDL_Renderer* tRenderer, SDL_Window* tWindow) {
	//	setup PPU
	SDL_Texture* tTexture;
	tTexture = SDL_CreateTexture(tRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 256);

	//	fill sprite map
	createSpriteMap();

	//	window decorations
	char title[50];
	snprintf(title, sizeof title, "[ sprite map ][ wx: 0x%02x ][ wy: 0x%02x ][ win_on: 0x%02x ]", 0, 0, 0);
	SDL_SetWindowTitle(tWindow, title);

	//	draw texture to renderer
	SDL_UpdateTexture(tTexture, NULL, spritemap, 256 * sizeof(unsigned char) * 3);
	SDL_RenderCopy(tRenderer, tTexture, NULL, NULL);
	SDL_RenderPresent(tRenderer);
}

void drawBGMap(SDL_Renderer* tRenderer, SDL_Window *tWindow) {

	//	setup PPU
	SDL_Texture* tTexture;
	tTexture = SDL_CreateTexture(tRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 256);

	//	fill BGMap
	createBGMap();

	//	window decorations
	char title[50];
	snprintf(title, sizeof title, "[ bg map ][ tilemap: 0x%04x ][ tiledata: 0x%04x ][ LCDC: 0x%02x ]", tilemap, tiledata, readFromMem(0xff40));
	SDL_SetWindowTitle(tWindow, title);

	//	draw texture to renderer
	SDL_UpdateTexture(tTexture, NULL, bgmap, 256 * sizeof(unsigned char) * 3);
	SDL_RenderCopy(tRenderer, tTexture, NULL, NULL);
	SDL_RenderPresent(tRenderer);
}


void drawLine() {
	//	clear the renderer
	SDL_RenderClear(renderer);

	//	reset SCY, SCX for correct scrolling values
	SCY = readFromMem(0xff42);
	SCX = readFromMem(0xff43);

	//	reset STAT
	STAT = 0x00;

	//	reset LY / LYC
	LY = readFromMem(0xff44);
	LYC = readFromMem(0xff45);

	//	refresh OAM map (sprites)
	if (readFromMem(0xff44) == 0)
		createSpriteMap();

	//	print by line, so image effects are possible
	row = readFromMem(0xff44);
	for (int col = 0; col < 160; col++) {
		yoff = ((SCY + row) * 256 * 3);
		yoffS = (row * 256 * 3);
		xoff = ((SCX + col) * 3);
		if (xoff >= 768)	//	X-Wrapping; 768 = 256 * 3, saving calcs
			xoff -= 768;
		xoffS = (col * 3);
		framebuffer[(row * 160 * 3) + (col * 3)] = bgmap[yoff + xoff];
		framebuffer[(row * 160 * 3) + (col * 3) + 1 ] = bgmap[yoff + xoff + 1];
		framebuffer[(row * 160 * 3) + (col * 3) + 2 ] = bgmap[yoff + xoff + 2];

		//	overwrite with sprites, if available
		if (spritemap[yoffS + xoffS] != 0x69) {	//	if the first RGB value is 'transparent', we assume the whole pixel is transparent
			framebuffer[(row * 160 * 3) + (col * 3)] = spritemap[yoffS + xoffS];
			framebuffer[(row * 160 * 3) + (col * 3) + 1] = spritemap[yoffS + xoffS + 1];
			framebuffer[(row * 160 * 3) + (col * 3) + 2] = spritemap[yoffS + xoffS + 2];
		}
	}

	//	refresh BG map
	if (readFromMem(0xff44) == 0)
		createBGMap();

	//	draw if v-blank
	if (readFromMem(0xff44) == 0x8F) {
		SDL_UpdateTexture(texture, NULL, framebuffer, 160 * sizeof(unsigned char) * 3);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
		STAT |= 0x01;	//	set v-blank bit
	}

	//	set LY==LYC coincidence flag
	STAT |= ((LY == LYC) << 2);

	//	write STAT
	writeToMem(0xff41, 0xc8 | STAT);

}

void drawBGTileset(SDL_Renderer* tRenderer, SDL_Window* tWindow) {

	//	setup PPU
	SDL_SetRenderDrawColor(tRenderer, 159, 201, 143, 0);
	SDL_RenderClear(tRenderer);
	SDL_SetRenderDrawColor(tRenderer, 255, 0, 0, 255);
	SDL_Texture* tTexture;
	unsigned char tFramebuffer[128 * 256 * 3];	//	3 bytes per pixel, RGB24
	tTexture = SDL_CreateTexture(tRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 128, 256);

	//	draw tileset
	for (int i = 0x8000; i < 0x9fff; i += 16)
	{
		for (int j = 0; j < 16; j += 2)
		{
			int line[8];
			line[0] = readFromMem(i + j) & 0x1 + (readFromMem(i + j + 1) & 0x1) * 2;
			line[1] = (readFromMem(i + j) >> 1) & 0x1 + ((readFromMem(i + j + 1) >> 1) & 0x1) * 2;
			line[2] = (readFromMem(i + j) >> 2) & 0x1 + ((readFromMem(i + j + 1) >> 2) & 0x1) * 2;
			line[3] = (readFromMem(i + j) >> 3) & 0x1 + ((readFromMem(i + j + 1) >> 3) & 0x1) * 2;
			line[4] = (readFromMem(i + j) >> 4) & 0x1 + ((readFromMem(i + j + 1) >> 4) & 0x1) * 2;
			line[5] = (readFromMem(i + j) >> 5) & 0x1 + ((readFromMem(i + j + 1) >> 5) & 0x1) * 2;
			line[6] = (readFromMem(i + j) >> 6) & 0x1 + ((readFromMem(i + j + 1) >> 6) & 0x1) * 2;
			line[7] = (readFromMem(i + j) >> 7) & 0x1 + ((readFromMem(i + j + 1) >> 7) & 0x1) * 2;
			for (int b = 0; b < 8; b++) {
				//	0xff47 is the adress of the BG / Window palette
				int x = (7 - b) + 8 * ((i - 0x8000) % 256) / 16;
				int y = 8 * ((i - 0x8000) / 256) + (j / 2);
				colorfrompal = (readFromMem(0xff48) >> (2 * colorval)) & 3;
				tFramebuffer[(y * 128 * 3) + (x * 3)] = COLORS[(readFromMem(0xff47) >> (2 * line[b]) & 3)];
				tFramebuffer[(y * 128 * 3) + (x * 3) + 1] = COLORS[(readFromMem(0xff47) >> (2 * line[b]) & 3)];
				tFramebuffer[(y * 128 * 3) + (x * 3) + 2] = COLORS[(readFromMem(0xff47) >> (2 * line[b]) & 3)];
			}
		}
	}

	SDL_SetWindowTitle(tWindow, "[ tileset ]");

	//	draw texture to renderer
	SDL_UpdateTexture(tTexture, NULL, tFramebuffer, 128 * sizeof(unsigned char) * 3);
	SDL_RenderCopy(tRenderer, tTexture, NULL, NULL);
	SDL_RenderPresent(tRenderer);

}

void stopPPU() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}