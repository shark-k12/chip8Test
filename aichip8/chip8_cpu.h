#ifndef CHIP8_CPU_H
#define CHIP8_CPU_H

#include <stdint.h>
#include <stdbool.h>

#define MEMORY_SIZE 4096
#define REGISTER_COUNT 16
#define STACK_SIZE 16
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define KEY_COUNT 16

typedef struct {
    uint8_t memory[MEMORY_SIZE];
    uint8_t V[REGISTER_COUNT];
    uint16_t I;
    uint16_t pc;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint16_t stack[STACK_SIZE];
    uint8_t sp;
    uint8_t screen[SCREEN_WIDTH][SCREEN_HEIGHT];
    uint8_t keypad[KEY_COUNT];
    bool draw_flag;
} Chip8;

void chip8_init(Chip8* cpu);
void chip8_load_rom(Chip8* cpu, const char* filename);
void chip8_cycle(Chip8* cpu);
void chip8_decrement_timers(Chip8* cpu);

#endif
