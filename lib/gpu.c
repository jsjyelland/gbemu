#include <gpu.h>

static uint32_t gpu_counter = 0;
static uint32_t display_refresh_counter = 0;

static GLFWwindow *window;

/**
 * Get the tile index in vram for block x & y
 */
static uint8_t get_tile_map_index(gb_t *gb, uint8_t block_x, uint8_t block_y) {
    // Determine the start of the background tile map from bit 3 of LCDC
    uint16_t tile_map_start = (mem_read_byte(gb, REG_LCDC) & LCDC_BG_TILE_MAP_DISPLAY_SELECT) ? 0x9C00 : 0x9800;

    uint16_t tile_addr = tile_map_start + 32 * block_y + block_x;

    // Get the character code of the tile at this location
    uint8_t character_code = mem_read_byte(gb, tile_addr);

    return character_code;
}

/**
 * Get the tile map index for a display position x & y (0-255)
 * Takes into account scroll
 */
static uint8_t get_tile_map_index_scroll(gb_t *gb, uint8_t x, uint8_t y) {
    return get_tile_map_index(gb, x >> 3, y >> 3);
}

/**
 * Transform from gameboy pixel number (0-3)
 * to greyscale level (0-255)
 */
static uint8_t grey_value(uint8_t pixel) {
    return pixel * 255 / 3;
}

/**
 * Transfrom a pixel through the bg palette
 */
static uint8_t bg_palette_transform(gb_t *gb, uint8_t pixel) {
    uint8_t bgp = mem_read_byte(gb, REG_BGP);

    return (0b11 << (2 * pixel)) & 0b11;
}

/**
 * Get the pixel value from a tile at (x, y) in the tile
 */
static uint8_t get_tile_pixel(gb_t *gb, uint16_t tile_index, uint8_t x, uint8_t y) {
    uint16_t tile_address;

    if (mem_read_byte(gb, REG_LCDC) & (mem_read_byte(gb, REG_LCDC) & LCDC_BG_TILE_DATA_SELECT)) {
        // 0x8000 mode

        tile_address = 0x8000 + tile_index * 16;
    } else {
        // 0x8000 mode

        tile_address = 0x9000 + ((int8_t)(tile_index) * 16);
    }

    uint8_t tile_lsb = mem_read_byte(gb, tile_address + (y * 2));
    uint8_t tile_msb = mem_read_byte(gb, tile_address + (y * 2) + 1);

    return (!!(tile_msb & (1 << (7 - x))) << 1) | (!!(tile_lsb & (1 << (7 - x))));
}

/**
 * Initialise the gpu
 */
int gpu_init(gb_t *gb) {
    if (!glfwInit()) {
        printf("Failed to init GLFW\n");
        return 0;
    }

    window = glfwCreateWindow(DISPLAY_WIDTH * DISPLAY_SCALE, DISPLAY_HEIGHT * DISPLAY_SCALE, "gbemu", NULL, NULL);

    if (!window) {
        printf("Failed to open window\n");
        return 0;
    }
    
    glfwMakeContextCurrent(window);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glShadeModel(GL_FLAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Assign key callback. Probably shouldn't be in gpu functions.
    glfwSetKeyCallback(window, key_pressed_callback);

    return 1;
}

// Data to hold the pixels to be drawn to the screen
static GLubyte pixelData[DISPLAY_HEIGHT][DISPLAY_WIDTH][3];

/**
 * Update
 */
int gpu_tick(gb_t *gb) {
    gpu_counter++;
    display_refresh_counter++;

    if (gpu_counter > 456) {
        gpu_counter = 0;
        gb->io_registers[0x44] = gb->io_registers[0x44] + 1;

        if (gb->io_registers[0x44] > 153) {
            gb->io_registers[0x44] = 0;
        }

        if (gb->io_registers[0x44] == 140) {
            mem_write_byte(gb, INTERRUPT_FLAGS, mem_read_byte(gb, INTERRUPT_FLAGS) | INT_FLAG_VBLANK);
        }
    }

    if (display_refresh_counter >= 400000) {
        display_refresh_counter = 0;
        int width, height;

        glClear(GL_COLOR_BUFFER_BIT);

        if ((mem_read_byte(gb, REG_LCDC) & LCDC_LCD_CONTROL) && (mem_read_byte(gb, REG_LCDC) & LCDC_BG_WINDOW_DISPLAY)) {
            glfwGetWindowSize(window, &width, &height);

            glPixelZoom((GLfloat)width / (GLfloat)DISPLAY_WIDTH, (GLfloat)height / (GLfloat)DISPLAY_HEIGHT);
            
            uint8_t scx = mem_read_byte(gb, REG_SCX);
            uint8_t scy = mem_read_byte(gb, REG_SCY);

            for (uint8_t y = 0; y < DISPLAY_HEIGHT; y++) {
                uint8_t y_adj = (y + scy) & 0xFF;
                for (uint8_t x = 0; x < DISPLAY_WIDTH; x++) {
                    uint8_t x_adj = (x + scx) & 0xFF;

                    uint8_t tile_index = get_tile_map_index_scroll(gb, x_adj, y_adj);

                    uint8_t col = grey_value(bg_palette_transform(gb, get_tile_pixel(gb, tile_index, x_adj & 0x7, y_adj & 0x7)));

                    pixelData[(DISPLAY_HEIGHT - 1) - y][x][0] = col;
                    pixelData[(DISPLAY_HEIGHT - 1) - y][x][1] = col;
                    pixelData[(DISPLAY_HEIGHT - 1) - y][x][2] = col;
                }
            }

            glDrawPixels(DISPLAY_WIDTH, DISPLAY_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
            
            glfwSwapBuffers(window);
        }

        glfwPollEvents(); 
    }

    return !glfwWindowShouldClose(window);
}