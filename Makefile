# 更新后的 Makefile
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lmingw32 -lSDL2main -lSDL2

# 关键修复：使用正斜杠或转义路径
SDL_DIR = /E/SDL2-2.32.8

INC_PATH = -I"$(SDL_DIR)/x86_64-w64-mingw32/include/"
LIB_PATH = -L"$(SDL_DIR)/x86_64-w64-mingw32/lib"

SRC = main.c chip8_cpu.c chip8_opcodes.c chip8_platform.c 
OBJ = $(SRC:.c=.o)
TARGET = chip8

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $^ -o $@ $(LIB_PATH) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INC_PATH) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
