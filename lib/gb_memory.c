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
    0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x4C,
    0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

/* MEMORY READS */

/**
 * Read from the ROM bank 0
 */
static uint8_t mem_read_rom(uint16_t address) {
    // If in the bios, map addresses 0x00 to 0xFF to the bios instructions
    if (address < 0x100 && get_gb_instance()->in_bios) {
        return bios[address];
    } else {
        return get_gb_instance()->rom[address];
    }
}

/**
 * Read from the switchable ROM bank on the cartridge
 */
static uint8_t mem_read_mbc_rom(uint16_t address) {
    return get_gb_instance()->rom[address];
}

/**
 * Read from the vram
 */
static uint8_t mem_read_vram(uint16_t address) {
    return get_gb_instance()->vram[address & 0x1FFF];
}

/**
 * Read from the switchable RAM bank on the cartridge
 */
static uint8_t mem_read_mbc_ram(uint16_t address) {
    return get_gb_instance()->mbc_ram[address & 0x1FFF];
}

/**
 * Read from the internal RAM
 */
static uint8_t mem_read_ram(uint16_t address) {
    return get_gb_instance()->ram[address & 0x1FFF];
}

/**
 * Read from the high RAM sections
 */
static uint8_t mem_read_high_ram(uint16_t address) {
    if (address < 0xFE00) {
        // Shadow copy of ram
        return mem_read_ram(address);
    }

    if (address < 0xFEA0) {
        // OAM
        return get_gb_instance()->oam[address & 0xFF];
    }

    if (address < 0xFF00) {
        // Nothing here
        return 0;
    }

    if (address < 0xFF80) {
        // I/O registers
        return get_gb_instance()->io_registers[address & 0xFF];
    }

    if (address == 0xFFFF) {
        // Interrupt enable register
    }

    return get_gb_instance()->high_speed_ram[address - 0xFF80];
}

static mem_read_function_t* const mem_read_map[] = {
    mem_read_rom,       // 0XXX
    mem_read_rom,       // 1XXX
    mem_read_rom,       // 2XXX
    mem_read_rom,       // 3XXX
    mem_read_mbc_rom,       // 4XXX
    mem_read_mbc_rom,       // 5XXX
    mem_read_mbc_rom,       // 6XXX
    mem_read_mbc_rom,       // 7XXX

    mem_read_vram,      // 8XXX
    mem_read_vram,      // 9XXX

    mem_read_mbc_ram,   // AXXX
    mem_read_mbc_ram,   // BXXX

    mem_read_ram,       // CXXX
    mem_read_ram,       // DXXX
    mem_read_ram,       // EXXX
    mem_read_high_ram,  // FXXX
};

/* MEMORY WRITES */

/**
 * Write to the ROM bank on the cartridge
 */
static void mem_write_mbc_rom(uint16_t address, uint8_t value) {
    // TODO: Can't write here. this controls memory bank switching
}

/**
 * Write to the vram
 */
static void mem_write_vram(uint16_t address, uint8_t value) {
    get_gb_instance()->vram[address & 0x1FFF] = value;
}

/**
 * Write to the switchable RAM bank on the cartridge
 */
static void mem_write_mbc_ram(uint16_t address, uint8_t value) {
    get_gb_instance()->mbc_ram[address & 0x1FFF] = value;
}

/**
 * Write to the internal RAM
 */
static void mem_write_ram(uint16_t address, uint8_t value) {
    get_gb_instance()->ram[address & 0x1FFF] = value;
}

/**
 * Write to the high RAM sections
 */
static void mem_write_high_ram(uint16_t address, uint8_t value) {
    if (address < 0xFE00) {
        // Shadow copy of ram
        mem_write_ram(address, value);
        return;
    }

    if (address < 0xFEA0) {
        // OAM
        get_gb_instance()->oam[address & 0xFF] = value;
        return;
    }

    if (address < 0xFF00) {
        // Nothing here
        return;
    }

    if (address < 0xFF80) {
        // I/O registers

        if (address == 0xFF50 && value) {
            // Disable in bios
            mem_remove_bios();
            printf("bios done\r\n");
        }

        get_gb_instance()->io_registers[address & 0xFF] = value;
        return;
    }

    if (address == 0xFFFF) {
        // Interrupt enable register
        return;
    }

    get_gb_instance()->high_speed_ram[address - 0xFF80] = value;
}

static mem_write_function_t* const mem_write_map[] = {
    mem_write_mbc_rom,       // 0XXX
    mem_write_mbc_rom,       // 1XXX
    mem_write_mbc_rom,       // 2XXX
    mem_write_mbc_rom,       // 3XXX
    mem_write_mbc_rom,       // 4XXX
    mem_write_mbc_rom,       // 5XXX
    mem_write_mbc_rom,       // 6XXX
    mem_write_mbc_rom,       // 7XXX

    mem_write_vram,      // 8XXX
    mem_write_vram,      // 9XXX

    mem_write_mbc_ram,   // AXXX
    mem_write_mbc_ram,   // BXXX

    mem_write_ram,       // CXXX
    mem_write_ram,       // DXXX
    mem_write_ram,       // EXXX
    mem_write_high_ram,  // FXXX
};

void mem_init() {
    get_gb_instance()->rom = malloc(ROM_SIZE);
    get_gb_instance()->vram = malloc(VRAM_SIZE);
    get_gb_instance()->mbc_ram = malloc(MBC_RAM_SIZE);
    get_gb_instance()->ram = malloc(RAM_SIZE);
    get_gb_instance()->io_registers = malloc(IO_REGISTER_SIZE);
    get_gb_instance()->high_speed_ram = malloc(HIGH_SPEED_RAM_SIZE);
}

void mem_load_rom(const char *fname) {
    // File object
    FILE *f;

    // Open file
    f = fopen(fname, "rb");

    // Load 32768 bytes into the rom
    fread(get_gb_instance()->rom, ROM_SIZE, 1, f);

    // Close
    fclose(f);
}

uint8_t mem_read_byte(uint16_t address) {
    return mem_read_map[address >> 12](address);
}

void mem_write_byte(uint16_t address, uint8_t val) {
    mem_write_map[address >> 12](address, val);
}

uint16_t mem_read_16(uint16_t address) {
    return (mem_read_byte(address + 1) << 8) | mem_read_byte(address); 
}

void mem_write_16(uint16_t address, uint16_t val) {
    mem_write_byte(address, val & 0xFF);
    mem_write_byte(address + 1, val >> 8);
}

void mem_remove_bios() {
    get_gb_instance()->in_bios = 0;
}