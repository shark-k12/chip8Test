#include <stdio.h>
#include <string.h>

#include "chip8_cpu.h"
#include "chip8_platform.h"

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

    while (is_running) 
    {
        input_detect();
        cycle();

        display_update();

        if (CHIP8_CPU->soundTimer > 0)
        {
            audio_beep();
        }
    } 

    display_destroy();
    audio_destroy();
    destroy();
    return 0;
}
