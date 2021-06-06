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

    mem_load_rom(gb, "C:/Users/yella/Downloads/cpu_instrs/cpu_instrs/individual/11-op a,(hl).gb");

    // Main tick loop
    for (;;) {
        cpu_tick(gb);

        if (!gpu_tick(gb)) {
            break;
        }

        joypad_update_io_registers(gb);
    }

    return 0;
}