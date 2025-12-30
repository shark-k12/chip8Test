#include <stdio.h>
#include <stdlib.h>
#include "chip8_cpu.h"
#include "chip8_opcodes.h"

void oc_exec(void)
{
    switch (CHIP8_CPU->opcode & 0xf000)
    {
        //@TODO 根据opcode决定调用哪个具体实现, 添加具体实现 
        case 0xc000:
            //oc_cxnn();
            break;
        default:
            oc_null();
            break;
    }
}

void oc_00e0(void){
    memset(CHIP8_CPU->video, 0, sizeof(CHIP8_CPU->video));
}

void oc_00ee(void){
    CHIP8_CPU->sp--;
    CHIP8_CPU->pc = CHIP8_CPU->stack[CHIP8_CPU->sp];
}

void oc_1nnn(void){
    uint16_t address = CHIP8_CPU->opcode & 0x0fff;
    CHIP8_CPU->pc = address;
}

void oc_null(void) 
{
    printf("[STATE][OPCODE] Unknown :(\n");
}

void oc_8xy4(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0f00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00f0) >> 4;
    uint16_t result = CHIP8_CPU->registers[x] + CHIP8_CPU->registers[y];
    if (result > 0xff) { CHIP8_CPU->registers[0xf] = 1; }
    else { CHIP8_CPU->registers[0xf] = 0; }

    CHIP8_CPU->registers[x] = (result & 0xff);
    CHIP8_CPU->pc += 2;     
}

// 2NNN - CALL addr: 调用子程序（地址NNN）
void oc_2nnn(void) {
    uint16_t address = CHIP8_CPU->opcode & 0x0FFF;
    CHIP8_CPU->stack[CHIP8_CPU->sp] = CHIP8_CPU->pc;
    CHIP8_CPU->sp++;
    CHIP8_CPU->pc = address;
}

// 3XNN - SE Vx, byte: Vx==NN则跳过下一条指令
void oc_3xnn(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t nn = CHIP8_CPU->opcode & 0x00FF;
    if (CHIP8_CPU->registers[x] == nn) {
        CHIP8_CPU->pc += 4;
    } else {
        CHIP8_CPU->pc += 2;
    }
}

// 4XNN - SNE Vx, byte: Vx!=NN则跳过下一条指令
void oc_4xnn(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t nn = CHIP8_CPU->opcode & 0x00FF;
    if (CHIP8_CPU->registers[x] != nn) {
        CHIP8_CPU->pc += 4;
    } else {
        CHIP8_CPU->pc += 2;
    }
}