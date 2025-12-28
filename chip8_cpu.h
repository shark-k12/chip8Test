#ifndef CHIP8_CPU_H_ 
#define CHIP8_CPU_H_ 

#include <stdint.h>

typedef struct {
    uint8_t registers[16];
	uint8_t memory[4096];
	uint16_t index;
	uint16_t pc;
	uint16_t stack[16];
	uint8_t sp;
	uint8_t delayTimer;
	uint8_t soundTimer;
	uint8_t keypad[16];
	uint8_t video[64*32];
	uint16_t opcode;  //当前要执行的指令
} chip8_cpu_t;


extern chip8_cpu_t * CHIP8_CPU;

int loadrom(const char *rom); //加载rom
void init(void); //初始化
void destroy(void); //回收
void cycle(void); //执行循环

#endif
