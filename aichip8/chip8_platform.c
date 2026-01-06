#include "chip8_platform.h"
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>

extern int speed_percent;

// SDL相关静态变量
static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* texture = NULL;
static int quit_flag = 0;

// 窗口缩放配置
static const int SCREEN_SCALE = 10;
static const int WINDOW_WIDTH = SCREEN_WIDTH * SCREEN_SCALE;
static const int WINDOW_HEIGHT = SCREEN_HEIGHT * SCREEN_SCALE;

// 字体和文本相关变量
static SDL_Texture* speed_texture = NULL;
static TTF_Font* font = NULL;

// 按键映射相关变量
static KeyMapping key_mappings[KEY_COUNT];  // 自定义按键映射表
static ConfigMode config_mode = CONFIG_MODE_NONE;
static int selected_chip8_key = -1;         // 当前选中的CHIP-8按键
static const char* config_filename = "chip8_keymap.cfg";

// 虚拟键盘布局参数
static const int VIRTUAL_KEY_SIZE = 40;
static const int VIRTUAL_KEY_SPACING = 10;
static const int VIRTUAL_KEYBOARD_X = WINDOW_WIDTH - (4 * VIRTUAL_KEY_SIZE + 3 * VIRTUAL_KEY_SPACING) - 20;
static const int VIRTUAL_KEYBOARD_Y = 20;

// CHIP-8按键标签
static const char* chip8_key_labels[KEY_COUNT] = {
    "0", "1", "2", "3",
    "4", "5", "6", "7",
    "8", "9", "A", "B",
    "C", "D", "E", "F"
};

// 默认按键映射
static const SDL_Keycode default_keymap[KEY_COUNT] = {
    SDLK_0, SDLK_1, SDLK_2, SDLK_3,
    SDLK_4, SDLK_5, SDLK_6, SDLK_7,
    SDLK_8, SDLK_9, SDLK_a, SDLK_b,
    SDLK_c, SDLK_d, SDLK_e, SDLK_f
};

// 获取SDL按键名称
static const char* get_sdl_key_name(SDL_Keycode key) {
    const char* name = SDL_GetKeyName(key);
    return (name && strlen(name) > 0) ? name : "UNKNOWN";
}

// 加载按键映射配置
void platform_load_key_mappings(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Config file not found, using default keymap\n");
        platform_reset_key_mappings();
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // 跳过注释和空行
        if (line[0] == '#' || line[0] == '\n') continue;

        int chip8_key;
        SDL_Keycode phys_key;
        if (sscanf(line, "KEY_%X=%d", &chip8_key, &phys_key) == 2) {
            if (chip8_key >= 0 && chip8_key < KEY_COUNT) {
                key_mappings[chip8_key].chip8_key = chip8_key;
                key_mappings[chip8_key].physical_key = phys_key;
                strncpy(key_mappings[chip8_key].key_name, 
                        get_sdl_key_name(phys_key), 
                        sizeof(key_mappings[chip8_key].key_name) - 1);
                key_mappings[chip8_key].key_name[sizeof(key_mappings[chip8_key].key_name) - 1] = '\0';
            }
        }
    }

    fclose(file);
    printf("Loaded keymap from %s\n", filename);
}

// 保存按键映射配置
void platform_save_key_mappings(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Failed to save keymap to %s\n", filename);
        return;
    }

    fprintf(file, "# CHIP-8 Keymap Configuration\n");
    fprintf(file, "# Format: KEY_<CHIP8_KEY>=<SDL_KEYCODE>\n\n");

    for (int i = 0; i < KEY_COUNT; i++) {
        fprintf(file, "KEY_%X=%d\n", i, key_mappings[i].physical_key);
    }

    fclose(file);
    printf("Saved keymap to %s\n", filename);
}

// 恢复默认按键映射
void platform_reset_key_mappings(void) {
    for (int i = 0; i < KEY_COUNT; i++) {
        key_mappings[i].chip8_key = i;
        key_mappings[i].physical_key = default_keymap[i];
        strncpy(key_mappings[i].key_name, 
                get_sdl_key_name(default_keymap[i]), 
                sizeof(key_mappings[i].key_name) - 1);
        key_mappings[i].key_name[sizeof(key_mappings[i].key_name) - 1] = '\0';
    }
    printf("Reset to default keymap\n");
}

// 切换配置模式
void platform_toggle_config_mode(void) {
    if (config_mode == CONFIG_MODE_NONE) {
        config_mode = CONFIG_MODE_EDITING;
        selected_chip8_key = -1;
        printf("Entered key config mode (Press ESC to exit, R to reset defaults, Tab to toggle)\n");
    } else {
        config_mode = CONFIG_MODE_NONE;
        selected_chip8_key = -1;
        platform_save_key_mappings(config_filename);
        printf("Exited key config mode, changes saved\n");
    }
}

// 绘制虚拟键盘
void platform_draw_virtual_keyboard(void) {
    if (config_mode == CONFIG_MODE_NONE) return;

    // 绘制键盘背景
    SDL_Rect kb_bg = {
        VIRTUAL_KEYBOARD_X - 10,
        VIRTUAL_KEYBOARD_Y - 10,
        4 * VIRTUAL_KEY_SIZE + 3 * VIRTUAL_KEY_SPACING + 20,
        4 * VIRTUAL_KEY_SIZE + 3 * VIRTUAL_KEY_SPACING + 20
    };
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer, &kb_bg);

    // 绘制每个按键
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            int index = row * 4 + col;
            int x = VIRTUAL_KEYBOARD_X + col * (VIRTUAL_KEY_SIZE + VIRTUAL_KEY_SPACING);
            int y = VIRTUAL_KEYBOARD_Y + row * (VIRTUAL_KEY_SIZE + VIRTUAL_KEY_SPACING);

            SDL_Rect key_rect = {x, y, VIRTUAL_KEY_SIZE, VIRTUAL_KEY_SIZE};
            
            // 设置按键颜色
            if (selected_chip8_key == index) {
                // 选中状态 - 高亮蓝色
                SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);
            } else if (config_mode == CONFIG_MODE_WAITING_KEY && selected_chip8_key == index) {
                // 等待按键状态 - 黄色
                SDL_SetRenderDrawColor(renderer, 255, 200, 0, 255);
            } else {
                // 普通状态 - 灰色
                SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
            }
            
            // 绘制按键背景
            SDL_RenderFillRect(renderer, &key_rect);
            
            // 绘制按键边框
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderDrawRect(renderer, &key_rect);

            // 绘制按键文本
            if (font) {
                char key_text[64];
                snprintf(key_text, sizeof(key_text), "%s\n(%s)", 
                         chip8_key_labels[index], key_mappings[index].key_name);
                
                SDL_Color text_color = {255, 255, 255, 255};
                SDL_Surface* surface = TTF_RenderText_Solid(font, key_text, text_color);
                if (surface) {
                    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, surface);
                    if (text_texture) {
                        SDL_Rect text_rect = {
                            x + 5,
                            y + 5,
                            surface->w,
                            surface->h
                        };
                        SDL_RenderCopy(renderer, text_texture, NULL, &text_rect);
                        SDL_DestroyTexture(text_texture);
                    }
                    SDL_FreeSurface(surface);
                }
            }
        }
    }

    // 绘制配置模式提示
    if (font) {
        const char* hint_text = config_mode == CONFIG_MODE_WAITING_KEY ? 
                               "Press a key to map..." : 
                               "Click virtual key to remap (ESC=exit, R=reset, Tab=toggle)";
        
        SDL_Color hint_color = {255, 255, 0, 255}; // 黄色
        SDL_Surface* hint_surface = TTF_RenderText_Solid(font, hint_text, hint_color);
        if (hint_surface) {
            SDL_Texture* hint_texture = SDL_CreateTextureFromSurface(renderer, hint_surface);
            if (hint_texture) {
                SDL_Rect hint_rect = {
                    VIRTUAL_KEYBOARD_X,
                    VIRTUAL_KEYBOARD_Y + 4 * (VIRTUAL_KEY_SIZE + VIRTUAL_KEY_SPACING) + 10,
                    hint_surface->w,
                    hint_surface->h
                };
                SDL_RenderCopy(renderer, hint_texture, NULL, &hint_rect);
                SDL_DestroyTexture(hint_texture);
            }
            SDL_FreeSurface(hint_surface);
        }
    }
}

void platform_init(void) {
    printf("Initializing SDL...\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL initialization failed: %s\n", SDL_GetError());
        exit(1);
    }
    printf("SDL initialized successfully.\n");
    
    printf("Creating window...\n");
    // 调整窗口宽度以容纳虚拟键盘
    int adjusted_width = WINDOW_WIDTH + (4 * VIRTUAL_KEY_SIZE + 3 * VIRTUAL_KEY_SPACING) + 40;
    window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        adjusted_width,
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
    } else {
        // 加载字体，尝试多个常见字体路径
        font = TTF_OpenFont("arial.ttf", 16);
        if (!font) {
            font = TTF_OpenFont("c:/windows/fonts/arial.ttf", 16);
        }
        if (!font) {
            font = TTF_OpenFont("DejaVuSans.ttf", 16);
        }
        if (!font) {
            printf("Failed to load font: %s\n", TTF_GetError());
            exit(1);
        }
    }
    
    // 加载按键映射配置
    platform_reset_key_mappings();
    platform_load_key_mappings(config_filename);
}

void platform_cleanup(void) {
    // 清理SDL纹理和渲染器
    if (texture) SDL_DestroyTexture(texture);
    if (speed_texture) SDL_DestroyTexture(speed_texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    
    // 清理字体资源
    if (font) TTF_CloseFont(font);
    
    // 退出SDL和TTF
    TTF_Quit();
    SDL_Quit();
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
}

void platform_draw(const Chip8* cpu) {
    uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    
    // 构建像素数据
    for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            pixels[y * SCREEN_WIDTH + x] = cpu->screen[x][y] ? 0xFFFFFFFF : 0xFF000000;
        }
    }
    
    // 更新纹理并渲染
    SDL_UpdateTexture(texture, NULL, pixels, SCREEN_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    
    // 绘制速度信息
    char speed_text[32];
    snprintf(speed_text, sizeof(speed_text), "Speed: %d%%", speed_percent);
    platform_draw_speed(speed_text);
    
    // 绘制虚拟键盘（配置模式下）
    platform_draw_virtual_keyboard();
    
    SDL_RenderPresent(renderer);
}

int platform_handle_input(Chip8* cpu) {
    SDL_Event event;
    int rom_dropped = 0;
    
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit_flag = 1;
        }
        
        // 配置模式处理
        if (config_mode != CONFIG_MODE_NONE) {
            // 退出配置模式
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                platform_toggle_config_mode();
                continue;
            }
            
            // 恢复默认配置
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_r) {
                platform_reset_key_mappings();
                continue;
            }
            
            // 等待按键映射
            if (config_mode == CONFIG_MODE_WAITING_KEY && event.type == SDL_KEYDOWN) {
                if (selected_chip8_key >= 0 && selected_chip8_key < KEY_COUNT) {
                    // 更新映射表
                    key_mappings[selected_chip8_key].physical_key = event.key.keysym.sym;
                    strncpy(key_mappings[selected_chip8_key].key_name,
                            get_sdl_key_name(event.key.keysym.sym),
                            sizeof(key_mappings[selected_chip8_key].key_name) - 1);
                    key_mappings[selected_chip8_key].key_name[sizeof(key_mappings[selected_chip8_key].key_name) - 1] = '\0';
                    
                    printf("Mapped CHIP-8 key %X to %s\n", 
                           selected_chip8_key, 
                           get_sdl_key_name(event.key.keysym.sym));
                }
                config_mode = CONFIG_MODE_EDITING;
                selected_chip8_key = -1;
                continue;
            }
            
            // 鼠标点击虚拟按键
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mouse_x = event.button.x;
                int mouse_y = event.button.y;
                
                // 检查是否点击了虚拟按键
                for (int row = 0; row < 4; row++) {
                    for (int col = 0; col < 4; col++) {
                        int index = row * 4 + col;
                        int key_x = VIRTUAL_KEYBOARD_X + col * (VIRTUAL_KEY_SIZE + VIRTUAL_KEY_SPACING);
                        int key_y = VIRTUAL_KEYBOARD_Y + row * (VIRTUAL_KEY_SIZE + VIRTUAL_KEY_SPACING);
                        
                        SDL_Rect key_rect = {key_x, key_y, VIRTUAL_KEY_SIZE, VIRTUAL_KEY_SIZE};
                        if (SDL_PointInRect(&(SDL_Point){mouse_x, mouse_y}, &key_rect)) {
                            selected_chip8_key = index;
                            config_mode = CONFIG_MODE_WAITING_KEY;
                            printf("Please press a physical key for CHIP-8 key %X\n", index);
                            break;
                        }
                    }
                    if (config_mode == CONFIG_MODE_WAITING_KEY) break;
                }
                continue;
            }
        }
        
        // 正常输入处理（使用自定义映射表）
        if (event.type == SDL_KEYDOWN) {
            // 切换配置模式（Tab键）
            if (event.key.keysym.sym == SDLK_TAB) {
                platform_toggle_config_mode();
                continue;
            }
            
            // 查找映射的CHIP-8按键
            for (int i = 0; i < KEY_COUNT; i++) {
                if (event.key.keysym.sym == key_mappings[i].physical_key) {
                    cpu->keypad[i] = 1;
                }
            }
            
            // 退出程序（非配置模式下按ESC）
            if (event.key.keysym.sym == SDLK_ESCAPE && config_mode == CONFIG_MODE_NONE) {
                quit_flag = 1;
            }
            
            // 速度调节功能
            if (event.key.keysym.scancode == SDL_SCANCODE_EQUALS && 
                (event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))) {
                if (speed_percent < 200) {
                    speed_percent += 10;
                    printf("Speed increased to %d%%\n", speed_percent);
                }
            }
            if (event.key.keysym.scancode == SDL_SCANCODE_MINUS) {
                if (speed_percent > 50) {
                    speed_percent -= 10;
                    printf("Speed decreased to %d%%\n", speed_percent);
                }
            }
        }
        
        // 按键松开处理
        if (event.type == SDL_KEYUP) {
            // 查找映射的CHIP-8按键
            for (int i = 0; i < KEY_COUNT; i++) {
                if (event.key.keysym.sym == key_mappings[i].physical_key) {
                    cpu->keypad[i] = 0;
                }
            }
        }
        
        // ROM文件拖放处理
        if (event.type == SDL_DROPFILE) {
            char* file_path = event.drop.file;
            printf("Loading ROM: %s\n", file_path);
            
            // 加载ROM到CPU
            chip8_load_rom(cpu, file_path);
            
            // 释放SDL分配的文件路径内存
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