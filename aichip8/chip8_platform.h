#ifndef CHIP8_PLATFORM_H
#define CHIP8_PLATFORM_H

#include "chip8_cpu.h"

extern int speed_percent;

void platform_init(void);
void platform_cleanup(void);
void platform_draw(const Chip8* cpu);
int platform_handle_input(Chip8* cpu);
void platform_beep(void);
void platform_draw_speed(const char* text);
int platform_should_quit(void);

void platform_draw_speed(const char* text); //速度显示函数

#endif
