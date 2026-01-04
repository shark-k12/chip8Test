#include "chip8_opcodes.h"
#include "chip8_cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void oc_0000(void)
{
    printf("未知指令: 0x%04X\n", CHIP8_CPU->opcode);
    CHIP8_CPU->pc += 2;
}

void oc_00e0(void)
{
    memset(CHIP8_CPU->video, 0, sizeof(CHIP8_CPU->video));
    CHIP8_CPU->pc += 2;
}

void oc_00ee(void)
{
    CHIP8_CPU->sp--;
    CHIP8_CPU->pc = CHIP8_CPU->stack[CHIP8_CPU->sp];
    CHIP8_CPU->pc += 2;
}

void oc_1nnn(void)
{
    uint16_t addr = CHIP8_CPU->opcode & 0x0FFF;
    CHIP8_CPU->pc = addr;
}

void oc_2nnn(void)
{
    uint16_t addr = CHIP8_CPU->opcode & 0x0FFF;
    CHIP8_CPU->stack[CHIP8_CPU->sp] = CHIP8_CPU->pc + 2;
    CHIP8_CPU->sp++;
    CHIP8_CPU->pc = addr;
}

void oc_3xnn(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t nn = CHIP8_CPU->opcode & 0x00FF;
    if (CHIP8_CPU->registers[x] == nn)
    {
        CHIP8_CPU->pc += 2;
    }
    CHIP8_CPU->pc += 2;
}

void oc_4xnn(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t nn = CHIP8_CPU->opcode & 0x00FF;
    if (CHIP8_CPU->registers[x] != nn)
    {
        CHIP8_CPU->pc += 2;
    }
    CHIP8_CPU->pc += 2;
}

void oc_5xy0(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    if (CHIP8_CPU->registers[x] == CHIP8_CPU->registers[y])
    {
        CHIP8_CPU->pc += 2;
    }
    CHIP8_CPU->pc += 2;
}

void oc_6xnn(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t nn = CHIP8_CPU->opcode & 0x00FF;
    CHIP8_CPU->registers[x] = nn;
    CHIP8_CPU->pc += 2;
}

void oc_7xnn(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t nn = CHIP8_CPU->opcode & 0x00FF;
    CHIP8_CPU->registers[x] += nn;
    CHIP8_CPU->pc += 2;
}

void oc_8xy0(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[x] = CHIP8_CPU->registers[y];
    CHIP8_CPU->pc += 2;
}

void oc_8xy1(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[x] |= CHIP8_CPU->registers[y];
    CHIP8_CPU->registers[0xF] = 0;
    CHIP8_CPU->pc += 2;
}

void oc_8xy2(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[x] &= CHIP8_CPU->registers[y];
    CHIP8_CPU->registers[0xF] = 0;
    CHIP8_CPU->pc += 2;
}

void oc_8xy3(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[x] ^= CHIP8_CPU->registers[y];
    CHIP8_CPU->registers[0xF] = 0;
    CHIP8_CPU->pc += 2;
}

void oc_8xy4(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    uint16_t sum = CHIP8_CPU->registers[x] + CHIP8_CPU->registers[y];
    CHIP8_CPU->registers[x] = sum & 0xFF;
    CHIP8_CPU->registers[0xF] = (sum > 255) ? 1 : 0;
    CHIP8_CPU->pc += 2;
}

void oc_8xy5(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[0xF] = (CHIP8_CPU->registers[x] >= CHIP8_CPU->registers[y]) ? 1 : 0;
    CHIP8_CPU->registers[x] -= CHIP8_CPU->registers[y];
    CHIP8_CPU->pc += 2;
}

void oc_8xy6(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->registers[0xF] = CHIP8_CPU->registers[x] & 0x01;
    CHIP8_CPU->registers[x] >>= 1;
    CHIP8_CPU->pc += 2;
}

void oc_8xy7(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    CHIP8_CPU->registers[0xF] = (CHIP8_CPU->registers[y] >= CHIP8_CPU->registers[x]) ? 1 : 0;
    CHIP8_CPU->registers[x] = CHIP8_CPU->registers[y] - CHIP8_CPU->registers[x];
    CHIP8_CPU->pc += 2;
}

void oc_8xye(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->registers[0xF] = (CHIP8_CPU->registers[x] & 0x80) ? 1 : 0;
    CHIP8_CPU->registers[x] <<= 1;
    CHIP8_CPU->pc += 2;
}

void oc_9xy0(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    if (CHIP8_CPU->registers[x] != CHIP8_CPU->registers[y])
    {
        CHIP8_CPU->pc += 2;
    }
    CHIP8_CPU->pc += 2;
}

void oc_annn(void)
{
    CHIP8_CPU->index = CHIP8_CPU->opcode & 0x0FFF;
    CHIP8_CPU->pc += 2;
}

void oc_bxnn(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->pc = (CHIP8_CPU->opcode & 0x0FFF) + CHIP8_CPU->registers[x];
}

void oc_cxnn(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t nn = CHIP8_CPU->opcode & 0x00FF;
    CHIP8_CPU->registers[x] = (rand() % 256) & nn;
    CHIP8_CPU->pc += 2;
}


void oc_dxyn(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t y = (CHIP8_CPU->opcode & 0x00F0) >> 4;
    uint8_t n = CHIP8_CPU->opcode & 0x000F;

    uint8_t xPos = CHIP8_CPU->registers[x] % 64;
    uint8_t yPos = CHIP8_CPU->registers[y] % 32;

    CHIP8_CPU->registers[0xF] = 0;

    for (uint8_t row = 0; row < n; row++)
    {
        // 防止行越界（超过屏幕高度）
        if ((yPos + row) >= 32) break; 
        uint8_t sprite = CHIP8_CPU->memory[CHIP8_CPU->index + row];
        for (uint8_t col = 0; col < 8; col++)
        {
            // 防止列越界（超过屏幕宽度）
            if ((xPos + col) >= 64) break; 
            if ((sprite & (0x80 >> col)) != 0)
            {
                uint16_t pixelIndex = (yPos + row) * 64 + (xPos + col);
                if (CHIP8_CPU->video[pixelIndex] == 1)
                {
                    CHIP8_CPU->registers[0xF] = 1;
                }
                CHIP8_CPU->video[pixelIndex] ^= 1;
            }
        }
    }
    CHIP8_CPU->pc += 2;
}

void oc_ex9e(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    if (CHIP8_CPU->keypad[CHIP8_CPU->registers[x]] == 1)
    {
        CHIP8_CPU->pc += 2;
    }
    CHIP8_CPU->pc += 2;
}

void oc_exa1(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    if (CHIP8_CPU->keypad[CHIP8_CPU->registers[x]] == 0)
    {
        CHIP8_CPU->pc += 2;
    }
    CHIP8_CPU->pc += 2;
}

void oc_fx07(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->registers[x] = CHIP8_CPU->delayTimer;
    CHIP8_CPU->pc += 2;
}

void oc_fx0a(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    int keyPressed = 0;
    for (int i = 0; i < 16; i++)
    {
        if (CHIP8_CPU->keypad[i] == 1)
        {
            CHIP8_CPU->registers[x] = i;
            keyPressed = 1;
            break;
        }
    }
    if (!keyPressed)
    {
        CHIP8_CPU->pc -= 2;
    }
    else
    {
        CHIP8_CPU->pc += 2;
    }
}

void oc_fx15(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->delayTimer = CHIP8_CPU->registers[x];
    CHIP8_CPU->pc += 2;
}

void oc_fx18(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->soundTimer = CHIP8_CPU->registers[x];
    CHIP8_CPU->pc += 2;
}

void oc_fx1e(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->index += CHIP8_CPU->registers[x];
    CHIP8_CPU->pc += 2;
}

void oc_fx29(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->index = CHIP8_CPU->registers[x] * 5;
    CHIP8_CPU->pc += 2;
}

void oc_fx33(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t value = CHIP8_CPU->registers[x];
    CHIP8_CPU->memory[CHIP8_CPU->index + 2] = value % 10;
    value /= 10;
    CHIP8_CPU->memory[CHIP8_CPU->index + 1] = value % 10;
    value /= 10;
    CHIP8_CPU->memory[CHIP8_CPU->index] = value % 10;
    CHIP8_CPU->pc += 2;
}

void oc_fx55(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    for (uint8_t i = 0; i <= x; i++)
    {
        CHIP8_CPU->memory[CHIP8_CPU->index + i] = CHIP8_CPU->registers[i];
    }
    CHIP8_CPU->index += x + 1;
    CHIP8_CPU->pc += 2;
}

void oc_fx65(void)
{
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    for (uint8_t i = 0; i <= x; i++)
    {
        CHIP8_CPU->registers[i] = CHIP8_CPU->memory[CHIP8_CPU->index + i];
    }
    CHIP8_CPU->index += x + 1;
    CHIP8_CPU->pc += 2;
}

void oc_null(void)
{
    printf("未知指令: 0x%04X\n", CHIP8_CPU->opcode);
    CHIP8_CPU->pc += 2;
}

void oc_exec(void)
{
    switch (CHIP8_CPU->opcode & 0xF000)
    {
        case 0x0000:
            switch (CHIP8_CPU->opcode & 0x00FF)
            {
                case 0x0000:
                    oc_0000();
                    break;
                case 0x00E0:
                    oc_00e0();
                    break;
                case 0x00EE:
                    oc_00ee();
                    break;
                default:
                    oc_null();
                    break;
            }
            break;
        case 0x1000:
            oc_1nnn();
            break;
        case 0x2000:
            oc_2nnn();
            break;
        case 0x3000:
            oc_3xnn();
            break;
        case 0x4000:
            oc_4xnn();
            break;
        case 0x5000:
            oc_5xy0();
            break;
        case 0x6000:
            oc_6xnn();
            break;
        case 0x7000:
            oc_7xnn();
            break;
        case 0x8000:
            switch (CHIP8_CPU->opcode & 0x000F)
            {
                case 0x0000:
                    oc_8xy0();
                    break;
                case 0x0001:
                    oc_8xy1();
                    break;
                case 0x0002:
                    oc_8xy2();
                    break;
                case 0x0003:
                    oc_8xy3();
                    break;
                case 0x0004:
                    oc_8xy4();
                    break;
                case 0x0005:
                    oc_8xy5();
                    break;
                case 0x0006:
                    oc_8xy6();
                    break;
                case 0x0007:
                    oc_8xy7();
                    break;
                case 0x000E:
                    oc_8xye();
                    break;
                default:
                    oc_null();
                    break;
            }
            break;
        case 0x9000:
            oc_9xy0();
            break;
        case 0xA000:
            oc_annn();
            break;
        case 0xB000:
            oc_bxnn();
            break;
        case 0xC000:
            oc_cxnn();
            break;
        case 0xD000:
            oc_dxyn();
            break;
        case 0xE000:
            switch (CHIP8_CPU->opcode & 0x00FF)
            {
                case 0x009E:
                    oc_ex9e();
                    break;
                case 0x00A1:
                    oc_exa1();
                    break;
                default:
                    oc_null();
                    break;
            }
            break;
        case 0xF000:
            switch (CHIP8_CPU->opcode & 0x00FF)
            {
                case 0x0007:
                    oc_fx07();
                    break;
                case 0x000A:
                    oc_fx0a();
                    break;
                case 0x0015:
                    oc_fx15();
                    break;
                case 0x0018:
                    oc_fx18();
                    break;
                case 0x001E:
                    oc_fx1e();
                    break;
                case 0x0029:
                    oc_fx29();
                    break;
                case 0x0033:
                    oc_fx33();
                    break;
                case 0x0055:
                    oc_fx55();
                    break;
                case 0x0065:
                    oc_fx65();
                    break;
                default:
                    oc_null();
                    break;
            }
            break;
        default:
            oc_null();
            break;
    }
}
