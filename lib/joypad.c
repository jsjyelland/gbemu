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
            uint8_t b;
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
    }
}

uint8_t get_joypad_mask(uint8_t port) {
    return port == JOYPAD_PORT_P14 ? joypad.p14 : joypad.p15;
}

void joypad_update_io_registers(gb_t *gb) {
    uint8_t port;

    if (mem_read_byte(gb, REG_P1) & (1 << 4)) {
        port = JOYPAD_PORT_P14;
    }

    if (mem_read_byte(gb, REG_P1) & (1 << 5)) {
        port = JOYPAD_PORT_P15;
    }

    uint8_t joypad_mask = get_joypad_mask(port);

    mem_write_byte(gb, REG_P1, joypad_mask);
}