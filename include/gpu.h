#ifndef GPU_H
#define GPU_H

#include <GLFW/glfw3.h>

#include <string.h>
#include <stdio.h>
#include <time.h>

#include <gb.h>
#include <cpu.h>

#define DISPLAY_WIDTH 160
#define DISPLAY_HEIGHT 144

int gpu_init(gb_t *gb);

int gpu_tick(gb_t *gb);

#endif