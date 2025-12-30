# 编译器和平台设置
CC = gcc
PLATFORM_LIBS = -lmingw32 -lSDL2main
LDFLAGS = -lSDL2
CFLAGS = -Wall -Wextra -O2 -std=c99 # 添加 -std=c99 是个好习惯

# SDL2 库路径 (使用转义的反斜杠以提高兼容性)
SDL_DIR = E:\\SDL2-2.32.8
INC_PATH = -I"$(SDL_DIR)/x86_64-w64-mingw32/include"
LIB_PATH = -L"$(SDL_DIR)/x86_64-w64-mingw32/lib"

# 项目文件设置
SRC = main.c chip8_cpu.c chip8_opcodes.c chip8_platform.c 
OBJ = $(SRC:.c=.o)
TARGET = chip8.exe # 明确指定 .exe 后缀

# 默认目标
all: $(TARGET)

# 链接目标
$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(LIB_PATH) $(PLATFORM_LIBS) $(LDFLAGS)

# 编译目标
%.o: %.c
	$(CC) $(CFLAGS) $(INC_PATH) -c $< -o $@

# 清理目标
clean:
	rm -f $(OBJ) $(TARGET)

# 运行目标 (方便测试)
run: $(TARGET)
	./$(TARGET) roms/your_game.ch8 # 请将 "your_game.ch8" 替换为你的ROM文件名

# 伪目标声明
.PHONY: all clean run