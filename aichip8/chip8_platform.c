#include "chip8_platform.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <windows.h>

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static int quit_flag = 0;

static const int SCREEN_SCALE = 10;
static const int WINDOW_WIDTH = SCREEN_WIDTH * SCREEN_SCALE;
static const int WINDOW_HEIGHT = SCREEN_HEIGHT * SCREEN_SCALE;

static const uint8_t keymap[KEY_COUNT] = {
    SDLK_0, SDLK_1, SDLK_2, SDLK_3,
    SDLK_4, SDLK_5, SDLK_6, SDLK_7,
    SDLK_8, SDLK_9, SDLK_a, SDLK_b,
    SDLK_c, SDLK_d, SDLK_e, SDLK_f
};

void platform_init(void) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        exit(1);
    }
    
    window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer creation failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
    
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );
    
    if (!texture) {
        printf("Texture creation failed: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }
}

void platform_cleanup(void) {
    if (texture) {
        SDL_DestroyTexture(texture);
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void platform_draw(const Chip8* cpu) {
    uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            pixels[y * SCREEN_WIDTH + x] = cpu->screen[x][y] ? 0xFFFFFFFF : 0xFF000000;
        }
    }
    
    SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void platform_handle_input(Chip8* cpu) {
    SDL_Event event;
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit_flag = 1;
        }
        
        if (event.type == SDL_KEYDOWN) {
            for (int i = 0; i < KEY_COUNT; i++) {
                if (event.key.keysym.sym == keymap[i]) {
                    cpu->keypad[i] = 1;
                }
            }
        }
        
        if (event.type == SDL_KEYUP) {
            for (int i = 0; i < KEY_COUNT; i++) {
                if (event.key.keysym.sym == keymap[i]) {
                    cpu->keypad[i] = 0;
                }
            }
        }
    }
}

void platform_beep(void) {
    Beep(440, 100);
}

int platform_should_quit(void) {
    return quit_flag;
}
