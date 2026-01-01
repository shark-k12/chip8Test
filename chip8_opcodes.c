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
        case 0x0000:
            switch (CHIP8_CPU->opcode & 0x00ff)
            {
                case 0x00e0:
                    oc_00e0();
                    break;
                case 0x00ee:
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
            switch (CHIP8_CPU->opcode & 0x000f)
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
                case 0x000e:
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
        case 0xa000:
            oc_annn();
            break;
        case 0xb000:
            oc_bxnn();
            break;
        case 0xc000:
            oc_cxnn();
            break;
        case 0xd000:
            oc_dxyn();
            break;
        case 0xe000:
            switch (CHIP8_CPU->opcode & 0x00ff)
            {
                case 0x009e:
                    oc_ex9e();
                    break;
                case 0x00a1:
                    oc_exa1();
                    break;
                default:
                    oc_null();
                    break;
            }
            break;
        case 0xf000:
            switch (CHIP8_CPU->opcode & 0x00ff)
            {
                case 0x0007:
                    oc_fx07();
                    break;
                case 0x000a:
                    oc_fx0a();
                    break;
                case 0x0015:
                    oc_fx15();
                    break;
                case 0x0018:
                    oc_fx18();
                    break;
                case 0x001e:
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
// EX9E - SKP Vx: 如果Vx寄存器中存储的键码对应的按键被按下，则跳过下一条指令
void oc_ex9e(void){
    // 从操作码中提取寄存器索引x
    uint8_t Vx = (CHIP8_CPU->opcode & 0x0F00u) >> 8u;

    // 获取Vx寄存器中存储的键码
	uint8_t key = CHIP8_CPU->registers[Vx];

    // 检查键盘状态数组中该键码对应的按键是否被按下
	if (CHIP8_CPU->keypad[key])
	{
        // PC（程序计数器）增加2，以跳过下一条指令
		CHIP8_CPU->pc += 2;
	}
}

// EXA1 - SKNP Vx: 如果Vx寄存器中存储的键码对应的按键没有被按下，则跳过下一条指令
void oc_exa1(void){
    // 从操作码中提取寄存器索引x
    uint8_t Vx = (CHIP8_CPU->opcode & 0x0F00u) >> 8u;

    // 获取Vx寄存器中存储的键码
	uint8_t key = CHIP8_CPU->registers[Vx];

    // 检查键盘状态数组中该键码对应的按键是否未被按下
	if (!CHIP8_CPU->keypad[key])
	{
        // PC（程序计数器）增加2，以跳过下一条指令
		CHIP8_CPU->pc += 2;
	}
}

// FX07 - LD Vx, DT: 将延时计时器(Delay Timer)的值加载到Vx寄存器中
void oc_fx07(void){
    // 从操作码中提取寄存器索引x
    uint8_t Vx = (CHIP8_CPU->opcode & 0x0F00u) >> 8u;

    // 将延时计时器的当前值赋给Vx寄存器
	CHIP8_CPU->registers[Vx] = CHIP8_CPU->delayTimer;
}

// FX0A - LD Vx, K: 等待按键输入，并将按下的键码存储到Vx寄存器中
void oc_fx0a(void){
    // 从操作码中提取寄存器索引x
    uint8_t Vx = (CHIP8_CPU->opcode & 0x0F00u) >> 8u;

    // 按顺序检查键盘上的每个按键（0到15）
	if (CHIP8_CPU->keypad[0])
	{
        // 如果按键0被按下，将键码0存入Vx
		CHIP8_CPU->registers[Vx] = 0;
	}
	else if (CHIP8_CPU->keypad[1])
	{
		CHIP8_CPU->registers[Vx] = 1;
	}
	else if (CHIP8_CPU->keypad[2])
	{
		CHIP8_CPU->registers[Vx] = 2;
	}
	else if (CHIP8_CPU->keypad[3])
	{
		CHIP8_CPU->registers[Vx] = 3;
	}
	else if (CHIP8_CPU->keypad[4])
	{
		CHIP8_CPU->registers[Vx] = 4;
	}
	else if (CHIP8_CPU->keypad[5])
	{
		CHIP8_CPU->registers[Vx] = 5;
	}
	else if (CHIP8_CPU->keypad[6])
	{
		CHIP8_CPU->registers[Vx] = 6;
	}
	else if (CHIP8_CPU->keypad[7])
	{
		CHIP8_CPU->registers[Vx] = 7;
	}
	else if (CHIP8_CPU->keypad[8])
	{
		CHIP8_CPU->registers[Vx] = 8;
	}
	else if (CHIP8_CPU->keypad[9])
	{
        // 如果按键9被按下，将键码9存入Vx
		CHIP8_CPU->registers[Vx] = 9;
	}
	else if (CHIP8_CPU->keypad[10])
	{
        // 如果按键A被按下，将键码10存入Vx
		CHIP8_CPU->registers[Vx] = 10;
	}
	else if (CHIP8_CPU->keypad[11])
	{
		CHIP8_CPU->registers[Vx] = 11;
	}
	else if (CHIP8_CPU->keypad[12])
	{
		CHIP8_CPU->registers[Vx] = 12;
	}
	else if (CHIP8_CPU->keypad[13])
	{
		CHIP8_CPU->registers[Vx] = 13;
	}
	else if (CHIP8_CPU->keypad[14])
	{
		CHIP8_CPU->registers[Vx] = 14;
	}
	else if (CHIP8_CPU->keypad[15])
	{
        // 如果按键F被按下，将键码15存入Vx
		CHIP8_CPU->registers[Vx] = 15;
	}
	else
	{
        //PC（程序计数器）减2，使CPU在下一个时钟周期重新执行此指令，从而实现等待
		CHIP8_CPU->pc -= 2;
	}
}

// FX15 - LD DT, Vx: 将Vx寄存器的值加载到延时计时器(Delay Timer)中
void oc_fx15(void){
    // 从操作码中提取寄存器索引x
    uint8_t Vx = (CHIP8_CPU->opcode & 0x0F00u) >> 8u;

    // 将Vx寄存器的值赋给延时计时器
	CHIP8_CPU->delayTimer = CHIP8_CPU->registers[Vx];
}

// FX18 - LD ST, Vx: 将Vx寄存器的值加载到声音计时器(Sound Timer)中
void oc_fx18(void){
    // 从操作码中提取寄存器索引x
	uint8_t Vx = (CHIP8_CPU->opcode & 0x0F00u) >> 8u;

    // 将Vx寄存器的值赋给声音计时器
	CHIP8_CPU->soundTimer = CHIP8_CPU->registers[Vx];
}

// FX1E - ADD I, Vx: 将Vx寄存器的值加到I(Index)寄存器上
void oc_fx1e(void){
    // 从操作码中提取寄存器索引x
	uint8_t Vx = (CHIP8_CPU->opcode & 0x0F00u) >> 8u;

    // 将Vx寄存器的值与I寄存器的当前值相加，结果存回I寄存器
	CHIP8_CPU->index += CHIP8_CPU->registers[Vx];
}

// FX29 - LD F, Vx: 将I(Index)寄存器设置为Vx寄存器中存储的数字的十六进制字形在内存中的地址
void oc_fx29(void){
    // 从操作码中提取寄存器索引x
    uint8_t Vx = (CHIP8_CPU->opcode & 0x0F00u) >> 8u;
    
    // 获取Vx寄存器中的数字（0-15）
	uint8_t digit = CHIP8_CPU->registers[Vx];

    // 计算字形地址并更新I寄存器。字形通常存储在ROM起始地址之后，每个字形占5个字节。
	CHIP8_CPU->index = ROM_START_ADDR + (5 * digit);
}

// FX33 - LD B, Vx: 将Vx寄存器中的值（0-255）分解为百位、十位、个位，并依次存储到I, I+1, I+2指向的内存中
void oc_fx33(void){
    // 从操作码中提取寄存器索引x
	uint8_t Vx = (CHIP8_CPU->opcode & 0x0F00u) >> 8u;
    
    // 获取Vx寄存器中的值
	uint8_t value = CHIP8_CPU->registers[Vx];

	// 计算个位数，并存储到I+2指向的内存
	CHIP8_CPU->memory[CHIP8_CPU->index + 2] = value % 10;
	value /= 10;

	// 计算十位数，并存储到I+1指向的内存
	CHIP8_CPU->memory[CHIP8_CPU->index + 1] = value % 10;
	value /= 10;

	// 计算百位数，并存储到I指向的内存
	CHIP8_CPU->memory[CHIP8_CPU->index] = value % 10;
}

// FX55 - LD [I], Vx: 将从V0到Vx的所有寄存器的值，依次存储到从I开始的内存中
void oc_fx55(void){
    // 从操作码中提取寄存器索引x
	uint8_t Vx = (CHIP8_CPU->opcode & 0x0F00u) >> 8u;

    // 循环将V0到Vx的寄存器值存入内存
	for (uint8_t i = 0; i <= Vx; ++i)
	{
		CHIP8_CPU->memory[CHIP8_CPU->index + i] = CHIP8_CPU->registers[i];
	}
}

// FX65 - LD Vx, [I]: 从I开始的内存中读取数据，依次加载到从V0到Vx的所有寄存器中
void oc_fx65(void){
    // 从操作码中提取寄存器索引x
	uint8_t Vx = (CHIP8_CPU->opcode & 0x0F00u) >> 8u;

    // 循环从内存中读取数据并存入V0到Vx的寄存器
	for (uint8_t i = 0; i <= Vx; ++i)
	{
		CHIP8_CPU->registers[i] = CHIP8_CPU->memory[CHIP8_CPU->index + i];
	}
}
//默认未知指令处理
void oc_null(void) 
{
    printf("[STATE][OPCODE] Unknown :(\n");
}

//EX9E - SKP Vx: 如果Vx对应的键被按下，则跳过下一条指令
void oc_ex9e(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    if (CHIP8_CPU->keypad[CHIP8_CPU->registers[x]]) {
        CHIP8_CPU->pc += 4;
    } else {
        CHIP8_CPU->pc += 2;
    }
}

//EXA1 - SKNP Vx: 如果Vx对应的键没有被按下，则跳过下一条指令
void oc_exa1(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    if (!CHIP8_CPU->keypad[CHIP8_CPU->registers[x]]) {
        CHIP8_CPU->pc += 4;
    } else {
        CHIP8_CPU->pc += 2;
    }
}

//FX07 - LD Vx, DT: 将延迟定时器的值存入Vx
void oc_fx07(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->registers[x] = CHIP8_CPU->delayTimer;
    CHIP8_CPU->pc += 2;
}

//FX0A - LD Vx, K: 等待按键，将按键值存入Vx
void oc_fx0a(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    int key_pressed = 0;
    for (int i = 0; i < 16; i++) {
        if (CHIP8_CPU->keypad[i]) {
            CHIP8_CPU->registers[x] = i;
            key_pressed = 1;
            break;
        }
    }
    if (!key_pressed) {
        CHIP8_CPU->pc -= 2;
    } else {
        CHIP8_CPU->pc += 2;
    }
}

//FX15 - LD DT, Vx: 将Vx的值存入延迟定时器
void oc_fx15(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->delayTimer = CHIP8_CPU->registers[x];
    CHIP8_CPU->pc += 2;
}

//FX18 - LD ST, Vx: 将Vx的值存入声音定时器
void oc_fx18(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->soundTimer = CHIP8_CPU->registers[x];
    CHIP8_CPU->pc += 2;
}

//FX1E - ADD I, Vx: I = I + Vx
void oc_fx1e(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->index += CHIP8_CPU->registers[x];
    CHIP8_CPU->pc += 2;
}

//FX29 - LD F, Vx: 设置I为数字Vx的字体位置
void oc_fx29(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    CHIP8_CPU->index = CHIP8_CPU->registers[x] * 5;
    CHIP8_CPU->pc += 2;
}

//FX33 - LD B, Vx: 将Vx的BCD表示存入内存I, I+1, I+2
void oc_fx33(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    uint8_t value = CHIP8_CPU->registers[x];
    CHIP8_CPU->memory[CHIP8_CPU->index] = value / 100;
    CHIP8_CPU->memory[CHIP8_CPU->index + 1] = (value / 10) % 10;
    CHIP8_CPU->memory[CHIP8_CPU->index + 2] = value % 10;
    CHIP8_CPU->pc += 2;
}

//FX55 - LD [I], Vx: 将寄存器V0到Vx的值存入内存I开始的位置
void oc_fx55(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    for (uint8_t i = 0; i <= x; i++) {
        CHIP8_CPU->memory[CHIP8_CPU->index + i] = CHIP8_CPU->registers[i];
    }
    CHIP8_CPU->index += x + 1;
    CHIP8_CPU->pc += 2;
}

//FX65 - LD Vx, [I]: 将内存I开始位置的值存入寄存器V0到Vx
void oc_fx65(void) {
    uint8_t x = (CHIP8_CPU->opcode & 0x0F00) >> 8;
    for (uint8_t i = 0; i <= x; i++) {
        CHIP8_CPU->registers[i] = CHIP8_CPU->memory[CHIP8_CPU->index + i];
    }
    CHIP8_CPU->index += x + 1;
    CHIP8_CPU->pc += 2;
}




