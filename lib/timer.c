#include <timer.h>

static uint16_t timer_increment_counter;
static uint8_t divider_increment_counter;

void timer_init() {
    timer_increment_counter = 0;
    divider_increment_counter = 0;
}

void timer_tick(gb_t *gb) {
    // DIV timer
    if (!divider_increment_counter) {
        (gb->io_registers[REG_DIV & 0xFF])++;
    }

    divider_increment_counter = (divider_increment_counter + 1) & 0xFF;

    // Timer counter

    uint8_t tmc = mem_read_byte(gb, REG_TMC);
    
    if (tmc & TMC_ENABLE) {
        timer_increment_counter++;
    }

    uint8_t increment = 0;

    switch (tmc & TMC_CLOCK_SELECT) {
        case TMC_CLOCK_DIV_16:
            if (timer_increment_counter >= 16) {
                increment = 1;
            }

            break;

        case TMC_CLOCK_DIV_64:
            if (timer_increment_counter >= 64) {
                increment = 1;
            }

            break;

        case TMC_CLOCK_DIV_256:
            if (timer_increment_counter >= 256) {
                increment = 1;
            }

            break;

        case TMC_CLOCK_DIV_1024:
            if (timer_increment_counter >= 1024) {
                increment = 1;
            }

            break;
    }

    if (increment) {
        timer_increment_counter = 0;

        uint8_t tima_val = mem_read_byte(gb, REG_TIMA);
        mem_write_byte(gb, REG_TIMA, (tima_val + 1) & 0xFF);

        if (tima_val == 0xFF) {
            // Overflow

            // Set counter to value in TMA
            mem_write_byte(gb, REG_TIMA, mem_read_byte(gb, REG_TMA));

            // Set interrupt flag
            mem_write_byte(gb, INTERRUPT_FLAGS, mem_read_byte(gb, INTERRUPT_FLAGS) | INT_FLAG_TIMER);
        }   
    }
}