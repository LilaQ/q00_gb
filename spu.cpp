#include <iostream>
#include <cstdint>
#include <vector>
#include "SDL2/include/SDL.h"
#include "mmu.h"
#define internal static

//	SC1 - Tone and Sweep
//	SC2 - Tone
//	SC3 - Wave Output
//	SC4 - Noise

int SamplesPerSecond = 44100;			//	resolution
int16_t Amplitude = 1000;				//	volume
bool SoundIsPlaying = false;

int cycle_count = 0;

std::vector<uint16_t> SC1buf;
std::vector<uint16_t> SC2buf;
uint32_t SC1timer = 0x00;
uint32_t SC2timer = 0x00;
uint8_t SC1dutyIndex = 0;
uint8_t SC2dutyIndex = 0;
uint8_t SC1pcc = 95;

uint8_t duties[4][8] = {
	{0,0,0,0,0,0,0,1},			//	00 (0x0)
	{1,0,0,0,0,0,0,1},			//	01 (0x1)
	{1,0,0,0,0,1,1,1},			//	10 (0x2)
	{0,1,1,1,1,1,1,0}			//	11 (0x3)
};


internal void SDLInitAudio(int32_t SamplesPerSecond, int32_t BufferSize)
{
	SDL_AudioSpec AudioSettings = { 0 };

	AudioSettings.freq = SamplesPerSecond;
	AudioSettings.format = AUDIO_S16LSB;		//	One of the modes that doesn't produce a high frequent pitched tone when having silence
	AudioSettings.channels = 2;
	//AudioSettings.samples = BufferSize;

	SDL_OpenAudio(&AudioSettings, 0);

	/*if (AudioSettings.format != AUDIO_S16LSB)
	{
		printf("Oops! We didn't get AUDIO_S16LSB as our sample format!\n");
		SDL_CloseAudio();
	}*/


	//	SWEEP UP
	/*
	FF10 - F7
	FF11 - BF
	FF12 - F0
	FF13 - 00
	FF14 - BE
	*/
	/*writeToMem(0xff10, 0xf7);
	writeToMem(0xff11, 0xbf);
	writeToMem(0xff12, 0xf0);
	writeToMem(0xff13, 0x00);
	writeToMem(0xff14, 0xbe);*/

}

void stepSC1(uint8_t c) {

	while (c--) {
		//	reset frequency (more of a timer) if it hits zero to : (2048 - freq) * 4
		if (SC1timer <= 0x00) {
			uint16_t r = (((readFromMem(0xff14) & 7) << 8) | readFromMem(0xff13));
			SC1timer = (2048 - r) * 4;

			//	tick duty pointer
			++SC1dutyIndex %= 8;
		}
		else
			SC1timer--;

		if (!--SC1pcc) {
			SC1pcc = 95;
			int duty = readFromMem(0xff11) >> 6;
			SC1buf.push_back((duties[duty][SC1dutyIndex] == 1) ? Amplitude : 0);
			SC1buf.push_back((duties[duty][SC1dutyIndex] == 1) ? Amplitude : 0);
		}
	}

}

void stepSC2(uint8_t c) {

	//	reset frequency (more of a timer) if it hits zero to : (2048 - freq) * 4
	if (SC2timer <= 0x00) {
		SC2timer = (2048 - (((readFromMem(0xff24) & 7) << 8) | readFromMem(0xff23))) * 4;
			
		//	tick duty pointer
		++SC2dutyIndex %= 8;
	}
	else
		SC2timer-=c;

	int duty = readFromMem(0xff21) >> 6;
	SC2buf.push_back((duties[duty][SC2dutyIndex]) ? Amplitude : -Amplitude);

}

void stepSPU(unsigned char cycles) {

	cycle_count += cycles;
	//if (cycle_count >= 95) 
	{
		cycle_count -= 95;

		stepSC1(cycles);
		//stepSC2(cycles);

		if (SC1buf.size() >= 735*2*2) {
			//	send audio data to device
			
			SDL_QueueAudio(1, SC1buf.data(), SC1buf.size() * 2);

			SC1buf.clear();
			//SC2buf.clear();

			//	SDL initially pauses. So, unpause to start playing
			if (!SoundIsPlaying)
			{
				SDL_PauseAudio(0);
				SoundIsPlaying = true;
			}
			
			//while (SDL_GetQueuedAudioSize(1) > 16655) {
			//	//printf("%d\n", aaaa);
			//	SDL_Delay(1);
			//}
		}
	}
}

void initSPU() {
	SDL_setenv("SDL_AUDIODRIVER", "directsound", 1);
	SDL_Init(SDL_INIT_AUDIO);

	// Open our audio device; Sample Rate will dictate the pace of our synthesizer
	SDLInitAudio(44100, 735*2*2);

}

