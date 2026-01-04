CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99
SDL_DIR = E:\SDL2-devel-2.32.8-mingw\SDL2-2.32.8
INCLUDES = -I"$(SDL_DIR)/x86_64-w64-mingw32/include"
LIBS = -L"$(SDL_DIR)/x86_64-w64-mingw32/lib" -lmingw32 -lSDL2main -lSDL2

SOURCES = main.c chip8_cpu.c chip8_opcodes.c chip8_platform.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = chip8.exe

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	del /Q *.o $(TARGET) 2>nul

run: $(TARGET)
	$(TARGET) $(ROM)

.PHONY: all clean run
