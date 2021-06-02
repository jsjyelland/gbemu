#include <gpu.h>

static uint32_t gpu_counter = 0;

static uint32_t last_cpu_ticks = 0;

void gpu_update() {

    gpu_counter += (get_ticks() - last_cpu_ticks);
    last_cpu_ticks = get_ticks();

    if (gpu_counter > 456) {
        gpu_counter = 0;
        get_gb_instance()->io_registers[0x44] = get_gb_instance()->io_registers[0x44] + 1;

        if (get_gb_instance()->io_registers[0x44] > 153) {
            get_gb_instance()->io_registers[0x44] = 0;
        }
    }

    if (gpu_counter >= 70224) {
        gpu_counter = 0;
    }
}