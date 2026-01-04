#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "chip8_cpu.h"
#include "chip8_platform.h"

// CHIP-8标准参数
#define CHIP8_CYCLES_PER_FRAME 9    // 60FPS * 9 = 540指令/秒（标准速度）
#define CHIP8_FRAME_DELAY 16        // 60FPS对应每帧16ms左右

int main(int argc, char *argv[]) 
{
    if (argc < 2) 
    {
        fprintf(stderr, "用法: %s <rom文件路径>\n", argv[0]);
        return 1;
    }

    init();

    if (loadrom(argv[1]) != 0) 
    {
        destroy();
        return 1;
    }

    display_init();
    audio_init();

    is_running = 1;

    Uint32 last_frame_time = SDL_GetTicks();
    Uint32 timer_tick = 0; // 定时器计数（60Hz）

    while (is_running) 
    {
        input_detect();  
        // 执行指令周期
        for (int i = 0; i < CHIP8_CYCLES_PER_FRAME; i++){
            cycle();
        }     
        // 60Hz更新定时器（delay/sound）
        Uint32 current_time = SDL_GetTicks();
        if (current_time - last_frame_time >= 16){
            if (CHIP8_CPU->delayTimer > 0) CHIP8_CPU->delayTimer--;
            if (CHIP8_CPU->soundTimer > 0) CHIP8_CPU->soundTimer--;
            timer_tick++;
            last_frame_time = current_time;
        }
            
        // 强制每帧刷新显示
        display_update();
            
         // 控制帧率
        Uint32 elapsed = SDL_GetTicks() - last_frame_time;
         if (elapsed < CHIP8_FRAME_DELAY){
            SDL_Delay(CHIP8_FRAME_DELAY - elapsed);
        }
            
         // 声音
        if (CHIP8_CPU->soundTimer > 0) audio_beep();
        else audio_stop();
    }

    display_destroy();
    audio_destroy();
    destroy();
    return 0;
}