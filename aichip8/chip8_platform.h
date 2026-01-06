#ifndef CHIP8_PLATFORM_H
#define CHIP8_PLATFORM_H

#include "chip8_cpu.h"
#include <SDL2/SDL.h>

extern int speed_percent;

// 配置模式枚举
typedef enum {
    CONFIG_MODE_NONE,
    CONFIG_MODE_WAITING_KEY,
    CONFIG_MODE_EDITING
} ConfigMode;

// 按键映射结构体
typedef struct {
    uint8_t chip8_key;          // CHIP-8按键 (0-F)
    SDL_Keycode physical_key;   // 物理键盘按键
    char key_name[32];          // 按键名称
} KeyMapping;

// 核心平台函数
void platform_init(void);
void platform_cleanup(void);
void platform_draw(const Chip8* cpu);
int platform_handle_input(Chip8* cpu);
void platform_beep(void);
void platform_draw_speed(const char* text);
int platform_should_quit(void);

// 按键配置相关函数
void platform_load_key_mappings(const char* filename);
void platform_save_key_mappings(const char* filename);
void platform_reset_key_mappings(void);
void platform_toggle_config_mode(void);
void platform_draw_virtual_keyboard(void);

#endif