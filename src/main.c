#include <stdio.h>
#include <cpu.h>
#include <gpu.h>
#include <gb_memory.h>

int main(int argc, char *argv[]) {
    gb_t *gb = get_gb_instance();

    cpu_init(gb);
    mem_init(gb);
    gpu_init(gb);
    joypad_init();

    printf("GBEMU begin\r\n");

    mem_load_rom(gb, "roms/tetris.gb");

    uint8_t cycle_counter = 0;

    // Main tick loop
    for (;;) {
        if (cycle_counter == 0) {
            // Divide the cpu clock by 4
            cpu_tick(gb);
        }

        mem_dma(gb);

        if (!gpu_tick(gb)) {
            break;
        }

        joypad_update_io_registers(gb);

        cycle_counter++;
        cycle_counter &= 3;
    }

    return 0;
}