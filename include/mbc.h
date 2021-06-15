#ifndef MBC_H
#define MBC_H

#include <gb.h>
#include <stdint.h>
#include <stdio.h>

#define MBC_TYPE_MBC1 0
#define MBC_TYPE_MBC2 1
#define MBC_TYPE_MBC3 2
#define MBC_TYPE_MBC5 3

#define MBC_ROM_MODE 0
#define MBC_RAM_MODE 1

void mbc_setup(gb_t *gb, FILE* f);

uint8_t mbc_read_rom_bank(gb_t *gb, uint16_t address);
void mbc_write_rom_bank(gb_t *gb, uint16_t address, uint8_t value);

uint8_t mbc_read_ram_bank(gb_t *gb, uint16_t address);
void mbc_write_ram_bank(gb_t *gb, uint16_t address, uint8_t value);

#endif