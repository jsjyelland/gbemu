#include <gb_memory.h>

#define ROM_SIZE 0x8000
#define VRAM_SIZE 0x2000
#define MBC_RAM_SIZE 0x2000
#define RAM_SIZE 0x2000
#define OAM_SIZE 0xA0
#define IO_REGISTER_SIZE 0x80
#define HIGH_SPEED_RAM_SIZE 0x80

// BIOS code
static const uint8_t bios[256] = {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
    0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
    0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
    0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
    0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
    0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
    0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xF2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
    0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
    0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

static const uint8_t test_bios[256] = {
    0x18, 0x03, 0x04, 0x18, 0x05, 0x18, 0xFB
};

/* MEMORY READS */

/**
 * Read from the ROM bank 0
 */
static uint8_t mem_read_rom(gb_t *gb, uint16_t address) {
    // If in the bios, map addresses 0x00 to 0xFF to the bios instructions
    if (address < 0x100 && gb->in_bios) {
        #if TEST_BIOS
            return test_bios[address];
        #else
            return bios[address];
        #endif
    } else {
        return gb->rom[address];
    }
}

/**
 * Read from the switchable ROM bank on the cartridge
 */
static uint8_t mem_read_mbc_rom(gb_t *gb, uint16_t address) {
    return gb->rom[address];
}

/**
 * Read from the vram
 */
static uint8_t mem_read_vram(gb_t *gb, uint16_t address) {
    return gb->vram[address & 0x1FFF];
}

/**
 * Read from the switchable RAM bank on the cartridge
 */
static uint8_t mem_read_mbc_ram(gb_t *gb, uint16_t address) {
    return gb->mbc_ram[address & 0x1FFF];
}

/**
 * Read from the internal RAM
 */
static uint8_t mem_read_ram(gb_t *gb, uint16_t address) {
    return gb->ram[address & 0x1FFF];
}

/**
 * Read from the high RAM sections
 */
static uint8_t mem_read_hram(gb_t *gb, uint16_t address) {
    if (address < 0xFE00) {
        // Shadow copy of ram
        return mem_read_ram(gb, address);
    }

    if (address < 0xFEA0) {
        // OAM
        return gb->oam[address & 0xFF];
    }

    if (address < 0xFF00) {
        // Nothing here
        return 0;
    }

    if (address < 0xFF80) {
        // I/O registers
        return gb->io_registers[address & 0xFF];
    }

    return gb->hram[address - 0xFF80];
}

static mem_read_function_t* const mem_read_map[] = {
    mem_read_rom,       // 0XXX
    mem_read_rom,       // 1XXX
    mem_read_rom,       // 2XXX
    mem_read_rom,       // 3XXX
    mem_read_mbc_rom,   // 4XXX
    mem_read_mbc_rom,   // 5XXX
    mem_read_mbc_rom,   // 6XXX
    mem_read_mbc_rom,   // 7XXX

    mem_read_vram,      // 8XXX
    mem_read_vram,      // 9XXX

    mem_read_mbc_ram,   // AXXX
    mem_read_mbc_ram,   // BXXX

    mem_read_ram,       // CXXX
    mem_read_ram,       // DXXX
    mem_read_ram,       // EXXX
    mem_read_hram,      // FXXX
};

/* MEMORY WRITES */

/**
 * Write to the ROM bank on the cartridge
 */
static void mem_write_mbc_rom(gb_t *gb, uint16_t address, uint8_t value) {
    // TODO: Can't write here. this controls memory bank switching
}

/**
 * Write to the vram
 */
static void mem_write_vram(gb_t *gb, uint16_t address, uint8_t value) {
    gb->vram[address & 0x1FFF] = value;
}

/**
 * Write to the switchable RAM bank on the cartridge
 */
static void mem_write_mbc_ram(gb_t *gb, uint16_t address, uint8_t value) {
    gb->mbc_ram[address & 0x1FFF] = value;
}

/**
 * Write to the internal RAM
 */
static void mem_write_ram(gb_t *gb, uint16_t address, uint8_t value) {
    gb->ram[address & 0x1FFF] = value;
}

/**
 * Write to the high RAM sections
 */
static void mem_write_high_ram(gb_t *gb, uint16_t address, uint8_t value) {
    if (address < 0xFE00) {
        // Shadow copy of ram
        mem_write_ram(gb, address, value);
        return;
    }

    if (address < 0xFEA0) {
        // OAM
        gb->oam[address & 0xFF] = value;
        return;
    }

    if (address < 0xFF00) {
        // Nothing here
        return;
    }

    if (address < 0xFF80) {
        // I/O registers

        if (address == 0xFF50 && value) {
            // Disable bios
            mem_remove_bios(gb);
        }

        if (address == REG_DMA) {
            // Begin DMA
            gb->dma_mode = DMA_MODE_REQUESTED;
            gb->dma_addr = value << 8;
        }

        gb->io_registers[address & 0xFF] = value;
        return;
    }

    gb->hram[address - 0xFF80] = value;
}

static mem_write_function_t* const mem_write_map[] = {
    mem_write_mbc_rom,   // 0XXX
    mem_write_mbc_rom,   // 1XXX
    mem_write_mbc_rom,   // 2XXX
    mem_write_mbc_rom,   // 3XXX
    mem_write_mbc_rom,   // 4XXX
    mem_write_mbc_rom,   // 5XXX
    mem_write_mbc_rom,   // 6XXX
    mem_write_mbc_rom,   // 7XXX

    mem_write_vram,      // 8XXX
    mem_write_vram,      // 9XXX

    mem_write_mbc_ram,   // AXXX
    mem_write_mbc_ram,   // BXXX

    mem_write_ram,       // CXXX
    mem_write_ram,       // DXXX
    mem_write_ram,       // EXXX
    mem_write_high_ram,  // FXXX
};

void mem_init(gb_t *gb) {
    gb->rom = malloc(ROM_SIZE);
    gb->vram = malloc(VRAM_SIZE);
    gb->mbc_ram = malloc(MBC_RAM_SIZE);
    gb->ram = malloc(RAM_SIZE);
    gb->io_registers = malloc(IO_REGISTER_SIZE);
    gb->hram = malloc(HIGH_SPEED_RAM_SIZE);
    gb->oam = malloc(OAM_SIZE);
}

void mem_load_rom(gb_t *gb, const char *fname) {
    // File object
    FILE *f;

    // Open file
    f = fopen(fname, "rb");

    // Load 32768 bytes into the rom
    fread(gb->rom, ROM_SIZE, 1, f);

    // Close
    fclose(f);
}

uint8_t mem_read_byte(gb_t *gb, uint16_t address) {
    return mem_read_map[address >> 12](gb, address);
}

void mem_write_byte(gb_t *gb, uint16_t address, uint8_t val) {
    mem_write_map[address >> 12](gb, address, val);
}

uint16_t mem_read_word(gb_t *gb, uint16_t address) {
    return (mem_read_byte(gb, address + 1) << 8) | mem_read_byte(gb, address); 
}

void mem_write_word(gb_t *gb, uint16_t address, uint16_t val) {
    mem_write_byte(gb, address, val & 0xFF);
    mem_write_byte(gb, address + 1, val >> 8);
}

void mem_remove_bios(gb_t *gb) {
    gb->in_bios = 0;
}

/**
 * Compute dma
 */
void mem_dma(gb_t *gb) {
    switch (gb->dma_mode) {
        case DMA_MODE_REQUESTED:
            gb->dma_cycles = 0;
            gb->dma_mode = DMA_MODE_TRANSFER;

            break;

        case DMA_MODE_TRANSFER:
            ;
            // Transfer data
            uint16_t read_addr = gb->dma_addr + gb->dma_cycles;
            uint16_t write_addr = 0xFE00 + gb->dma_cycles;

            uint8_t val = mem_read_byte(gb, read_addr);

            mem_write_byte(gb, write_addr, val);

            gb->dma_cycles++;

            if (gb->dma_cycles == 160) {
                // Done transfer
                gb->dma_mode = DMA_MODE_NONE;
            }

            break;
        
        default:
            break;
    }
}