#ifndef CHIP8_OPCODES_H
#define CHIP8_OPCODES_H

#include "chip8_cpu.h"

void execute_opcode(Chip8* cpu, uint16_t opcode);

#endif
