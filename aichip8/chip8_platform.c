#include "chip8_platform.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <windows.h>
#include <SDL2/SDL_ttf.h>

extern int speed_percent;

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static int quit_flag = 0;

static const int SCREEN_SCALE = 10;
static const int WINDOW_WIDTH = SCREEN_WIDTH * SCREEN_SCALE;
static const int WINDOW_HEIGHT = SCREEN_HEIGHT * SCREEN_SCALE;

// 添加字体相关静态变量
static SDL_Texture* speed_texture = NULL;
static TTF_Font* font = NULL;

static const uint8_t keymap[KEY_COUNT] = {
    SDLK_0, SDLK_1, SDLK_2, SDLK_3,
    SDLK_4, SDLK_5, SDLK_6, SDLK_7,
    SDLK_8, SDLK_9, SDLK_a, SDLK_b,
    SDLK_c, SDLK_d, SDLK_e, SDLK_f
};

void platform_init(void) {
    printf("Initializing SDL...\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        exit(1);
    }
    printf("SDL initialized successfully.\n");
    
    printf("Creating window...\n");
    window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );
    
    // Enable file dropping
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }
    printf("Window created successfully.\n");
    
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
                                                     // 初始化TTF库
    if (TTF_Init() == -1) {
        printf("TTF initialization failed: %s\n", TTF_GetError());
        exit(1);
    }else {
        // 加载字体，设置字体大小为16
        font = TTF_OpenFont("arial.ttf", 16);
        if (!font) {
            printf("Failed to load font: %s\n", TTF_GetError());
            exit(1);
        }
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
                                              // 清理TTF资源
    if (speed_texture) {
        SDL_DestroyTexture(speed_texture);
    }
    if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}

// 速度显示函数实现
void platform_draw_speed(const char* text){
    if (!font || !text) return;
    
    // 创建文字表面
    SDL_Color color = {255, 255, 255, 255};  // 白色文字
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    if (!surface) {
        printf("Failed to create text surface: %s\n", TTF_GetError());
        return;
    }
    
    // 创建纹理
    if (speed_texture) {
        SDL_DestroyTexture(speed_texture);
    }
    speed_texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    
    if (!speed_texture) {
        printf("Failed to create text texture: %s\n", SDL_GetError());
        return;
    }
    
    // 绘制到窗口左上角
    SDL_Rect dest_rect = {10, 10, 0, 0};
    SDL_QueryTexture(speed_texture, NULL, NULL, &dest_rect.w, &dest_rect.h);
    SDL_RenderCopy(renderer, speed_texture, NULL, &dest_rect);
    SDL_RenderPresent(renderer);
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

int platform_handle_input(Chip8* cpu) {
    SDL_Event event;
    int rom_dropped = 0;
    
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
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                quit_flag = 1;
            }
            // 速度调节
            if (event.key.keysym.sym == SDLK_EQUALS && (event.key.keysym.mod & KMOD_LSHIFT)) {
                if (speed_percent < 200) speed_percent += 10;
            }
            if (event.key.keysym.sym == SDLK_MINUS) {
                if (speed_percent > 50) speed_percent -= 10;
            }
        }
        
        if (event.type == SDL_KEYUP) {
            for (int i = 0; i < KEY_COUNT; i++) {
                if (event.key.keysym.sym == keymap[i]) {
                    cpu->keypad[i] = 0;
                }
            }
        }
        
        if (event.type == SDL_DROPFILE) {
            char* file_path = event.drop.file;
            printf("Loading ROM: %s\n", file_path);
            
            // Load the ROM into the CPU
            chip8_load_rom(cpu, file_path);
            
            // Free the file path allocated by SDL
            SDL_free(file_path);
            
            rom_dropped = 1;
        }
    }
    
    return rom_dropped;
}

void platform_beep(void) {
    Beep(440, 100);
}

int platform_should_quit(void) {
    return quit_flag;
}
