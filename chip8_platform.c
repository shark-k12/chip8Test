#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "chip8_platform.h"
#include "chip8_cpu.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_AudioDeviceID audio_device = 0;

static SDL_Keycode keymap[KEY_SZ] = {
    SDLK_1, SDLK_2, SDLK_3, SDLK_4,
    SDLK_q, SDLK_w, SDLK_e, SDLK_r,
    SDLK_a, SDLK_s, SDLK_d, SDLK_f,
    SDLK_z, SDLK_x, SDLK_c, SDLK_v
};

char is_running = 1;

void display_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL初始化失败: %s\n", SDL_GetError());
        exit(1);
    }

    window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCR_W * SCALE,
        SCR_H * SCALE,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        fprintf(stderr, "窗口创建失败: %s\n", SDL_GetError());
        exit(1);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "渲染器创建失败: %s\n", SDL_GetError());
        exit(1);
    }

    // 显式设置渲染器默认颜色为纯黑色（RGBA：0,0,0,255）
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        SCR_W,
        SCR_H
    );

    if (!texture) {
        fprintf(stderr, "纹理创建失败: %s\n", SDL_GetError());
        exit(1);
    }
}

// chip8_platform.c 的 display_update 函数
void display_update(void)
{
    void *pixels;
    int pitch;

    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    Uint32 *pixel_data = (Uint32 *)pixels;
    // 强制背景为黑色，前景为白色
    for (int i = 0; i < SCR_SZ; i++) {
        pixel_data[i] = CHIP8_CPU->video[i] ? FG_COLOR : 0xFF000000; // 直接写死黑色，避免宏定义问题
    }

    SDL_UnlockTexture(texture);

    // 清屏用黑色
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

void display_destroy(void)
{
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}

static void audio_callback(void *userdata, Uint8 *stream, int len)
{
    static int phase = 0;
    Sint16 *buffer = (Sint16 *)stream;
    int samples = len / 2;
    // 调整为440Hz方波（标准CHIP-8蜂鸣频率）
    int freq = 440;
    int period = AUDIO_SAMPLING_RATE / freq;

    for (int i = 0; i < samples; i++) {
        buffer[i] = (Sint16)(AUDIO_AMPLITUDE * ((phase / period) % 2 ? 1 : -1));
        phase++;
    }
}

void audio_init(void)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL音频初始化失败: %s\n", SDL_GetError());
        return;
    }

    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = AUDIO_SAMPLING_RATE;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 512;
    spec.callback = audio_callback;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
    if (audio_device == 0) {
        fprintf(stderr, "音频设备打开失败: %s\n", SDL_GetError());
    }
}

void audio_beep(void)
{
    if (audio_device != 0) {
        SDL_PauseAudioDevice(audio_device, 0);
    }
}

void audio_stop(void)
{
    if (audio_device != 0) {
        SDL_PauseAudioDevice(audio_device, 1);
    }
}

void audio_destroy(void)
{
    if (audio_device != 0) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
}

void input_detect(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            is_running = 0;
        }
        else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                is_running = 0;
            }

            for (int i = 0; i < KEY_SZ; i++) {
                if (event.key.keysym.sym == keymap[i]) {
                    CHIP8_CPU->keypad[i] = 1;
                }
            }
        }
        else if (event.type == SDL_KEYUP) {
            for (int i = 0; i < KEY_SZ; i++) {
                if (event.key.keysym.sym == keymap[i]) {
                    CHIP8_CPU->keypad[i] = 0;
                }
            }
        }
    }
}