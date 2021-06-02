CC = gcc

CFLAGS = -Iinclude -W -I "C:\Program Files\mingw-w64\x86_64-8.1.0-posix-seh-rt_v6-rev0\mingw64\x86_64-w64-mingw32\include"
LDFLAGS = -lopengl32 -lglew32 -lfreeglut -lglu32

TARGET = build/gbemu.exe

SRC = $(wildcard src/*.c) $(wildcard lib/*.c)
OBJ = $(SRC:.c=.o)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(TARGET) $(OBJ) $(wildcard **/*.o)

run:
	./$(TARGET)