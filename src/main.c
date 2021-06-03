#include <stdio.h>
#include <cpu.h>
#include <gpu.h>
#include <gb_memory.h>

int main(int argc, char *argv[]) {
    gb_t *gb = get_gb_instance();

    cpu_init(gb);
    mem_init(gb);
    gpu_init(gb);

    printf("GBEMU begin\r\n");

    mem_load_rom(gb, "roms/tetris.gb");

    // Main tick loop
    for (;;) {
        cpu_tick(gb);
        if (!gpu_tick(gb)) {
            break;
        }

        // Handle inputs. For now - all buttons not pressed
        mem_write_byte(gb, REG_P1, 0xF);
    }

    return 0;
}