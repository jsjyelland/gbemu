#ifndef JOYPAD_H
#define JOYPAD_H

#include <stdio.h>
#include <stdint.h>
#include <GLFW/glfw3.h>

#include <gb.h>
#include <gb_memory.h>

#define JOYPAD_PORT_P14 0
#define JOYPAD_PORT_P15 1

void joypad_init();

void key_pressed_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

uint8_t get_joypad_mask(uint8_t port);

void joypad_update_io_registers(gb_t *gb);

#endif