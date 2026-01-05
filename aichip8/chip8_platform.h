#ifndef CHIP8_PLATFORM_H
#define CHIP8_PLATFORM_H

#include "chip8_cpu.h"

void platform_init(void);
void platform_cleanup(void);
void platform_draw(const Chip8* cpu);
void platform_handle_input(Chip8* cpu);
void platform_beep(void);
int platform_should_quit(void);

#endif
