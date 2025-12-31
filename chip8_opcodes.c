#include <stdio.h>
#include <string.h>
#include <time.h>
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

// 00E0 - CLS: 清屏
void oc_00e0(void){
    memset(CHIP8_CPU->video, 0, sizeof(CHIP8_CPU->video));
}

// 00EE - RET: 返回从栈中弹出的地址
void oc_00ee(void){
    CHIP8_CPU->sp--;
    CHIP8_CPU->pc = CHIP8_CPU->stack[CHIP8_CPU->sp];
}

// 1NNN - JP addr: 跳转到地址NNN
void oc_1nnn(void){
    uint16_t address = CHIP8_CPU->opcode & 0x0fff;
    CHIP8_CPU->pc = address;
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

//5XY0 - SE Vx, Vy: Vx==Vy则跳过下一条指令
void oc_5xy0(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    if (CHIP8_CPU->registers[x] == CHIP8_CPU->registers[y]) {
        CHIP8_CPU->pc += 4;
    } else {
        CHIP8_CPU->pc += 2;
    }
}

//6XNN - LD Vx, byte: Vx=NN
void oc_6xnn(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t nn = CHIP8_CPU->opcode & 0x00FF;
    CHIP8_CPU->registers[x] = nn;
}

//7XNN - ADD Vx, byte: Vx=Vx+NN
void oc_7xnn(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t nn = CHIP8_CPU->opcode & 0x00FF;
    CHIP8_CPU->registers[x] += nn;
    CHIP8_CPU->pc += 2;
}

//8XY0 - LD Vx, Vy: Vx=Vy
void oc_8xy0(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[x] = CHIP8_CPU->registers[y];
    CHIP8_CPU->pc += 2;
}

//8XY1 - OR Vx, Vy: Vx=Vx OR Vy
void oc_8xy1(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[x] |= CHIP8_CPU->registers[y];
    CHIP8_CPU->registers[0xf] = 0;
    CHIP8_CPU->pc += 2;
}

//8XY2 - AND Vx, Vy: Vx=Vx AND Vy
void oc_8xy2(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[x] &= CHIP8_CPU->registers[y];
    CHIP8_CPU->registers[0xf] = 0;
    CHIP8_CPU->pc += 2;
}

//8XY3 - XOR Vx, Vy: Vx=Vx XOR Vy
void oc_8xy3(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[x] ^= CHIP8_CPU->registers[y];
    CHIP8_CPU->registers[0xf] = 0;
    CHIP8_CPU->pc += 2;
}

//8XY4 - ADD Vx, Vy: Vx=Vx+Vy, VF=进位标志
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

//8XY5 - SUB Vx, Vy: Vx=Vx-Vy, VF=借位标志
void oc_8xy5(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    if (CHIP8_CPU->registers[x] >= CHIP8_CPU->registers[y]) {
        CHIP8_CPU->registers[0xf] = 1;
    } else {
        CHIP8_CPU->registers[0xf] = 0;
    }
    CHIP8_CPU->registers[x] -= CHIP8_CPU->registers[y];
    CHIP8_CPU->pc += 2;
}

//8XY6 - SHR Vx {, Vy}: Vx=Vx SHR 1, VF=LSB(Vx)
void oc_8xy6(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[0xf] = CHIP8_CPU->registers[x] & 0x01;
    CHIP8_CPU->registers[x] >>= 1;
    CHIP8_CPU->pc += 2;
}

//8XY7 - SUBN Vx, Vy: Vx=Vy-Vx, VF=借位标志
void oc_8xy7(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    if (CHIP8_CPU->registers[y] >= CHIP8_CPU->registers[x]) {
        CHIP8_CPU->registers[0xf] = 1;
    } else {
        CHIP8_CPU->registers[0xf] = 0;
    }
    CHIP8_CPU->registers[x] = CHIP8_CPU->registers[y] - CHIP8_CPU->registers[x];
    CHIP8_CPU->pc += 2;
}

//8XYE - SHL Vx {, Vy}: Vx=Vx SHL 1, VF=MSB(Vx)
void oc_8xye(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[0xf] = (CHIP8_CPU->registers[x] & 0x80) ? 1:0;
    CHIP8_CPU->registers[x] <<= 1;
    CHIP8_CPU->pc += 2;
}

//9XY0 - SNE Vx, Vy: Vx!=Vy则跳过下一条指令
void oc_9xy0(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    if (CHIP8_CPU->registers[x] != CHIP8_CPU->registers[y]) {
        CHIP8_CPU->pc += 4;
    } else {
        CHIP8_CPU->pc += 2;
    }
}

//ANNN - LD I, addr: I=NNN
void oc_annn(void) {
    uint16_t nnn = CHIP8_CPU->opcode & 0x0FFF;
    CHIP8_CPU->index = nnn;
    CHIP8_CPU->pc += 2;
}

//BXNN - JP V0, addr: 跳转到地址NNN+V0
void oc_bxnn(void) {
    uint16_t nnn = CHIP8_CPU->opcode & 0x0FFF;
    CHIP8_CPU->registers[0xf] = 0;
    CHIP8_CPU->pc = CHIP8_CPU->registers[0] + nnn;
}

//CXNN - RND Vx, byte: Vx=随机数 AND NN
void oc_cxnn(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t nn = CHIP8_CPU->opcode & 0x00FF;
    // 初始化随机数生成器
    static int rand_init = 0;

    if (!rand_init) {// 确保随机数生成器只初始化一次
        srand((unsigned int)time(NULL));
        rand_init = 1;
    }
    CHIP8_CPU->registers[x] = (rand() % 256) & nn;
    CHIP8_CPU->registers[0xf] = 0;
    CHIP8_CPU->pc += 2;
}

//DXYN - DRW Vx, Vy, nibble: 在坐标(Vx, Vy)处绘制宽8像素高N像素的精灵/
void oc_dxyn(void) {
    // 从操作码中提取寄存器索引x和y，以及精灵高度n
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    uint8_t n = (CHIP8_CPU->opcode & 0x000F);
    
    // 获取屏幕绘制位置(Vx, Vy)
    uint8_t pos_x = CHIP8_CPU->registers[x] % 64;  // 确保x坐标在0-63范围内
    uint8_t pos_y = CHIP8_CPU->registers[y] % 32;  // 确保y坐标在0-31范围内
    
    // 初始化碰撞标志为0
    CHIP8_CPU->registers[0xF] = 0;
    
    // 绘制n行精灵数据
    for (uint8_t row = 0; row < n; row++) {
        // 从内存地址I+row读取当前行的精灵数据(8个像素)
        uint8_t sprite_data = CHIP8_CPU->memory[CHIP8_CPU->index + row];
        uint8_t screen_y = (pos_y + row) % 32;
        
        // 逐位绘制当前行的8个像素
        for (uint8_t col = 0; col < 8; col++) {
            // 计算当前像素在屏幕上的位置(考虑环绕)
            uint8_t screen_x = (pos_x + col) % 64;
            uint32_t pixel_index = screen_y * 64 + screen_x;
            uint8_t sprite_pixel = (sprite_data >> (7 - col)) & 0x1;
            
            
            if (sprite_pixel) {
                // 检查是否发生碰撞(当前屏幕像素为1，即将被翻转为0)
                if (CHIP8_CPU->video[pixel_index]) {
                    CHIP8_CPU->registers[0xF] = 1;  // 设置碰撞标志
                }
                // 翻转像素状态
                CHIP8_CPU->video[pixel_index] ^= 1;
            }
        }
    }
    
    CHIP8_CPU->pc += 2;
}

//默认未知指令处理
void oc_null(void) 
{
    printf("[STATE][OPCODE] Unknown :(\n");
}




