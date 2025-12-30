#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "chip8_cpu.h"
#include "chip8_opcodes.h"

static const unsigned char FONTSET[80] = 
{
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

chip8_cpu_t * CHIP8_CPU = NULL;

void init(void) 
{
    //有实例，先销毁
    if (CHIP8_CPU != NULL) 
    {
        destroy();
    }

    //分配内存
    CHIP8_CPU = malloc(sizeof(chip8_cpu_t));
    if (CHIP8_CPU == NULL) 
    {
        printf("Chip8初始化失败:无法分配CPU内存\n");
        return;
    }

    memset(CHIP8_CPU, 0, sizeof(chip8_cpu_t));

    // 加载字体集到内存中, 从0x000开始到0x04F结束
    for (size_t i = 0; i < 80; i++)
    {
        CHIP8_CPU->memory[i] = FONTSET[i];
    }

    // 设置PC为ROM_START_ADDR,从0x200开始
    CHIP8_CPU->pc = ROM_START_ADDR;
    CHIP8_CPU->sp = 0;
    CHIP8_CPU->index = 0;

    fprintf(stdout, "Chip8初始化成功:PC=0x%04X, SP=0x%02X, Index=0x%04X\n", CHIP8_CPU->pc, CHIP8_CPU->sp, CHIP8_CPU->index);    
}

void destroy(void) 
{
    free(CHIP8_CPU);
    CHIP8_CPU = NULL;
}

int loadrom(const char *rom) 
{
    FILE *file = NULL;
    long file_size = 0;
    size_t bytes_read = 0;

    if (CHIP8_CPU == NULL) 
    {
        fprintf(stderr, "加载ROM失败: CPU未初始化\n");
        return -1;
    }

    if (rom == NULL) 
    {
        fprintf(stderr, "加载ROM失败: 文件路径为空\n");
        return -1;
    }

    file = fopen(rom, "rb");
    if (file == NULL) 
    {
        fprintf(stderr, "加载ROM失败: 无法打开文件 %s\n", rom);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size > (MEMORY_SIZE - ROM_START_ADDR)) 
    {
        fprintf(stderr, "加载ROM失败: ROM文件过大 (%ld字节), 超过可用内存 (%d字节)\n", 
                file_size, MEMORY_SIZE - ROM_START_ADDR);
        fclose(file);
        return -1;
    }

    bytes_read = fread(CHIP8_CPU->memory + ROM_START_ADDR, 1, file_size, file);
    if (bytes_read != (size_t)file_size) 
    {
        fprintf(stderr, "加载ROM失败: 读取文件错误 (期望%ld字节, 实际读取%zu字节)\n", 
                file_size, bytes_read);
        fclose(file);
        return -1;
    }

    fclose(file);

    CHIP8_CPU->pc = ROM_START_ADDR;

    fprintf(stdout, "ROM加载成功: %s (%ld字节) 加载到 0x%04X\n", rom, file_size, ROM_START_ADDR);
    return 0;
}

void cycle(void)  //模拟一个指令周期
{
    // Fetch指令，从pc处每次取两个字节到opcode中

	// Increment the PC before we execute anything,  pc += 2;

	// Decode and Execute, 执行每条指令对应的行为, 调用oc_exec

	// Decrement the delay timer if it's been set，此处需要考虑60Hz与CPU每秒执行多少条指令的关系

	// Decrement the sound timer if it's been set，此处需要考虑60Hz与CPU每秒执行多少条指令的关系
 
}
