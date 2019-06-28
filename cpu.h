#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <cstdint>
#include "structs.h"
#include "mmu.h"
int processOpcode(uint16_t& pc, uint16_t& sp, Registers& registers, Flags& flags, int& interrupts_enabled);
//	opcodes functions
void op_rlc(unsigned char& parameter, Flags& flags, Registers& registers, uint16_t& pc);