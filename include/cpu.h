#ifndef CPU_H
#define CPU_H

#define TICK_PASS 0
#define TICK_FAIL 1

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <gb_memory.h>
#include <gb.h>

/**
 * Initialise the cpu
 */
void cpu_init();

/**
 * Read, decode, execute loop
 */
int tick();

/**
 * Return the program counter
 */
uint16_t get_program_counter();

/**
 * Return the number of t-cycles
 */
uint32_t get_ticks();

#endif