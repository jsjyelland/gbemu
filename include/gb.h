#ifndef GB_H
#define GB_H

#include <stdint.h>
#include <stdlib.h>

#define REG_P1 0xFF00

#define REG_DIV 0xFF04
#define REG_TIMA 0xFF05
#define REG_TMA 0xFF06
#define REG_TMC 0xFF07

#define TMC_ENABLE (1 << 2)
#define TMC_CLOCK_SELECT 0b11

#define TMC_CLOCK_DIV_1024 0b00
#define TMC_CLOCK_DIV_16 0b01
#define TMC_CLOCK_DIV_64 0b10
#define TMC_CLOCK_DIV_256 0b11

#define REG_LCDC 0xFF40

#define LCDC_BG_WINDOW_DISPLAY (1)
#define LCDC_OBJ_ON (1 << 1)
#define LCDC_OBJ_BLOCK_COMPOSITION (1 << 2)
#define LCDC_BG_TILE_MAP_DISPLAY_SELECT (1 << 3)
#define LCDC_BG_WINDOW_TILE_DATA_SELECT (1 << 4)
#define LCDC_WINDOW_ON (1 << 5)
#define LCDC_WINDOW_TILE_MAP_DISPLAY_SELECT (1 << 6)
#define LCDC_LCD_CONTROL (1 << 7)

#define REG_STAT 0xFF41

#define STAT_MODE (1 | (1 << 1))
#define STAT_MATCH (1 << 2)
#define STAT_INTERRUPT_HBLANK (1 << 3)
#define STAT_INTERRUPT_VBLANK (1 << 4)
#define STAT_INTERRUPT_OAM (1 << 5)
#define STAT_INTERRUPT_LCY (1 << 6)

#define REG_SCY 0xFF42
#define REG_SCX 0xFF43
#define REG_LY 0xFF44
#define REG_LYC 0xFF45
#define REG_DMA 0xFF46
#define REG_BGP 0xFF47
#define REG_OBP0 0xFF48
#define REG_OBP1 0xFF49
#define REG_WY 0xFF4A
#define REG_WX 0xFF4B

#define INTERRUPT_ENABLE 0xFFFF
#define INTERRUPT_FLAGS 0xFF0F

#define INT_FLAG_VBLANK (1)
#define INT_FLAG_LCD (1 << 1)
#define INT_FLAG_TIMER (1 << 2)
#define INT_FLAG_SERIAL (1 << 3)
#define INT_FLAG_JOYPAD (1 << 4)

#define DMA_MODE_REQUESTED 0
#define DMA_MODE_TRANSFER 1
#define DMA_MODE_NONE 2

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
    uint8_t ime;

    // CPU
    gb_cpu_core_t cpu;
    
    // Memory
    uint8_t *rom;
    uint8_t *vram;
    uint8_t *ram;
    uint8_t *oam;
    uint8_t *io_registers;
    uint8_t *hram;

    // MBC
    uint8_t *mbc_rom;
    uint8_t *mbc_ram;

    uint8_t current_rom_bank;
    uint8_t current_ram_bank;

    uint8_t rom_ram_mode;

    // DMA
    uint8_t dma_mode;
    uint8_t dma_cycles;
    uint16_t dma_addr;
} gb_t;

/**
 * Initialises and returns the global gb instance
 */
gb_t* get_gb_instance();

#endif