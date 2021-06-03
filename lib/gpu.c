#include <gpu.h>

static uint32_t gpu_counter = 0;
static uint32_t display_refresh_counter = 0;

static GLFWwindow *window;

/**
 * Get the tile index in vram for block x & y
 */
static uint8_t get_tile_map_index(gb_t *gb, uint8_t block_x, uint8_t block_y) {
    // Determine the start of the background tile map from bit 3 of LCDC
    uint16_t tile_map_start = (mem_read_byte(gb, REG_LCDC) & (1 << 3)) ? 0x9C00 : 0x9800;

    uint16_t tile_addr = tile_map_start + 32 * block_y + block_x;

    // Get the character code of the tile at this location
    uint8_t character_code = mem_read_byte(gb, tile_addr);

    return character_code;
}

/**
 * Get the tile map index for a display block x & y
 * Takes into account scroll
 */
static uint8_t get_tile_map_index_scroll(gb_t *gb, uint8_t block_x, uint8_t block_y) {
    uint8_t scx = mem_read_byte(gb, REG_SCX);
    uint8_t scy = mem_read_byte(gb, REG_SCY);
    return get_tile_map_index(gb, (block_x + scx) & 0x3F, (block_y + scy) & 0x3F);
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

    if (mem_read_byte(gb, REG_LCDC) & (1 << 4)) {
        // 0x8000 mode

        tile_address = 0x8000 + tile_index * 16;
    } else {
        // 0x8000 mode

        tile_address = 0x8800 + (int8_t)(tile_index) * 16;
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

    window = glfwCreateWindow(DISPLAY_WIDTH, DISPLAY_HEIGHT, "gbemu", NULL, NULL);

    if (!window) {
        printf("Failed to open window\n");
        
        return 0;
    }
    
    glfwMakeContextCurrent(window);

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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

    if (gpu_counter > 20) {
        gpu_counter = 0;
        get_gb_instance()->io_registers[0x44] = get_gb_instance()->io_registers[0x44] + 1;

        if (get_gb_instance()->io_registers[0x44] > 153) {
            get_gb_instance()->io_registers[0x44] = 0;
        }       
    }

    if (display_refresh_counter >= 70000) {
        display_refresh_counter = 0;

        // glRasterPos2d(0, 0);

        for (uint8_t i_x = 0; i_x < DISPLAY_WIDTH / 8; i_x++) {
            for (uint8_t i_y = 0; i_y < DISPLAY_HEIGHT / 8; i_y++) {
                uint8_t tile_index = get_tile_map_index_scroll(gb, i_x, i_y);

                for (uint8_t x = 0; x <= 7; x++) {
                    for (uint8_t y = 0; y <= 7; y++) {
                        uint8_t col = grey_value(bg_palette_transform(gb, get_tile_pixel(gb, tile_index, x, y)));

                        pixelData[(DISPLAY_HEIGHT - 1) - (i_y * 8 + y)][i_x * 8 + x][0] = col;
                        pixelData[(DISPLAY_HEIGHT - 1) - (i_y * 8 + y)][i_x * 8 + x][1] = col;
                        pixelData[(DISPLAY_HEIGHT - 1) - (i_y * 8 + y)][i_x * 8 + x][2] = col;
                    }
                }
            }
        }

        glDrawPixels(DISPLAY_WIDTH, DISPLAY_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixelData);
        
        glfwSwapBuffers(window);

        glFlush();

        glfwPollEvents(); 
    }

    return !glfwWindowShouldClose(window);
}