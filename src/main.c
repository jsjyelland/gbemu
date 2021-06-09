#include <stdio.h>
#include <cpu.h>
#include <gpu.h>
#include <gb_memory.h>
#include <timer.h>

int main() {
    gb_t *gb = get_gb_instance();

    cpu_init(gb);
    mem_init(gb);
    gpu_init(gb);
    joypad_init();
    timer_init();

    printf("GBEMU begin\r\n");

    mem_load_rom(gb, "roms/dr mario.gb");

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