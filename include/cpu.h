#ifndef CPU_H
#define CPU_H

#define CPU_TICK_PASS 0
#define CPU_TICK_ERROR 1

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <gb_memory.h>
#include <gb.h>

/**
 * Initialise the CPU by setting the registers to 0
 * and the SP to 0xFFFE
 */
void cpu_init(gb_t *gb);

/**
 * Read, decode, execute loop
 */
int cpu_tick(gb_t *gb);

#endif