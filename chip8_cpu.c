#include "chip8_cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

chip8_cpu_t *CHIP8_CPU = NULL;

int loadrom(const char *rom)
{
    FILE *fp = fopen(rom, "rb");
    if (!fp)
    {
        fprintf(stderr, "无法打开ROM文件: %s\n", rom);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size > MEMORY_SIZE - ROM_START_ADDR)
    {
        fprintf(stderr, "ROM文件太大: %ld 字节\n", size);
        fclose(fp);
        return -1;
    }

    fread(CHIP8_CPU->memory + ROM_START_ADDR, 1, size, fp);
    fclose(fp);

    printf("ROM加载成功: %s (%ld字节) 加载到0x%04X\n", rom, size, ROM_START_ADDR);
    return 0;
}

void init(void)
{
    CHIP8_CPU = (chip8_cpu_t *)malloc(sizeof(chip8_cpu_t));
    if (!CHIP8_CPU)
    {
        fprintf(stderr, "内存分配失败\n");
        exit(1);
    }

    memset(CHIP8_CPU, 0, sizeof(chip8_cpu_t));

    CHIP8_CPU->pc = ROM_START_ADDR;
    CHIP8_CPU->sp = 0;
    CHIP8_CPU->index = 0;
    CHIP8_CPU->opcode = 0;

    uint8_t fontset[80] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    for (int i = 0; i < 80; i++)
    {
        CHIP8_CPU->memory[i] = fontset[i];
    }

    printf("Chip8初始化成功 PC=0x%04X, SP=0x%02X, Index=0x%04X\n",
           CHIP8_CPU->pc, CHIP8_CPU->sp, CHIP8_CPU->index);
}

void destroy(void)
{
    if (CHIP8_CPU)
    {
        free(CHIP8_CPU);
        CHIP8_CPU = NULL;
    }
}

void cycle(void)
{
    CHIP8_CPU->opcode = (CHIP8_CPU->memory[CHIP8_CPU->pc] << 8) | CHIP8_CPU->memory[CHIP8_CPU->pc + 1];

    extern void oc_exec(void);
    oc_exec();
}
