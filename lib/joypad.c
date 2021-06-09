#include <joypad.h>

static struct {
    union {
        struct {
            uint8_t right:1;
            uint8_t left:1;
            uint8_t up:1;
            uint8_t down:1;
        };
        struct {
            uint8_t p14;
        };
    };
    
    union {
        struct {
            uint8_t a:1;
            uint8_t b:1;
            uint8_t select:1;
            uint8_t start:1;
        };
        struct {
            uint8_t p15;
        };
    };
} joypad;

void joypad_init() {
    joypad.p14 = 0xF;
    joypad.p15 = 0xF;
}

void key_pressed_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    uint8_t joypad_key_pressed = 1;

    switch (key) {
        case GLFW_KEY_UP:
            joypad.up = !action;
            break;

        case GLFW_KEY_DOWN:
            joypad.down = !action;
            break;

        case GLFW_KEY_LEFT:
            joypad.left = !action;
            break;

        case GLFW_KEY_RIGHT:
            joypad.right = !action;
            break;

        case GLFW_KEY_A:
            joypad.a = !action;
            break;

        case GLFW_KEY_B:
            joypad.b = !action;
            break;

        case GLFW_KEY_ENTER:
            joypad.start = !action;
            break;

        case GLFW_KEY_RIGHT_SHIFT:
            joypad.select = !action;
            break;

        default:
            joypad_key_pressed = 0;
    }

    if (action && joypad_key_pressed) {
        gb_t *gb = get_gb_instance();

        // Interrupt
        mem_write_byte(gb, INTERRUPT_FLAGS, mem_read_byte(gb, INTERRUPT_FLAGS) | INT_FLAG_JOYPAD);
    }
}

void joypad_update_io_registers(gb_t *gb) {
    uint8_t joypad_mask;

    uint8_t read_mask = mem_read_byte(gb, REG_P1);

    if (!(read_mask & (1 << 4))) {
        joypad_mask = joypad.p14 & 0xF;
    } else if (!(read_mask & (1 << 5))) {
        joypad_mask = joypad.p15 & 0xF;
    } else {
        return;
    }

    read_mask &= 0xF0;
    mem_write_byte(gb, REG_P1, read_mask | joypad_mask);
}