#define _CRT_SECURE_NO_DEPRECATE
#include "SDL2/include/SDL.h"
#include <iostream>
#ifdef _WIN32
	#include <Windows.h>
	#include <WinUser.h>
	#include "SDL2/include/SDL_syswm.h"
#endif // _WIN32

//	init PPU
SDL_Renderer* renderer;
SDL_Window* window;
SDL_Texture* texture;

unsigned char framebuffer[160 * 144 * 3];	//	3 bytes per pixel, RGB24
unsigned char bgmap[256 * 256 * 3];	//	3 bytes per pixel, RGB24

int tilemap;
int tiledata;
int tilenr, colorval, colorfrompal;

void initPPU() {
	//	init and create window and renderer
	SDL_Init(SDL_INIT_VIDEO);
	SDL_CreateWindowAndRenderer(160, 144, 0, &window, &renderer);
	SDL_SetWindowSize(window, 480, 432);
	SDL_SetWindowResizable(window, SDL_TRUE);

	//	for fast rendering, create a texture
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 160, 144);
}

SDL_Window* getWindow() {
	return window;
}

void createBGMap(unsigned char memory[]) {
	tilemap = (((memory[0xff40] >> 3) & 1) == 1) ? 0x9c00 : 0x9800;	//	check which location was set in LCDC for BG Map
	tiledata = (((memory[0xff40] >> 4) & 1) == 1) ? 0x8000 : 0x8800;	//	check which location was set in LCDC for BG / Window Tile Data (Catalog)
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 256; j++) {
			//	which tile no. is wanted from tiledata
			tilenr = memory[tilemap + ((i / 8 * 32) + (j / 8))];
			//	get color value for the current pixel (00, 01, 10, 11)
			colorval = (memory[tiledata + (tilenr * 0x10) + (i % 8 * 2)] >> (7 - (j % 8)) & 0x1) + (memory[tiledata + (tilenr * 0x10) + (i % 8 * 2) + 1] >> (7 - (j % 8)) & 0x1) * 2;
			//	if 0x8800 we adress as signed tilenr from 0x9000 being tile 0 (overwrite the original value)
			if (tiledata == 0x8800) {
				colorval = (memory[tiledata + 0x800 + ((int8_t)tilenr * 0x10) + (i % 8 * 2)] >> (7 - (j % 8)) & 0x1) + ((memory[tiledata + 0x800 + ((int8_t)tilenr * 0x10) + (i % 8 * 2) + 1] >> (7 - (j % 8)) & 0x1) * 2);
			}
			//	get real color from palette
			colorfrompal = (memory[0xff47] >> (2 * colorval)) & 0xff;
			bgmap[(i * 256 * 3) + (j * 3)] = colorfrompal;
			bgmap[(i * 256 * 3) + (j * 3) + 1] = colorfrompal;
			bgmap[(i * 256 * 3) + (j * 3) + 2] = colorfrompal;
		}
	}
}

void drawBGMap(unsigned char memory[], SDL_Renderer* tRenderer, SDL_Window *tWindow) {

	//	setup PPU
	SDL_SetRenderDrawColor(tRenderer, 159, 201, 143, 0);
	SDL_RenderClear(tRenderer);
	SDL_Texture* tTexture;
	tTexture = SDL_CreateTexture(tRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, 256, 256);

	//	fill BGMap
	createBGMap(memory);

	//	window decorations
	char title[50];
	snprintf(title, sizeof title, "[ bg map ][ tilemap: 0x%04x ][ tiledata: 0x%04x ][ LCDC: 0x%02x ]", tilemap, tiledata, memory[0xff40]);
	SDL_SetWindowTitle(tWindow, title);

	//	draw texture to renderer
	SDL_UpdateTexture(tTexture, NULL, bgmap, 256 * sizeof(unsigned char) * 3);
	SDL_RenderCopy(tRenderer, tTexture, NULL, NULL);
	SDL_RenderPresent(tRenderer);
}


void drawLine(unsigned char memory[]) {
	//	clear the renderer
	SDL_RenderClear(renderer);

	//	refresh BG map
	if(memory[0xff44] <= 1)
		createBGMap(memory);

	//	print by line, so image effects are possible
	int row = memory[0xff44];
	for (int col = 0; col < 160; col++) {
		framebuffer[(row * 160 * 3) + (col * 3)] = bgmap[((memory[0xff42] + row) * 256 * 3) + ((memory[0xff43] + col) * 3)];
		framebuffer[(row * 160 * 3) + (col * 3) + 1 ] = bgmap[((memory[0xff42] + row) * 256 * 3) + ((memory[0xff43] + col) * 3) + 1];
		framebuffer[(row * 160 * 3) + (col * 3) + 2 ] = bgmap[((memory[0xff42] + row) * 256 * 3) + ((memory[0xff43] + col) * 3) + 2];
	}

	//	draw if v-blank
	if (memory[0xff44] == 0x8F) {
		SDL_UpdateTexture(texture, NULL, framebuffer, 160 * sizeof(unsigned char) * 3);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

}

void drawBGTileset(unsigned char memory[], SDL_Renderer* tRenderer, SDL_Window* tWindow) {

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
			line[0] = memory[i + j] & 0x1 + (memory[i + j + 1] & 0x1) * 2;
			line[1] = (memory[i + j] >> 1) & 0x1 + ((memory[i + j + 1] >> 1) & 0x1) * 2;
			line[2] = (memory[i + j] >> 2) & 0x1 + ((memory[i + j + 1] >> 2) & 0x1) * 2;
			line[3] = (memory[i + j] >> 3) & 0x1 + ((memory[i + j + 1] >> 3) & 0x1) * 2;
			line[4] = (memory[i + j] >> 4) & 0x1 + ((memory[i + j + 1] >> 4) & 0x1) * 2;
			line[5] = (memory[i + j] >> 5) & 0x1 + ((memory[i + j + 1] >> 5) & 0x1) * 2;
			line[6] = (memory[i + j] >> 6) & 0x1 + ((memory[i + j + 1] >> 6) & 0x1) * 2;
			line[7] = (memory[i + j] >> 7) & 0x1 + ((memory[i + j + 1] >> 7) & 0x1) * 2;
			for (int b = 0; b < 8; b++) {
				//	0xff47 is the adress of the BG / Window palette
				int x = (7 - b) + 8 * ((i - 0x8000) % 256) / 16;
				int y = 8 * ((i - 0x8000) / 256) + (j / 2);
				tFramebuffer[(y * 128 * 3) + (x * 3)] = (memory[0xff47] >> (2 * line[b]) & 0xff);
				tFramebuffer[(y * 128 * 3) + (x * 3) + 1] = (memory[0xff47] >> (2 * line[b]) & 0xff);
				tFramebuffer[(y * 128 * 3) + (x * 3) + 2] = (memory[0xff47] >> (2 * line[b]) & 0xff);
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