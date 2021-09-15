#include <stdio.h>
#include <cpu.h>
#include <gpu.h>
#include <gb_memory.h>
#include <timer.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: gbemu <filename>\n");
        return 0;
    }

    gb_t *gb = get_gb_instance();

    cpu_init(gb);
    mem_init(gb);
    gpu_init(gb);
    joypad_init();
    timer_init();

    mem_load_rom(gb, argv[1]);

    // Functions as a clock divider
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

        timer_tick(gb);

        cycle_counter++;
        cycle_counter &= 3;
    }

    return 0;
}