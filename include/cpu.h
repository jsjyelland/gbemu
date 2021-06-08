#ifndef CPU_H
#define CPU_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <gb_memory.h>
#include <gb.h>

#define OPCODE_DEBUG 0
#define OPCODE_BIOS_DEBUG 0

/**
 * Initialise the CPU by setting the registers to 0
 * and the SP to 0xFFFE
 */
void cpu_init(gb_t *gb);

/**
 * Read, decode, execute loop
 */
void cpu_tick(gb_t *gb);

#endif