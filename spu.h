#pragma once
void initSPU();
void stepSPU(unsigned char cycles);
void resetSC1length(uint8_t val);
void resetSC2length(uint8_t val);
void resetSC3length(uint8_t val);
void resetSC4length(uint8_t val);
void stopSPU();
void toggleAudio();
void toggleSC1();
void toggleSC2();
void toggleSC3();
void toggleSC4();
void toggleRemix();
void setVolume(float v);