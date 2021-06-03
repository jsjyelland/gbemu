#ifndef GB_H
#define GB_H

#include <stdint.h>
#include <stdlib.h>

#define REG_P1 0xFF00
#define REG_LCDC 0xFF40
#define REG_SCY 0xFF42
#define REG_SCX 0xFF43
#define REG_BGP 0xFF47

// CPU core registers
typedef struct {
    union {
        struct {
            uint16_t af;
            uint16_t bc;
            uint16_t de;
            uint16_t hl;
        };
        struct {
            union {
                uint8_t f;
                struct {
                    uint8_t flag_blank:4;
                    uint8_t flag_c:1;
                    uint8_t flag_h:1;
                    uint8_t flag_n:1;
                    uint8_t flag_z:1;
                };
            };
            uint8_t a;
            uint8_t c, b;
            uint8_t e, d;
            uint8_t l, h;
        };
    };

    uint16_t sp;
    uint16_t pc;

    uint8_t remaining_machine_cycles;
} gb_cpu_core_t;

// Struct for holding gameboy system variables
typedef struct {
    uint8_t in_bios;
    uint8_t interrupts_enabled;

    // CPU
    gb_cpu_core_t cpu;
    
    // Memory
    
    uint8_t *rom;
    uint8_t *vram;
    uint8_t *mbc_ram;
    uint8_t *ram;
    uint8_t *oam;
    uint8_t *io_registers;
    uint8_t *hram;
} gb_t;

gb_t* get_gb_instance();

#endif