CC = gcc
CFLAGS = -Wall -Wextra -O2 -I"E:\SDL2-devel-2.32.8-mingw\SDL2-2.32.8\x86_64-w64-mingw32\include" -I"E:\SDL2_ttf-2.24.0\x86_64-w64-mingw32\include"
LDFLAGS = -L"E:\SDL2-devel-2.32.8-mingw\SDL2-2.32.8\x86_64-w64-mingw32\lib" -L"E:\SDL2_ttf-2.24.0\x86_64-w64-mingw32\lib" -lSDL2 -lSDL2_ttf
TARGET = chip8_emulator.exe
SRC = main.c chip8_cpu.c chip8_opcodes.c chip8_platform.c
OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del $(OBJ) $(TARGET) 2>nul

.PHONY: all clean