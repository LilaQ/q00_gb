#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstdint>
#include "structs.h"
void processOpcode(uint16_t& pc, uint16_t& sp, unsigned char memory[], Registers& registers, Flags& flags, int& interrupts_enabled);
//	opcodes functions
void op_rlc(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc);