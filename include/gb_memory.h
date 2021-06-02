#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cpu.h>
#include <gb.h>

typedef uint8_t mem_read_function_t(uint16_t address);
typedef void mem_write_function_t(uint16_t address, uint8_t value);

/**
 * Initialise the memory
 */
void mem_init();

/**
 * Remove bios from memory map
 */
void mem_remove_bios();

/**
 * Read a byte from memory at address.
 */
uint8_t mem_read_byte(uint16_t address);

/**
 * Write a byte to memory at address
 */
void mem_write_byte(uint16_t address, uint8_t val);

/**
 * Read a 16-bit value from memory at address
 */
uint16_t mem_read_16(uint16_t address);

/**
 * Write a 16-bit value to memory at address
 */
void mem_write_16(uint16_t address, uint16_t val);

/**
 * Load a ROM file into the memory
 */
void mem_load_rom(const char *fname);

#endif