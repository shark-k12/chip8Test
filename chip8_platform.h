#ifndef CHIP8_PLATFORM_H
#define CHIP8_PLATFORM_H

#include "chip8_cpu.h"
#include <SDL2/SDL.h>
#include <stdbool.h>

// Key mapping structure
typedef struct {
    uint8_t chip8_key;
    SDL_Keycode sdl_key;
    const char* label;
} KeyMapping;

// Virtual key struct
typedef struct {
    int chip8_key;
    SDL_Rect rect;
    bool is_selected;
    const char* label;
} VirtualKey;

extern KeyMapping keymap[KEY_COUNT];

// Configuration mode state
typedef struct {
    bool is_configuring;
    int selected_key;
    bool is_learning;
} ConfigState;

extern ConfigState config_state;

void platform_init(void);
void platform_cleanup(void);
void platform_draw(const Chip8* cpu);
int platform_handle_input(Chip8* cpu);
void platform_beep(void);
int platform_should_quit(void);

// Key mapping functions
void platform_set_key_mapping(int chip8_key, SDL_Keycode sdl_key);
void platform_load_key_mappings(const char* filename);
void platform_save_key_mappings(const char* filename);
void platform_reset_key_mappings(void);

// Speed control
#define MIN_SPEED_PERCENT 50
#define MAX_SPEED_PERCENT 200
#define DEFAULT_SPEED_PERCENT 100

extern int game_speed_percent;

double platform_get_speed_factor(void);
void platform_increase_speed(void);
void platform_decrease_speed(void);

// Configuration functions
void platform_toggle_config_mode(void);
void platform_draw_virtual_keyboard(void);
int platform_handle_config_input(const SDL_Event* event);
const char* platform_get_key_name(SDL_Keycode key);

#endif
