#pragma once
#include <cstdint> 
#include "structs.h"
void initMMU();
void loadROM(unsigned char rom[]);
void loadBootROM();
void lockBootROM();
void writeToMem(uint16_t, unsigned char);
void aluToMem(uint16_t, int8_t);
unsigned char& readFromMem(uint16_t);
void saveState(char filename[], Registers& registers, Flags& flags, uint16_t pc, uint16_t sp);
void loadState(char filename[], Registers & registers, Flags & flags, uint16_t pc, uint16_t sp);
