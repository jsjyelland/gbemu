#ifndef GPU_H
#define GPU_H

#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>

#include <string.h>
#include <stdio.h>
#include <time.h>

#include <gb.h>
#include <cpu.h>
#include <joypad.h>

#define LCD_MODE_0_HBLANK 0
#define LCD_MODE_1_VBLANK 1
#define LCD_MODE_2_OAM 2
#define LCD_MODE_3_TRANSFER 3

#define DISPLAY_WIDTH 160
#define DISPLAY_HEIGHT 144

#define DISPLAY_SCALE 4

#define SPRITE_INDEX_NO_SPRITE 255

#define SPRITE_YPOS(n) ((n & 0xFF) - 16)
#define SPRITE_XPOS(n) (((n >> 8) & 0xFF) - 8)
#define SPRITE_TILENUM(n) ((n >> 16) & 0xFF)
#define SPRITE_ATTR(n) ((n >> 24) & 0xFF)

#define SPRITE_ATTR_PALETTE (1 << 4)
#define SPRITE_ATTR_XFLIP (1 << 5)
#define SPRITE_ATTR_YFLIP (1 << 6)
#define SPRITE_ATTR_OBJ_BG_PRIORITY (1 << 7)

#define OBJ_PALETTE_0 0
#define OBJ_PALETTE_1 1

int gpu_init(gb_t *gb);

int gpu_tick(gb_t *gb);

#endif