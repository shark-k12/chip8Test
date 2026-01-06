#define SDL_MAIN_HANDLED
#include "chip8_cpu.h"
#include "chip8_platform.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MIN_SPEED 50
#define MAX_SPEED 200
#define SPEED_STEP 10

#define CYCLES_PER_FRAME 8
#define FRAME_RATE 60
#define FRAME_DELAY (1000 / FRAME_RATE)

int speed_percent = 100;  // 全局速度变量

int main(int argc, char* argv[]) {
    printf("Starting CHIP-8 emulator...\n");
    Chip8 cpu;
    chip8_init(&cpu);
    int rom_loaded = 0;
    
    platform_init();

    char speed_text[20];  // 用于显示速度的文本缓冲区
    
    if (argc == 2) {
        chip8_load_rom(&cpu, argv[1]);
        rom_loaded = 1;
        printf("Starting CHIP-8 emulation with %s...\n", argv[1]);
    } else {
        printf("CHIP-8 Emulator started.\n");
        printf("Drag and drop a ROM file into the window to load.\n");
    }
    
    printf("Press ESC to quit\n");
    
    while (!platform_should_quit()) {
    clock_t start_time = clock();
    
    if (rom_loaded) {
        // 根据当前速度计算每帧执行的指令数（基准10条/帧）
        int adjusted_cycles = (CYCLES_PER_FRAME * speed_percent) / 100;
        for (int i = 0; i < adjusted_cycles; i++) {
            chip8_cycle(&cpu);
        }
        
        // 定时器调节：根据速度动态调整更新间隔（基准60Hz）
        static Uint32 last_timer_update = 0;
        Uint32 current_time = SDL_GetTicks();
        Uint32 timer_interval = (1000 / 60) * 100 / speed_percent;  // 速度越快，间隔越小
        if (current_time - last_timer_update >= timer_interval) {
            chip8_decrement_timers(&cpu);
            last_timer_update = current_time;
        }
    }
    
    // 处理输入和文件拖放
    if (platform_handle_input(&cpu)) {
        rom_loaded = 1;
        printf("Starting CHIP-8 emulation...\n");
    }
    
    if (rom_loaded && cpu.draw_flag) {
        platform_draw(&cpu);
        // 绘制速度显示（需要在platform_draw后添加文字渲染）
        sprintf(speed_text, "Speed: %d%%", speed_percent);
        platform_draw_speed(speed_text);
        cpu.draw_flag = false;
    }
    
    // 帧速率控制：根据速度调整延迟时间
    clock_t end_time = clock();
    double elapsed = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000;
    double adjusted_delay = FRAME_DELAY * 100.0 / speed_percent;  // 速度越快，延迟越小
    
    if (elapsed < adjusted_delay) {
        SDL_Delay((Uint32)(adjusted_delay - elapsed));
    }
}
    
    platform_cleanup();
    printf("Emulation stopped.\n");
    
    return 0;
}
