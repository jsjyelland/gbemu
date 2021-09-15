CC = gcc

CFLAGS = -Iinclude -W

SRC = $(wildcard src/*.c) $(wildcard lib/*.c)
OBJ = $(SRC:.c=.o)

OBJDIR = ./build
BIN = ./bin

ifeq ($(OS), Windows_NT)
	CFLAGS += -I "C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\x86_64-w64-mingw32\include"
	LDFLAGS = -lopengl32 -lglew32 -lglfw3 -lglu32 -lgdi32
	TARGET = $(BIN)/gbemu.exe
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Darwin)
		LDFLAGS = -lglfw -lGLEW -framework OpenGL
		TARGET = $(BIN)/gbemu
	endif
endif

$(OBJDIR):
	mkdir $@

$(BIN):
	mkdir $@

$(OBJDIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

all: $(TARGET) 

$(TARGET): $(OBJ) | $(BIN)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(TARGET) $(OBJ) $(wildcard **/*.o) $(BIN)

run: $(TARGET)
	$(TARGET) $(args)