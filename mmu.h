#pragma once
#include <cstdint> 
void initMMU();
void loadROM(unsigned char rom[]);
void loadBootROM();
void lockBootROM();
void writeToMem(uint16_t, unsigned char);
void aluToMem(uint16_t, int8_t);
unsigned char& readFromMem(uint16_t);
