#include "chip8_opcodes.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void execute_opcode(Chip8* cpu, uint16_t opcode) {
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t n = opcode & 0x000F;
    uint8_t nn = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;
    
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode) {
                case 0x00E0:
                    for (int i = 0; i < SCREEN_WIDTH; i++) {
                        for (int j = 0; j < SCREEN_HEIGHT; j++) {
                            cpu->screen[i][j] = 0;
                        }
                    }
                    cpu->draw_flag = true;
                    break;
                    
                case 0x00EE:
                    cpu->sp--;
                    cpu->pc = cpu->stack[cpu->sp];
                    break;
                    
                default:
                    printf("Unknown opcode: 0x%04X\n", opcode);
                    break;
            }
            break;
            
        case 0x1000:
            cpu->pc = nnn;
            break;
            
        case 0x2000:
            cpu->stack[cpu->sp] = cpu->pc;
            cpu->sp++;
            cpu->pc = nnn;
            break;
            
        case 0x3000:
            if (cpu->V[x] == nn) {
                cpu->pc += 2;
            }
            break;
            
        case 0x4000:
            if (cpu->V[x] != nn) {
                cpu->pc += 2;
            }
            break;
            
        case 0x5000:
            if (cpu->V[x] == cpu->V[y]) {
                cpu->pc += 2;
            }
            break;
            
        case 0x6000:
            cpu->V[x] = nn;
            break;
            
        case 0x7000:
            cpu->V[x] += nn;
            break;
            
        case 0x8000:
            switch (n) {
                case 0x0:
                    cpu->V[x] = cpu->V[y];
                    break;
                    
                case 0x1:
                    cpu->V[x] |= cpu->V[y];
                    cpu->V[0xF] = 0;
                    break;
                    
                case 0x2:
                    cpu->V[x] &= cpu->V[y];
                    cpu->V[0xF] = 0;
                    break;
                    
                case 0x3:
                    cpu->V[x] ^= cpu->V[y];
                    cpu->V[0xF] = 0;
                    break;
                    
                case 0x4: {
                    uint16_t sum = cpu->V[x] + cpu->V[y];
                    cpu->V[x] = sum & 0xFF;
                    cpu->V[0xF] = (sum > 255) ? 1 : 0;
                    break;
                }
                    
                case 0x5:
                    cpu->V[0xF] = (cpu->V[x] > cpu->V[y]) ? 1 : 0;
                    cpu->V[x] -= cpu->V[y];
                    break;
                    
                case 0x6:
                    cpu->V[0xF] = cpu->V[y] & 0x1;
                    cpu->V[x] = cpu->V[y] >> 1;
                    break;
                    
                case 0x7:
                    cpu->V[0xF] = (cpu->V[y] > cpu->V[x]) ? 1 : 0;
                    cpu->V[x] = cpu->V[y] - cpu->V[x];
                    break;
                    
                case 0xE:
                    cpu->V[0xF] = (cpu->V[y] & 0x80) >> 7;
                    cpu->V[x] = cpu->V[y] << 1;
                    break;
                    
                default:
                    printf("Unknown opcode: 0x%04X\n", opcode);
                    break;
            }
            break;
            
        case 0x9000:
            if (cpu->V[x] != cpu->V[y]) {
                cpu->pc += 2;
            }
            break;
            
        case 0xA000:
            cpu->I = nnn;
            break;
            
        case 0xB000:
            cpu->pc = nnn + cpu->V[0];
            break;
            
        case 0xC000: {
            srand(time(NULL));
            uint8_t random = rand() % 256;
            cpu->V[x] = random & nn;
            break;
        }
            
        case 0xD000: {
            uint8_t xPos = cpu->V[x] % SCREEN_WIDTH;
            uint8_t yPos = cpu->V[y] % SCREEN_HEIGHT;
            cpu->V[0xF] = 0;
            
            for (int row = 0; row < n; row++) {
                uint8_t sprite = cpu->memory[cpu->I + row];
                for (int col = 0; col < 8; col++) {
                    if ((sprite & (0x80 >> col)) != 0) {
                        int xPixel = (xPos + col) % SCREEN_WIDTH;
                        int yPixel = (yPos + row) % SCREEN_HEIGHT;
                        
                        if (cpu->screen[xPixel][yPixel] == 1) {
                            cpu->V[0xF] = 1;
                        }
                        
                        cpu->screen[xPixel][yPixel] ^= 1;
                    }
                }
            }
            cpu->draw_flag = true;
            break;
        }
            
        case 0xE000:
            switch (nn) {
                case 0x9E:
                    if (cpu->keypad[cpu->V[x]] == 1) {
                        cpu->pc += 2;
                    }
                    break;
                    
                case 0xA1:
                    if (cpu->keypad[cpu->V[x]] == 0) {
                        cpu->pc += 2;
                    }
                    break;
                    
                default:
                    printf("Unknown opcode: 0x%04X\n", opcode);
                    break;
            }
            break;
            
        case 0xF000:
            switch (nn) {
                case 0x07:
                    cpu->V[x] = cpu->delay_timer;
                    break;
                    
                case 0x0A:
                    for (int i = 0; i < KEY_COUNT; i++) {
                        if (cpu->keypad[i] == 1) {
                            cpu->V[x] = i;
                            break;
                        }
                    }
                    cpu->pc -= 2;
                    break;
                    
                case 0x15:
                    cpu->delay_timer = cpu->V[x];
                    break;
                    
                case 0x18:
                    cpu->sound_timer = cpu->V[x];
                    break;
                    
                case 0x1E:
                    cpu->I += cpu->V[x];
                    break;
                    
                case 0x29:
                    cpu->I = cpu->V[x] * 5;
                    break;
                    
                case 0x33:
                    cpu->memory[cpu->I] = cpu->V[x] / 100;
                    cpu->memory[cpu->I + 1] = (cpu->V[x] / 10) % 10;
                    cpu->memory[cpu->I + 2] = cpu->V[x] % 10;
                    break;
                    
                case 0x55:
                    for (int i = 0; i <= x; i++) {
                        cpu->memory[cpu->I + i] = cpu->V[i];
                    }
                    cpu->I += x + 1;
                    break;
                    
                case 0x65:
                    for (int i = 0; i <= x; i++) {
                        cpu->V[i] = cpu->memory[cpu->I + i];
                    }
                    cpu->I += x + 1;
                    break;
                    
                default:
                    printf("Unknown opcode: 0x%04X\n", opcode);
                    break;
            }
            break;
            
        default:
            printf("Unknown opcode: 0x%04X\n", opcode);
            break;
    }
}
