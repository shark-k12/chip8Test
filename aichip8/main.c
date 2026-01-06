#define SDL_MAIN_HANDLED
#include "chip8_cpu.h"
#include "chip8_platform.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CYCLES_PER_FRAME 10
#define FRAME_RATE 60
#define FRAME_DELAY (1000 / FRAME_RATE)

int main(int argc, char* argv[]) {
    Chip8 cpu;
    chip8_init(&cpu);
    int rom_loaded = 0;
    
    platform_init();
    
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
            for (int i = 0; i < CYCLES_PER_FRAME; i++) {
                chip8_cycle(&cpu);
            }
            
            chip8_decrement_timers(&cpu);
        }
        
        // Handle input and check for dropped files
        if (platform_handle_input(&cpu)) {
            // A ROM was dropped and loaded successfully
            rom_loaded = 1;
            printf("Starting CHIP-8 emulation...\n");
        }
        
        if (rom_loaded && cpu.draw_flag) {
            platform_draw(&cpu);
            cpu.draw_flag = false;
        }
        
        clock_t end_time = clock();
        double elapsed = ((double)(end_time - start_time) / CLOCKS_PER_SEC) * 1000;
        
        if (elapsed < FRAME_DELAY) {
            SDL_Delay((Uint32)(FRAME_DELAY - elapsed));
        }
    }
    
    platform_cleanup();
    printf("Emulation stopped.\n");
    
    return 0;
}
