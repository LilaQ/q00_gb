#include <iostream>
#include <cstdint>
#include <vector>
#include <deque>
#include "SDL2/include/SDL.h"
#include "mmu.h"
#define internal static

//	SC1 - Tone and Sweep
//	SC2 - Tone
//	SC3 - Wave Output
//	SC4 - Noise

int SamplesPerSecond = 44100;			//	resolution
bool SoundIsPlaying = false;

int cycle_count = 0;

std::vector<float> SC1buf;
std::vector<float> SC2buf;
std::vector<float> SC3buf;
std::vector<float> SC4buf;
std::vector<float> Mixbuf;
uint32_t SC1timer = 0x00;
uint32_t SC2timer = 0x00;
uint32_t SC3timer = 0x00;
uint32_t SC4timer = 0x00;
int16_t SC1amp = 22;
int8_t SC2amp = 0;
int8_t SC3amp = 0;
int8_t SC4amp = 0;
uint8_t SC1dutyIndex = 0;
uint8_t SC2dutyIndex = 0;
uint8_t SC3waveIndex = 0;
uint8_t SC1pcc = 95;
uint8_t SC2pcc = 95;
uint8_t SC3pcc = 95;
uint8_t SC4pcc = 95;
uint16_t SC1pcFS = 0;
uint16_t SC2pcFS = 0;
uint16_t SC3pcFS = 0;
uint16_t SC4pcFS = 0;
uint8_t SC1FrameSeq = 0;
uint8_t SC2FrameSeq = 0;
uint8_t SC3FrameSeq = 0;
uint8_t SC4FrameSeq = 0;
uint8_t SC1len = 0;
uint8_t SC2len = 0;
uint8_t SC3len = 0;
uint8_t SC4len = 0;
bool SC1enabled = false;
bool SC2enabled = false;
bool SC3enabled = false;
bool SC4enabled = false;
bool SC1envelopeEnabled = false;
bool SC2envelopeEnabled = false;
bool SC4envelopeEnabled = false;
bool SC1sweepEnabled = false;
uint8_t SC1sweep = 0;
uint16_t SC1sweepShadow = 0;
uint8_t SC1envelope = 0;
uint8_t SC2envelope = 0;
uint8_t SC4envelope = 0;
uint8_t SC4divisor[8] = { 8, 16, 32, 48, 64, 80, 96, 112 };
uint16_t SC4lfsr = 0;


uint8_t duties[4][8] = {
	{0,0,0,0,0,0,0,1},			//	00 (0x0)
	{1,0,0,0,0,0,0,1},			//	01 (0x1)
	{1,0,0,0,0,1,1,1},			//	10 (0x2)
	{0,1,1,1,1,1,1,0}			//	11 (0x3)
};

void AudioCallback(void* userdata, Uint8* stream, int len) {
	float dst = 0x0;
	float src = 0x0;
	for (int i = 0; i < len * 4; i++) {
		float res = 0;
		if (i < SC1buf.size())
			res += SC1buf.at(i);
		if (i < SC2buf.size())
			res += SC2buf.at(i);
		if (i < SC3buf.size())
			res += SC3buf.at(i);
		/*if (i < SC4buf.size())
			res += SC4buf.at(i);*/
		Mixbuf.push_back(res);
		*stream = Mixbuf.at(i);
	}
	//	send audio data to device; buffer is times 4, because we use floats now, which have 4 bytes per float, and buffer needs to have information of amount of bytes to be used
	//SDL_QueueAudio(1, Mixbuf.data(), Mixbuf.size() * 4);

	//stream = (uint8_t*) Mixbuf.data();

	SC1buf.clear();
	SC2buf.clear();
	SC3buf.clear();
	SC4buf.clear();
	Mixbuf.clear();
}

internal void SDLInitAudio(int32_t SamplesPerSecond, int32_t BufferSize)
{
	SDL_AudioSpec AudioSettings = { 0 };

	AudioSettings.freq = SamplesPerSecond;
	AudioSettings.format = AUDIO_F32SYS;		//	One of the modes that doesn't produce a high frequent pitched tone when having silence
	AudioSettings.channels = 2;
	AudioSettings.samples = BufferSize;
	//AudioSettings.callback = AudioCallback;

	SDL_OpenAudio(&AudioSettings, 0);

}

//	Square1 channel
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

		//	handle frame sequencer, which ticks length, envelope and sweep
		SC1pcFS++;
		if (SC1pcFS == 8192) {
			SC1pcFS = 0;
			++SC1FrameSeq %= 8;

			//	handle sweep (frequency sweep)
			if ((SC1FrameSeq == 2 || SC1FrameSeq == 6) && SC1sweepEnabled) {
				//	tick sweep down
				--SC1sweep;
				if (SC1sweep <= 0) {
					//	reload sweep
					SC1sweep = (readFromMem(0xff10) >> 4) & 7;
					SC1sweepShadow += (SC1sweepShadow >> (readFromMem(0xff10) & 0x3)) * ((readFromMem(0xff11) >> 3) & 0x1) ? -1 : 1;
					if ((SC1sweepShadow <= 2047 && SC1sweepShadow >= 0) && (readFromMem(0xff10) & 0x3)) {
						writeToMem(0xff13, SC1sweepShadow & 0xff);
						writeToMem(0xff14, readFromMem(0xff14) & ((SC1sweepShadow >> 8) & 7));
						SC1sweepShadow += (SC1sweepShadow >> (readFromMem(0xff10) & 0x3)) * ((readFromMem(0xff11) >> 3) & 0x1) ? -1 : 1;
					}
					else {
						SC1enabled = false;
					}
				}
			}

			//	handle length
			if (SC1FrameSeq % 2 == 0 && ((readFromMem(0xff14) >> 6) & 1)) {
				SC1len--;
				if (!SC1len) {
					SC1enabled = false;
					printf("SC1 len expired, disabling channel! \n");
				}
			}

			//	handle envelope (volume envelope)
			if (SC1FrameSeq == 6 && SC1envelopeEnabled) {
				//	tick envelope down
				--SC1envelope;
				//	when the envelope is done ticking to zero, it's time to trigger it's purpose
				if (SC1envelope <= 0) {
					//	reload envelope tick count
					SC1envelope = readFromMem(0xff12) & 7;
					//	calc new frequence (+1 / -1)
					int8_t newamp = SC1amp + (((readFromMem(0xff12) >> 3) & 1) ? 1 : -1);
					//	the new volume needs to be inside 0-15 range
					if (newamp >= 0 && newamp <= 15)
						SC1amp = newamp;
					//	otherwise envelope is disabled
					else
						SC1envelopeEnabled = false;
				}
			}
		}

		if (!--SC1pcc) {
			SC1pcc = 95;
			//	enabled channel
			if (SC1enabled) {
				int duty = readFromMem(0xff11) >> 6;
				SC1buf.push_back((duties[duty][SC1dutyIndex] == 1) ? ((float)SC1amp / 100) : 0);
				SC1buf.push_back((duties[duty][SC1dutyIndex] == 1) ? ((float)SC1amp / 100) : 0);
			}
			//	disabled channel
			else {
				SC1buf.push_back(0);
				SC1buf.push_back(0);
			}
		}
	}

}

//	Square2 channel
void stepSC2(uint8_t c) {

	while (c--) {
		//	reset frequency (more of a timer) if it hits zero to : (2048 - freq) * 4
		if (SC2timer <= 0x00) {
			uint16_t r = (((readFromMem(0xff19) & 7) << 8) | readFromMem(0xff18));
			SC2timer = (2048 - r) * 4;

			//	tick duty pointer
			++SC2dutyIndex %= 8;
		}
		else
			SC2timer--;

		//	handle frame sequencer, which ticks length, envelope and sweep
		SC2pcFS++;
		if (SC2pcFS == 8192) {
			SC2pcFS = 0;
			++SC2FrameSeq %= 8;

			//	handle length
			if (SC2FrameSeq % 2 == 0 && ((readFromMem(0xff19) >> 6) & 1)) {
				SC2len--;
				if (!SC2len) {
					SC2enabled = false;
					printf("SC2 len expired, disabling channel! \n");
				}
			}

			//	handle envelope (volume envelope)
			if (SC2FrameSeq == 6 && SC2envelopeEnabled) {
				//	tick envelope down
				--SC2envelope;
				//	when the envelope is done ticking to zero, it's time to trigger it's purpose
				if (SC2envelope <= 0) {
					//	reload envelope tick count
					SC2envelope = readFromMem(0xff17) & 7;
					//	calc new frequence (+1 / -1)
					int8_t newamp = SC2amp + (((readFromMem(0xff17) >> 3) & 1) ? 1 : -1);
					//	the new volume needs to be inside 0-15 range
					if (newamp >= 0 && newamp <= 15)
						SC2amp = newamp;
					//	otherwise envelope is disabled
					else
						SC2envelopeEnabled = false;
				}
			}

		}

		if (!--SC2pcc) {
			//	enabled channel
			if (SC2enabled) {
				SC2pcc = 95;
				int duty = readFromMem(0xff16) >> 6;
				SC2buf.push_back((duties[duty][SC2dutyIndex] == 1) ? ((float)SC2amp / 100) : 0);
				SC2buf.push_back((duties[duty][SC2dutyIndex] == 1) ? ((float)SC2amp / 100) : 0);
			}
			//	disabled channel
			else {
				SC2buf.push_back(0);
				SC2buf.push_back(0);
			}
		}
	}

}

//	WAVE channel
void stepSC3(uint8_t c) {

	while (c--) {
		//	reset frequency (more of a timer) if it hits zero to : (2048 - freq) * 2
		if (SC3timer <= 0x00) {
			uint16_t r = (((readFromMem(0xff1e) & 7) << 8) | readFromMem(0xff1d));
			SC3timer = (2048 - r) * 2;

			//	tick duty pointer
			++SC3waveIndex %= 32;
		}
		else
			SC3timer--;

		//	handle frame sequencer, which ticks length, envelope and sweep
		SC3pcFS++;
		if (SC3pcFS == 8192) {
			SC3pcFS = 0;
			++SC3FrameSeq %= 8;

			//	handle length
			if (!(SC3FrameSeq % 2) && (readFromMem(0xff1e) & 0x40)) {
				SC3len--;
				if (!SC3len)
					SC3enabled = false;
			}

		}

		if (!--SC3pcc) {
			//	enabled channel
			if (SC3enabled) {
				SC3pcc = 95;
				uint8_t wave = readFromMem(0xff30 + (SC3waveIndex / 2));
				if (SC3waveIndex % 2) {
					wave &= 0xf;
				}
				else {
					wave = wave >> 4;
				}

				//printf("reading 0x%04x , value is %f\n", 0xff30 + (SC3waveIndex / 2), ((float)wave/100));
				int8_t vol = (readFromMem(0xff1c) >> 5) & 0x3;
				if (vol)
					wave = wave >> (vol - 1);
				else
					wave = wave >> 4;

				//	if dac is enabled, output actual wave
				if (readFromMem(0xff1a) >> 7) {
					SC3buf.push_back((float)wave / 100);
					SC3buf.push_back((float)wave / 100);
				}
				else {
					SC3buf.push_back(0);
					SC3buf.push_back(0);
				}
			}
			//	disabled channel
			else {
				SC3buf.push_back(0);
				SC3buf.push_back(0);
			}
		}
	}
}

//	Noise channel
void stepSC4(uint8_t c) {

	while (c--) {
		if (SC4timer <= 0x00) {
			SC4timer = SC4divisor[readFromMem(0xff22) & 0x7] << (readFromMem(0xff22) >> 4);
		}
		else
			SC4timer--;

		//	handle frame sequencer, which ticks length, envelope and sweep
		SC4pcFS++;
		if (SC4pcFS == 8192) {
			SC4pcFS = 0;
			++SC4FrameSeq %= 8;

			//	handle length
			if (SC4FrameSeq % 2 == 0 && ((readFromMem(0xff23) >> 6) & 1)) {
				SC4len--;
				if (!SC4len)
					SC4enabled = false;
			}

			//	handle envelope (volume envelope)
			if (SC4FrameSeq == 7 && SC4envelopeEnabled) {
				//	tick envelope down
				--SC4envelope;
				//	when the envelope is done ticking to zero, it's time to trigger it's purpose
				if (SC4envelope <= 0) {
					//	reload envelope tick count
					SC4envelope = readFromMem(0xff21) & 7;
					//	calc new frequence (+1 / -1)
					int8_t newamp = SC4amp + (((readFromMem(0xff21) >> 3) & 1) ? 1 : -1);
					//	the new volume needs to be inside 0-15 range
					if (newamp >= 0 && newamp <= 15)
						SC4amp = newamp;
					//	otherwise envelope is disabled
					else
						SC4envelopeEnabled = false;
				}
			}

		}

		if (!--SC4pcc) {
			//	enabled channel
			if (SC4enabled) {
				SC4pcc = 95;
				//	handle lfsr
				uint8_t xor_res = (SC4lfsr & 0x1) ^ ((SC4lfsr & 0x2) >> 1);
				SC4lfsr >>= 1;
				SC4lfsr |= (xor_res << 14);
				if ((readFromMem(0xff22) >> 3) & 0x1)
					SC4lfsr |= (xor_res << 6);
				//	push data to buffer
				SC4buf.push_back((SC4lfsr & 0x1) ? 0 : (float) SC4amp / 100);
				SC4buf.push_back((SC4lfsr & 0x1) ? 0 : (float) SC4amp / 100);
			}
			//	disabled channel
			else {
				SC4buf.push_back(0);
				SC4buf.push_back(0);
			}
		}
	}

}

void stepSPU(unsigned char cycles) {

	stepSC1(cycles);
	stepSC2(cycles);
	stepSC3(cycles);
	stepSC4(cycles);

	if (SC1buf.size() >= 4096 || SC2buf.size() >= 4096 || SC3buf.size() >= 4096 || SC4buf.size() >= 4096) {

		float dst = 0x0;
		float src = 0x0;

		for (int i = 0; i < 4096; i++) {
			float res = 0;
			if (i < SC1buf.size())
				res += SC1buf.at(i);
			if(i < SC2buf.size())
				res += SC2buf.at(i);
			if (i < SC3buf.size())
				res += SC3buf.at(i);
			if (i < SC4buf.size())
				res += SC4buf.at(i);
			Mixbuf.push_back(res);
		}
		//	send audio data to device; buffer is times 4, because we use floats now, which have 4 bytes per float, and buffer needs to have information of amount of bytes to be used
		SDL_QueueAudio(1, Mixbuf.data(), Mixbuf.size() * 4);
		
		SC1buf.clear();
		SC2buf.clear();
		SC3buf.clear();
		SC4buf.clear();
		Mixbuf.clear();
		
		while (SDL_GetQueuedAudioSize(1) > 4096 * 4 * 4) {}
	}

}

void initSPU() {
	SDL_setenv("SDL_AUDIODRIVER", "directsound", 1);
	SDL_Init(SDL_INIT_AUDIO);

	//	initial volumes
	SC1amp = readFromMem(0xff12) >> 4;
	SC2amp = readFromMem(0xff17) >> 4;
	SC3amp = readFromMem(0xff1c) >> 4;
	SC4amp = readFromMem(0xff21) >> 4;

	// Open our audio device; Sample Rate will dictate the pace of our synthesizer
	SDLInitAudio(44100, 4096);

	if (!SoundIsPlaying)
	{
		SDL_PauseAudio(0);
		SoundIsPlaying = true;
	}
}

void stopSPU() {
	SDL_Quit();
	SC1buf.clear();
	SC2buf.clear();
	SC3buf.clear();
	SC4buf.clear();
	SoundIsPlaying = false;
}

//	reloads the length counter for SC1, with all the other according settings
/*
Channel is enabled (see length counter).
If length counter is zero, it is set to 64 (256 for wave channel).
Channel volume is reloaded from NRx2.
Volume envelope timer is reloaded with period.
Enable Volume envelope <----- LILAQ
Wave channel's position is set to 0 but sample buffer is NOT refilled.

Frequency timer is reloaded with period.
Noise channel's LFSR bits are all set to 1.
Square 1's sweep does several things (see frequency sweep).
*/
void resetSC1length(uint8_t val) {
	if(!SC1len)
		SC1len = 64 - val;
	SC1enabled = true;
	SC1sweep = (readFromMem(0xff10) >> 4) & 7;
	if(SC1sweep || (readFromMem(0xff10) & 7))
		SC1sweepEnabled = true;
	else
		SC1sweepEnabled = false;
	SC1amp = readFromMem(0xff12) >> 4;
	SC1envelope = readFromMem(0xff12) & 7;
	SC1envelopeEnabled = true;
}

//	reloads the length counter for SC2, with all the other according settings
void resetSC2length(uint8_t val) {
	if (!SC2len)
		SC2len = 64 - val;
	SC2enabled = true;
	SC2amp = readFromMem(0xff17) >> 4;
	SC2envelope = readFromMem(0xff17) & 7;
	SC2envelopeEnabled = true;
}

//	reloads the length counter for SC3, with all the other according settings
void resetSC3length(uint8_t val) {
	if (!SC3len)
		SC3len = 256 - val;
	SC3enabled = true;
	SC3waveIndex = 0;
}

//	reloads the length counter for SC4, with all the other according settings
void resetSC4length(uint8_t val) {
	if (!SC4len)
		SC4len = 64 - val;
	SC4enabled = true;
	SC4timer = SC4divisor[readFromMem(0xff22) & 0x7] << (readFromMem(0xff22) >> 4);
	SC4lfsr = 0x7fff;
	SC4amp = readFromMem(0xff21) >> 4;
}
