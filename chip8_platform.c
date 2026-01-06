#include "chip8_platform.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static int quit_flag = 0;

static const int SCREEN_SCALE = 10;
static const int WINDOW_WIDTH = SCREEN_WIDTH * SCREEN_SCALE;
static const int WINDOW_HEIGHT = SCREEN_HEIGHT * SCREEN_SCALE;

// Virtual keyboard constants
static const int VKB_WIDTH = 320;
static const int VKB_HEIGHT = 240;
static const int VKB_KEY_SIZE = 60;
static const int VKB_KEY_SPACING = 15;
static const int VKB_X = WINDOW_WIDTH / 2 - VKB_WIDTH / 2;
static const int VKB_Y = WINDOW_HEIGHT / 2 - VKB_HEIGHT / 2;

// CHIP-8 keypad layout (4x4)
static const char* chip8_key_labels[KEY_COUNT] = {
    "1", "2", "3", "C",
    "4", "5", "6", "D",
    "7", "8", "9", "E",
    "A", "0", "B", "F"
};

// Game speed control
int game_speed_percent = DEFAULT_SPEED_PERCENT;

// Function declarations for speed display
static void draw_digit(int x, int y, int digit, int size, SDL_Color color);
static void draw_speed_display(void);

// Key mapping initialization
KeyMapping keymap[KEY_COUNT] = {
    {0x1, SDLK_1, "1"},
    {0x2, SDLK_2, "2"},
    {0x3, SDLK_3, "3"},
    {0xC, SDLK_4, "4"},
    {0x4, SDLK_q, "Q"},
    {0x5, SDLK_w, "W"},
    {0x6, SDLK_e, "E"},
    {0xD, SDLK_r, "R"},
    {0x7, SDLK_a, "A"},
    {0x8, SDLK_s, "S"},
    {0x9, SDLK_d, "D"},
    {0xE, SDLK_f, "F"},
    {0xA, SDLK_z, "Z"},
    {0x0, SDLK_x, "X"},
    {0xB, SDLK_c, "C"},
    {0xF, SDLK_v, "V"}
};

// Virtual keys array
static VirtualKey virtual_keys[KEY_COUNT];

// Configuration state
ConfigState config_state = {
    .is_configuring = false,
    .selected_key = -1,
    .is_learning = false
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
        SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI
    );
    
    // Enable file dropping
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    
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

    // Try to load custom key mappings from file
    platform_load_key_mappings("keymap.cfg");
    
    // Initialize virtual keyboard
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            int index = row * 4 + col;
            int chip8_key;
            
            // Correct CHIP-8 key mapping for 4x4 matrix
            if (row == 0) {
                // Top row: 1, 2, 3, C
                chip8_key = 0x1 + col;
                if (col == 3) chip8_key = 0xC;
            } else if (row == 1) {
                // Second row: 4, 5, 6, D
                chip8_key = 0x4 + col;
                if (col == 3) chip8_key = 0xD;
            } else if (row == 2) {
                // Third row: 7, 8, 9, E
                chip8_key = 0x7 + col;
                if (col == 3) chip8_key = 0xE;
            } else {
                // Bottom row: A, 0, B, F
                switch (col) {
                    case 0: chip8_key = 0xA; break;
                    case 1: chip8_key = 0x0; break;
                    case 2: chip8_key = 0xB; break;
                    case 3: chip8_key = 0xF; break;
                    default: chip8_key = 0x0;
                }
            }
            
            virtual_keys[index].chip8_key = chip8_key;
            virtual_keys[index].rect.x = VKB_X + col * (VKB_KEY_SIZE + VKB_KEY_SPACING);
            virtual_keys[index].rect.y = VKB_Y + row * (VKB_KEY_SIZE + VKB_KEY_SPACING);
            virtual_keys[index].rect.w = VKB_KEY_SIZE;
            virtual_keys[index].rect.h = VKB_KEY_SIZE;
            virtual_keys[index].is_selected = false;
            virtual_keys[index].label = chip8_key_labels[index];
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
}

void platform_draw(const Chip8* cpu) {
    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    // Draw CHIP-8 screen if not in configuration mode
    if (!config_state.is_configuring && cpu != NULL) {
        uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
        
        for (int y = 0; y < SCREEN_HEIGHT; y++) {
            for (int x = 0; x < SCREEN_WIDTH; x++) {
                pixels[y * SCREEN_WIDTH + x] = cpu->screen[x][y] ? 0xFFFFFFFF : 0xFF000000;
            }
        }
        
        SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
        SDL_RenderCopy(renderer, texture, NULL, NULL);
    }
    
    // Draw configuration interface if needed
    if (config_state.is_configuring) {
        platform_draw_virtual_keyboard();
    }
    
    // Draw speed display in top-right corner
    draw_speed_display();
    
    SDL_RenderPresent(renderer);
}

int platform_handle_input(Chip8* cpu) {
    SDL_Event event;
    int rom_dropped = 0;
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit_flag = 1;
        }
        
        // Toggle configuration mode with F1 key
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F1) {
            platform_toggle_config_mode();
        }
        
        // Handle configuration mode input
        if (config_state.is_configuring) {
            if (platform_handle_config_input(&event)) {
                continue; // Skip regular input if handled by config
            }
        }
        
        if (event.type == SDL_KEYDOWN) {
            for (int i = 0; i < KEY_COUNT; i++) {
                if (event.key.keysym.sym == keymap[i].sdl_key) {
                    // Find the chip8 key index
                    int chip8_key = keymap[i].chip8_key;
                    if (chip8_key < KEY_COUNT) {
                        cpu->keypad[chip8_key] = 1;
                    }
                }
            }
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                quit_flag = 1;
            }
            // Handle speed control keys
            if (event.key.keysym.sym == SDLK_PLUS || event.key.keysym.sym == SDLK_KP_PLUS || event.key.keysym.sym == SDLK_EQUALS) {
                platform_increase_speed();
            }
            if (event.key.keysym.sym == SDLK_MINUS || event.key.keysym.sym == SDLK_KP_MINUS) {
                platform_decrease_speed();
            }
        }
        
        if (event.type == SDL_KEYUP) {
            for (int i = 0; i < KEY_COUNT; i++) {
                if (event.key.keysym.sym == keymap[i].sdl_key) {
                    // Find the chip8 key index
                    int chip8_key = keymap[i].chip8_key;
                    if (chip8_key < KEY_COUNT) {
                        cpu->keypad[chip8_key] = 0;
                    }
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

// Simple 7-segment display for digits
static void draw_digit(int x, int y, int digit, int size, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    
    // Define 7-segment display segments (0-6)
    bool segments[10][7] = {
        {1, 1, 1, 1, 1, 1, 0}, // 0
        {0, 1, 1, 0, 0, 0, 0}, // 1
        {1, 1, 0, 1, 1, 0, 1}, // 2
        {1, 1, 1, 1, 0, 0, 1}, // 3
        {0, 1, 1, 0, 0, 1, 1}, // 4
        {1, 0, 1, 1, 0, 1, 1}, // 5
        {1, 0, 1, 1, 1, 1, 1}, // 6
        {1, 1, 1, 0, 0, 0, 0}, // 7
        {1, 1, 1, 1, 1, 1, 1}, // 8
        {1, 1, 1, 1, 0, 1, 1}  // 9
    };
    
    // Segment positions
    int w = size;
    int h = size;
    int t = size / 4; // Thickness
    
    // Horizontal segments
    if (segments[digit][0]) { // Top segment
        SDL_Rect seg = {x + t, y, w - 2*t, t};
        SDL_RenderFillRect(renderer, &seg);
    }
    if (segments[digit][3]) { // Middle segment
        SDL_Rect seg = {x + t, y + h/2 - t/2, w - 2*t, t};
        SDL_RenderFillRect(renderer, &seg);
    }
    if (segments[digit][6]) { // Bottom segment
        SDL_Rect seg = {x + t, y + h - t, w - 2*t, t};
        SDL_RenderFillRect(renderer, &seg);
    }
    
    // Vertical segments
    if (segments[digit][1]) { // Top-right
        SDL_Rect seg = {x + w - t, y + t, t, h/2 - t};
        SDL_RenderFillRect(renderer, &seg);
    }
    if (segments[digit][2]) { // Bottom-right
        SDL_Rect seg = {x + w - t, y + h/2, t, h/2 - t};
        SDL_RenderFillRect(renderer, &seg);
    }
    if (segments[digit][4]) { // Top-left
        SDL_Rect seg = {x, y + t, t, h/2 - t};
        SDL_RenderFillRect(renderer, &seg);
    }
    if (segments[digit][5]) { // Bottom-left
        SDL_Rect seg = {x, y + h/2, t, h/2 - t};
        SDL_RenderFillRect(renderer, &seg);
    }
}

// Draw speed percentage in top-right corner
static void draw_speed_display(void) {
    int speed = game_speed_percent;
    SDL_Color color = {255, 255, 255, 255};
    
    int x = WINDOW_WIDTH - 120;
    int y = 20;
    int size = 20;
    int spacing = 5;
    
    // Draw digits
    int hundreds = speed / 100;
    int tens = (speed / 10) % 10;
    int units = speed % 10;
    
    if (hundreds > 0) {
        draw_digit(x, y, hundreds, size, color);
        x += size + spacing;
    }
    
    draw_digit(x, y, tens, size, color);
    x += size + spacing;
    
    draw_digit(x, y, units, size, color);
    x += size + spacing;
    
    // Draw percentage sign
    int percent_size = size / 2;
    SDL_Rect percent_rect = {x, y + size - percent_size, percent_size, percent_size};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &percent_rect);
}

int platform_should_quit(void) {
    return quit_flag;
}

// Set a specific key mapping
void platform_set_key_mapping(int chip8_key, SDL_Keycode sdl_key) {
    for (int i = 0; i < KEY_COUNT; i++) {
        if (keymap[i].chip8_key == chip8_key) {
            keymap[i].sdl_key = sdl_key;
            // Update label to show current mapping
            keymap[i].label = platform_get_key_name(sdl_key);
            printf("Mapped CHIP-8 key %X to SDL key %s\n", chip8_key, platform_get_key_name(sdl_key));
            break;
        }
    }
}

// Load key mappings from a configuration file
void platform_load_key_mappings(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Could not open key mappings file: %s\n", filename);
        return;
    }

    int chip8_key;
    SDL_Keycode sdl_key;
    int loaded = 0;

    while (fscanf(file, "%x %d", &chip8_key, &sdl_key) == 2) {
        platform_set_key_mapping(chip8_key, sdl_key);
        loaded++;
    }

    fclose(file);
    printf("Loaded %d key mappings from %s\n", loaded, filename);
}

// Save current key mappings to a configuration file
void platform_save_key_mappings(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Could not create key mappings file: %s\n", filename);
        return;
    }

    for (int i = 0; i < KEY_COUNT; i++) {
        fprintf(file, "%X %d\n", keymap[i].chip8_key, keymap[i].sdl_key);
    }

    fclose(file);
    printf("Saved key mappings to %s\n", filename);
}

// Reset key mappings to default values
void platform_reset_key_mappings(void) {
    // Default mapping (QWERTY layout)
    KeyMapping default_keymap[KEY_COUNT] = {
        {0x1, SDLK_1, "1"},
        {0x2, SDLK_2, "2"},
        {0x3, SDLK_3, "3"},
        {0xC, SDLK_4, "4"},
        {0x4, SDLK_q, "Q"},
        {0x5, SDLK_w, "W"},
        {0x6, SDLK_e, "E"},
        {0xD, SDLK_r, "R"},
        {0x7, SDLK_a, "A"},
        {0x8, SDLK_s, "S"},
        {0x9, SDLK_d, "D"},
        {0xE, SDLK_f, "F"},
        {0xA, SDLK_z, "Z"},
        {0x0, SDLK_x, "X"},
        {0xB, SDLK_c, "C"},
        {0xF, SDLK_v, "V"}
    };

    memcpy(keymap, default_keymap, sizeof(default_keymap));
    printf("Key mappings reset to default\n");
}

// Toggle configuration mode
void platform_toggle_config_mode(void) {
    config_state.is_configuring = !config_state.is_configuring;
    config_state.selected_key = -1;
    config_state.is_learning = false;
    
    if (config_state.is_configuring) {
        printf("Entered key mapping configuration mode\n");
        printf("- Click on a virtual key to select it\n");
        printf("- Press any key to map it\n");
        printf("- Press R to reset to default mappings\n");
        printf("- Press F1 to exit configuration mode\n");
    } else {
        printf("Exited configuration mode\n");
        // Save mappings when exiting configuration mode
        platform_save_key_mappings("keymap.cfg");
    }
}

// Draw virtual keyboard for configuration
void platform_draw_virtual_keyboard(void) {
    // Calculate actual keyboard area dimensions (4 keys wide x 4 keys tall)
    int actual_keyboard_width = 4 * (VKB_KEY_SIZE + VKB_KEY_SPACING) - VKB_KEY_SPACING;
    int actual_keyboard_height = 4 * (VKB_KEY_SIZE + VKB_KEY_SPACING) - VKB_KEY_SPACING;
    
    // Calculate centered position for background panel
    int panel_x = VKB_X - 20;
    int panel_y = VKB_Y - 20;
    int panel_width = actual_keyboard_width + 40;
    int panel_height = actual_keyboard_height + 40;
    
    // Draw background panel - use bright color to ensure visibility
    SDL_SetRenderDrawColor(renderer, 0, 128, 255, 255);
    SDL_Rect panel_rect = {panel_x, panel_y, panel_width, panel_height};
    SDL_RenderFillRect(renderer, &panel_rect);
    
    // Draw title area
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    // Draw virtual keys
    for (int i = 0; i < KEY_COUNT; i++) {
        VirtualKey* vkey = &virtual_keys[i];
        
        // Determine color based on selection state
        if (config_state.selected_key == vkey->chip8_key) {
            if (config_state.is_learning) {
                // Learning state - bright red
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            } else {
                // Selected state - bright yellow
                SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
            }
        } else {
            // Normal state - bright gray
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        }
        
        // Draw key background
        SDL_RenderFillRect(renderer, &vkey->rect);
        
        // Draw key border - bright white
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &vkey->rect);
        
        // Draw simple key label (larger square for better visibility)
        SDL_Rect label_rect = {
            vkey->rect.x + vkey->rect.w / 3,
            vkey->rect.y + vkey->rect.h / 3,
            20,
            20
        };
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &label_rect);
    }
    
    // Draw instructions
    if (config_state.is_learning) {
        // Calculate actual keyboard area dimensions again
        int actual_keyboard_width = 4 * (VKB_KEY_SIZE + VKB_KEY_SPACING) - VKB_KEY_SPACING;
        
        // Draw "Press any key" message area - align with keyboard width
        SDL_Rect message_rect = {
            VKB_X - 10,
            VKB_Y - 30,
            actual_keyboard_width + 20,
            40
        };
        SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &message_rect);
    }
}

// Handle input during configuration mode
int platform_handle_config_input(const SDL_Event* event) {
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_F1) {
            return 0; // Let the main handler handle this
        }
        
        // Check if Ctrl key is pressed (without any other keys)
        if ((event->key.keysym.sym == SDLK_LCTRL || event->key.keysym.sym == SDLK_RCTRL) && 
            (event->key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL))) {
            // Reset to default mappings when Ctrl key is pressed
            platform_reset_key_mappings();
            return 1;
        }
        
        if (config_state.is_learning) {
            // Learning mode - assign the pressed key
            platform_set_key_mapping(config_state.selected_key, event->key.keysym.sym);
            config_state.is_learning = false;
            return 1;
        }
    }
    
    if (event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        int mouse_x = event->button.x;
        int mouse_y = event->button.y;
        
        // Check if any virtual key was clicked
        for (int i = 0; i < KEY_COUNT; i++) {
            VirtualKey* vkey = &virtual_keys[i];
            if (mouse_x >= vkey->rect.x && mouse_x <= vkey->rect.x + vkey->rect.w &&
                mouse_y >= vkey->rect.y && mouse_y <= vkey->rect.y + vkey->rect.h) {
                
                config_state.selected_key = vkey->chip8_key;
                config_state.is_learning = true;
                printf("Select CHIP-8 key %X. Press any key to map it...\n", vkey->chip8_key);
                return 1;
            }
        }
    }
    
    return 0; // Event not handled by config mode
}

// Get human-readable key name from SDL_Keycode
const char* platform_get_key_name(SDL_Keycode key) {
    // Simple key name mapping
    static char key_name[16];
    
    switch (key) {
        case SDLK_0: return "0";
        case SDLK_1: return "1";
        case SDLK_2: return "2";
        case SDLK_3: return "3";
        case SDLK_4: return "4";
        case SDLK_5: return "5";
        case SDLK_6: return "6";
        case SDLK_7: return "7";
        case SDLK_8: return "8";
        case SDLK_9: return "9";
        case SDLK_q: return "Q";
        case SDLK_w: return "W";
        case SDLK_e: return "E";
        case SDLK_r: return "R";
        case SDLK_t: return "T";
        case SDLK_y: return "Y";
        case SDLK_u: return "U";
        case SDLK_i: return "I";
        case SDLK_o: return "O";
        case SDLK_p: return "P";
        case SDLK_a: return "A";
        case SDLK_s: return "S";
        case SDLK_d: return "D";
        case SDLK_f: return "F";
        case SDLK_g: return "G";
        case SDLK_h: return "H";
        case SDLK_j: return "J";
        case SDLK_k: return "K";
        case SDLK_l: return "L";
        case SDLK_z: return "Z";
        case SDLK_x: return "X";
        case SDLK_c: return "C";
        case SDLK_v: return "V";
        case SDLK_b: return "B";
        case SDLK_n: return "N";
        case SDLK_m: return "M";
        default:
            sprintf(key_name, "%d", key);
            return key_name;
    }
}

// Get current speed factor as a double
double platform_get_speed_factor(void) {
    return (double)game_speed_percent / 100.0;
}

// Increase game speed by 10%
void platform_increase_speed(void) {
    if (game_speed_percent < MAX_SPEED_PERCENT) {
        game_speed_percent += 10;
        printf("Game speed increased to %d%%\n", game_speed_percent);
    }
}

// Decrease game speed by 10%
void platform_decrease_speed(void) {
    if (game_speed_percent > MIN_SPEED_PERCENT) {
        game_speed_percent -= 10;
        printf("Game speed decreased to %d%%\n", game_speed_percent);
    }
}
