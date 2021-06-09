#include <gpu.h>

static uint32_t gpu_counter = 0;
static uint32_t display_refresh_counter = 0;

static uint8_t lcd_mode = LCD_MODE_2_OAM;

// The indexes of the (max 10) sprites to be drawn on the current line
static uint8_t line_sprites[10];

// Current x and y position being drawn
static uint8_t x_pos = 0;
static uint8_t y_pos = 0;

static GLFWwindow *window;

// Data to hold the pixels to be drawn to the screen
static GLubyte pixel_buffer[DISPLAY_HEIGHT][DISPLAY_WIDTH][3];

/**
 * Transform from gameboy pixel number (0-3)
 * to greyscale level (0-255)
 */
static uint8_t grey_value(uint8_t pixel) {
    return 255 - (pixel * 85);
}

/**
 * Transfrom a pixel through the bg palette
 */
static uint8_t bg_palette_transform(gb_t *gb, uint8_t pixel) {
    uint8_t bgp = mem_read_byte(gb, REG_BGP);

    return (bgp & (0b11 << (2 * pixel))) >> (2 * pixel);
}

/**
 * Transfrom a pixel through the obj palette
 */
static uint8_t obj_palette_transform(gb_t *gb, uint8_t pixel, uint8_t obj_pallete) {
    // Colour 0 is transparent
    if (pixel == 0) {
        return 0;
    }

    uint8_t obp = mem_read_byte(gb, obj_pallete ? REG_OBP1 : REG_OBP0);

    return (obp & (0b11 << (2 * pixel))) >> (2 * pixel);
}

/**
 * Get the tile map index for an absolute display position x & y (0-255)
 */
static uint8_t get_bg_tile_index(gb_t *gb, uint8_t x, uint8_t y) {
    // Turn into block number (0-32)
    uint8_t block_x = x >> 3;
    uint8_t block_y = y >> 3;

    // Determine the start of the background tile map from bit 3 of LCDC
    uint16_t tile_map_start = (mem_read_byte(gb, REG_LCDC) & LCDC_BG_TILE_MAP_DISPLAY_SELECT) ? 0x9C00 : 0x9800;

    uint16_t tile_addr = tile_map_start + 32 * block_y + block_x;

    // Get the character code of the tile at this location
    uint8_t character_code = mem_read_byte(gb, tile_addr);

    return character_code;
}

/**
 * Get the pixel value from a tile at (x, y) in the tile
 */
static uint8_t get_tile_pixel(gb_t *gb, uint16_t tile_address, uint8_t x, uint8_t y) {
    uint8_t tile_lsb = mem_read_byte(gb, tile_address + (y * 2));
    uint8_t tile_msb = mem_read_byte(gb, tile_address + (y * 2) + 1);

    uint8_t val = (!!(tile_msb & (1 << (7 - x))) << 1) | (!!(tile_lsb & (1 << (7 - x))));

    return val;
}

/**
 * Get the pixel value from a background/window tile at (x, y) in the tile
 */
static uint8_t get_tile_pixel_bg_window(gb_t *gb, uint16_t tile_index, uint8_t x, uint8_t y) {
    uint16_t tile_address;

    if (mem_read_byte(gb, REG_LCDC) & (mem_read_byte(gb, REG_LCDC) & LCDC_BG_WINDOW_TILE_DATA_SELECT)) {
        // 0x8000 mode
        tile_address = 0x8000 + tile_index * 16;
    } else {
        // 0x8800 mode
        tile_address = 0x9000 + ((int8_t)(tile_index) * 16);
    }

    return get_tile_pixel(gb, tile_address, x, y);
}

/**
 * Get the entry in the OAM for index 0 - 39
 */
static uint32_t get_oam_entry(gb_t *gb, uint8_t index) {
    if (index > 40) {
        printf("Index %i in OAM out of range (0-39)\n", index);
        abort();
    }

    uint32_t ret_val;

    // Read the 4 bytes
    ret_val = mem_read_byte(gb, (index * 4) + 0xFE00);
    ret_val |= mem_read_byte(gb, (index * 4) + 0xFE01) << 8;
    ret_val |= mem_read_byte(gb, (index * 4) + 0xFE02) << 16;
    ret_val |= mem_read_byte(gb, (index * 4) + 0xFE03) << 24;

    return ret_val;
}

/*
 * Determine if sprite overlaps an x position
 */
static uint8_t sprite_at_x(gb_t *gb, uint8_t index, uint8_t x) {
    uint32_t sprite_val = get_oam_entry(gb, index);

    int16_t sprite_x = SPRITE_XPOS(sprite_val);

    return (x >= sprite_x) && (x < (sprite_x + 8));
}

/**
 * Determine if sprite overlaps a y position
 */
static uint8_t sprite_at_y(gb_t *gb, uint8_t index, uint8_t y) {
    uint32_t sprite_val = get_oam_entry(gb, index);

    int16_t sprite_y = SPRITE_YPOS(sprite_val);

    uint8_t sprite_height = (mem_read_byte(gb, REG_LCDC) & LCDC_OBJ_BLOCK_COMPOSITION) ? 16 : 8;

    return (y >= sprite_y) && (y < (sprite_y + sprite_height));
}

/**
 * Get the pixel colour from a sprite (transformed through palette)
 */
static uint8_t get_sprite_pixel(gb_t *gb, uint8_t index, uint8_t x, uint8_t y) {
    if (!sprite_at_x(gb, index, x) || !(sprite_at_y(gb, index, y))) {
        // Sprite isn't at this location
        return 0;
    }

    uint32_t sprite_val = get_oam_entry(gb, index);

    // Determine if 8x8 or 8x16 mode
    uint8_t double_height_mode = !!(mem_read_byte(gb, REG_LCDC) & LCDC_OBJ_BLOCK_COMPOSITION);

    uint8_t x_in_sprite = x - SPRITE_XPOS(sprite_val);

    if (SPRITE_ATTR(sprite_val) & SPRITE_ATTR_XFLIP) {
        // Flip horizontally
        x_in_sprite = 7 - x_in_sprite;
    }

    uint8_t y_in_sprite = y - SPRITE_YPOS(sprite_val);

    if (SPRITE_ATTR(sprite_val) & SPRITE_ATTR_YFLIP) {
        // Flip vertically
        y_in_sprite = (double_height_mode ? 7 : 15) - y_in_sprite;
    }

    uint16_t tile_address = 0x8000 + ((SPRITE_TILENUM(sprite_val) & (double_height_mode ? 0xFE : 0xFF))) * 16;

    if (double_height_mode && y_in_sprite > 7) {
        // In second tile
        tile_address++;
        y_in_sprite -= 8;
    }

    uint8_t col = get_tile_pixel(gb, tile_address, x_in_sprite, y_in_sprite);
    col = obj_palette_transform(gb, col, !!(SPRITE_ATTR(sprite_val) & SPRITE_ATTR_PALETTE));

    return col;
}

/**
 * Render the current frame to the screen
 */
static void gpu_render_frame(gb_t *gb) {
    int width, height;

    glClear(GL_COLOR_BUFFER_BIT);

    if (mem_read_byte(gb, REG_LCDC) & LCDC_LCD_CONTROL) {
        glfwGetFramebufferSize(window, &width, &height);

        glPixelZoom((GLfloat)width / (GLfloat)DISPLAY_WIDTH, (GLfloat)height / (GLfloat)DISPLAY_HEIGHT);
        
        glDrawPixels(DISPLAY_WIDTH, DISPLAY_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, pixel_buffer);
        
        glfwSwapBuffers(window);
    }
}

/**
 * Calculate the value of a pixel and put into pixel buffer
 */
static void calculate_pixel(gb_t *gb, uint8_t x, uint8_t y) {
    uint8_t scx = mem_read_byte(gb, REG_SCX);
    uint8_t scy = mem_read_byte(gb, REG_SCY);

    // Adjust for scroll
    uint8_t x_adj = (x + scx) & 0xFF;
    uint8_t y_adj = (y + scy) & 0xFF;
    
    // Calculate background
    uint8_t tile_index = get_bg_tile_index(gb, x_adj, y_adj);
    uint8_t bg_pixel = bg_palette_transform(gb, get_tile_pixel_bg_window(gb, tile_index, x_adj & 0x7, y_adj & 0x7));

    // Calculate window
    // TODO

    // Calculate sprites
    uint8_t sprite_bg_priority = 0;
    uint8_t sprite_pixel = 0;
    uint8_t sprite_least_x = 255;

    for (uint8_t i = 0; i < 10; i++) {
        uint8_t index = line_sprites[i];
        if (index == SPRITE_INDEX_NO_SPRITE) {
            // Reached the end of the list
            break;
        }

        if (sprite_at_x(gb, index, x)) {
            // Current x is in this sprite

            // Sprite with lowest x is always displayed
            uint32_t sprite_val = get_oam_entry(gb, index);
            uint8_t sprite_x = SPRITE_XPOS(sprite_val);
            
            if (sprite_x < sprite_least_x) {
                // This sprite has a lower x position, and thus has
                // priority
                sprite_pixel = get_sprite_pixel(gb, index, x, y);
                sprite_bg_priority = !!(SPRITE_ATTR(sprite_val) & SPRITE_ATTR_OBJ_BG_PRIORITY); 
                sprite_least_x = sprite_x;
            }
        }
    }

    // Depending on the priority of the sprite, overlay the sprite pixel
    if ((sprite_pixel && !sprite_bg_priority) || (!bg_pixel && sprite_bg_priority)) {
        bg_pixel = sprite_pixel;
    }

    // Calculate the screen colour
    uint8_t col = grey_value(bg_pixel);

    // Push to pixel buffer
    pixel_buffer[(DISPLAY_HEIGHT - 1) - y][x][0] = col;
    pixel_buffer[(DISPLAY_HEIGHT - 1) - y][x][1] = col;
    pixel_buffer[(DISPLAY_HEIGHT - 1) - y][x][2] = col;
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

/**
 * Write the mode value to the stat register
 */
static void write_mode(gb_t *gb) {
    uint8_t stat = mem_read_byte(gb, REG_STAT);
    stat &= ~3;
    stat |= (lcd_mode & 3);
    mem_write_byte(gb, REG_STAT, stat);
}

/**
 * Update
 */
int gpu_tick(gb_t *gb) {
    // Update based on mode
    switch (lcd_mode) {
        case LCD_MODE_0_HBLANK:
            gpu_counter++;

            if (gpu_counter > 200) {
                gpu_counter = 0;
                y_pos++;

                // Write y position to LY register
                mem_write_byte(gb, REG_LY, y_pos);

                if (y_pos == DISPLAY_HEIGHT) {
                    // Drawn all lines, go into vblank

                    // Vblank interrupt
                    mem_write_byte(gb, INTERRUPT_FLAGS, mem_read_byte(gb, INTERRUPT_FLAGS) | INT_FLAG_VBLANK);

                    lcd_mode = LCD_MODE_1_VBLANK;
                    write_mode(gb);

                    // Render to screen                    
                    gpu_render_frame(gb);
                    glfwPollEvents();
                } else {
                    lcd_mode = LCD_MODE_2_OAM;
                    write_mode(gb);
                }
            }

            break;
        
        case LCD_MODE_1_VBLANK:
            gpu_counter++;

            if (gpu_counter == 456) {
                gpu_counter = 0;

                y_pos++;

                // Write y position to LY register
                mem_write_byte(gb, REG_LY, y_pos);
            }

            if (y_pos == 154) {
                // Restart
                lcd_mode = LCD_MODE_2_OAM;
                write_mode(gb);

                x_pos = 0;
                y_pos = 0;
                
                // Write y position to LY register
                mem_write_byte(gb, REG_LY, y_pos);
            }

            break;

        case LCD_MODE_2_OAM:
            gpu_counter++;

            if (gpu_counter > 80) {
                gpu_counter = 0;

                // Index of sprite on line (max of 10)
                uint8_t sprite_array_index = 0;

                // Reset line sprite array
                memset(line_sprites, SPRITE_INDEX_NO_SPRITE, sizeof(line_sprites));
                
                if (mem_read_byte(gb, REG_LCDC) | LCDC_OBJ_ON) {
                    // Loop through all of OAM to find the first 10 sprites that are
                    // on the current line
                    for (uint8_t i = 0; i < 40; i++) {
                        uint32_t sprite_val = get_oam_entry(gb, i);

                        if (sprite_at_y(gb, i, y_pos)) {
                            // On current line - add to array
                            line_sprites[sprite_array_index++] = i;
                        }

                        if (sprite_array_index == 10) {
                            // Reached max number of sprites
                            break;
                        }
                    }
                }

                lcd_mode = LCD_MODE_3_TRANSFER;
                write_mode(gb);
            }

            break;

        case LCD_MODE_3_TRANSFER:
            // Calculate pixel
            calculate_pixel(gb, x_pos, y_pos);

            if (x_pos < DISPLAY_WIDTH) {
                x_pos++;
            } else {
                gpu_counter++;

                if (gpu_counter > 296) {
                    gpu_counter = 0;

                    // Reached end of line, go into hblank
                    lcd_mode = LCD_MODE_0_HBLANK;
                    write_mode(gb);
                    
                    x_pos = 0;
                }
            }

            break;

        default:
            break;
    }

    return !glfwWindowShouldClose(window);
}