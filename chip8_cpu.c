#include "chip8_cpu.h"
#include "chip8_opcodes.h"
#include "chip8_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,
    0x20, 0x60, 0x20, 0x20, 0x70,
    0xF0, 0x10, 0xF0, 0x80, 0xF0,
    0xF0, 0x10, 0xF0, 0x10, 0xF0,
    0x90, 0x90, 0xF0, 0x10, 0x10,
    0xF0, 0x80, 0xF0, 0x10, 0xF0,
    0xF0, 0x80, 0xF0, 0x90, 0xF0,
    0xF0, 0x10, 0x20, 0x40, 0x40,
    0xF0, 0x90, 0xF0, 0x90, 0xF0,
    0xF0, 0x90, 0xF0, 0x10, 0xF0,
    0xF0, 0x90, 0xF0, 0x90, 0x90,
    0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0,
    0xE0, 0x90, 0x90, 0x90, 0xE0,
    0xF0, 0x80, 0xF0, 0x80, 0xF0,
    0xF0, 0x80, 0xF0, 0x80, 0x80
};

void chip8_init(Chip8* cpu) {
    memset(cpu->memory, 0, MEMORY_SIZE);
    memset(cpu->V, 0, REGISTER_COUNT);
    memset(cpu->stack, 0, STACK_SIZE * sizeof(uint16_t));
    memset(cpu->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
    memset(cpu->keypad, 0, KEY_COUNT);
    
    cpu->I = 0;
    cpu->pc = 0x200;
    cpu->sp = 0;
    cpu->delay_timer = 0;
    cpu->sound_timer = 0;
    cpu->draw_flag = false;
    
    memcpy(cpu->memory, fontset, sizeof(fontset));
}

void chip8_load_rom(Chip8* cpu, const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Error: Could not open ROM file: %s\n", filename);
        exit(1);
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    if (file_size > (MEMORY_SIZE - 0x200)) {
        printf("Error: ROM file is too large\n");
        fclose(file);
        exit(1);
    }
    
    size_t bytes_read = fread(cpu->memory + 0x200, 1, file_size, file);
    fclose(file);
    
    printf("Loaded %zu bytes from %s\n", bytes_read, filename);
}

void chip8_cycle(Chip8* cpu) {
    uint16_t opcode = cpu->memory[cpu->pc] << 8 | cpu->memory[cpu->pc + 1];
    cpu->pc += 2;
    
    execute_opcode(cpu, opcode);
}

void chip8_decrement_timers(Chip8* cpu) {
    if (cpu->delay_timer > 0) {
        cpu->delay_timer--;
    }
    
    if (cpu->sound_timer > 0) {
        cpu->sound_timer--;
        if (cpu->sound_timer == 0) {
            platform_beep();
        }
    }
}
