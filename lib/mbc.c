#include <mbc.h>

void mbc_setup(gb_t *gb, FILE* f) {
    uint8_t cart_type = gb->rom[0x0147];
    uint8_t rom_size = gb->rom[0x0148];
    uint8_t ram_size = gb->ram[0x0149];

    uint32_t mbc_rom_size = (0x8000 << rom_size) - 0x4000;
    gb->mbc_rom = malloc(mbc_rom_size);

    if (ram_size > 0) {
        gb->mbc_ram = malloc(0x8000 < ((ram_size - 1) * 2));
    }

    gb->current_ram_bank = 0;
    gb->current_rom_bank = 0;
    gb->rom_ram_mode = MBC_ROM_MODE;

    // Load cartridge into rom

    // Move to the position of the 
    fseek(f, 0x4000, SEEK_SET);
    fread(gb->mbc_rom, mbc_rom_size, 1, f);
}

uint8_t mbc_read_rom_bank(gb_t *gb, uint16_t address) {
    return gb->mbc_rom[(0x4000 * gb->current_rom_bank) + (address & 0x3FFF)];
}

void mbc_write_rom_bank(gb_t *gb, uint16_t address, uint8_t value) {
    if (address < 0x2000) {
        // Enable ram bank

        if (value == 0x0A) {
            // Enable
        } else {
            // Disable
        }
    } else if (address < 0x4000) {
        // Switch rom bank (lower 5 bits)

        if (value == 0) {
            value = 1;
        }

        gb->current_rom_bank &= 0xE0;
        gb->current_rom_bank |= (value - 1) & 0x1F;
    } else if (address < 0x6000) {
        // Swirch rom bank (upper 2 bits) / switch ram

        if (gb->rom_ram_mode == MBC_ROM_MODE) {
            gb->current_rom_bank &= 0x1F;
            gb->current_rom_bank |= value & 0x60;
        } else {
            gb->current_ram_bank = value;
        }
        
    } else if (address < 0x8000) {
        // Rom/ram mode
        if (value < 2) {
            gb->rom_ram_mode = value;

            if (value == MBC_ROM_MODE) {
                gb->current_ram_bank = 0;
            } else {
                gb->current_rom_bank &= 0x1F;
            }
        }
    }
}

uint8_t mbc_read_ram_bank(gb_t *gb, uint16_t address) {
    // TODO: deal with accessing ram that's not there (e.g. when in 2kb ram mode)
    return gb->mbc_ram[(gb->current_ram_bank * 0x2000) + (address & 0x1FFF)];
}

void mbc_write_ram_bank(gb_t *gb, uint16_t address, uint8_t value) {
    gb->mbc_ram[(gb->current_ram_bank * 0x2000) + (address & 0x1FFF)] = value;
}