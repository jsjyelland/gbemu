#ifndef GB_H
#define GB_H

#include <stdint.h>
#include <stdlib.h>

// Struct for holding gameboy system variables
typedef struct {
    uint8_t in_bios;
    uint8_t interrupts_enabled;

    
    // Memory
    uint8_t *io_registers;
    uint8_t *rom;
    uint8_t *vram;
    uint8_t *mbc_ram;
    uint8_t *ram;
    uint8_t *oam;
    uint8_t *high_speed_ram;
} gb_t;

gb_t* get_gb_instance();

#endif