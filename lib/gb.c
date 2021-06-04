#include <gb.h>

gb_t* get_gb_instance() {
    static gb_t *gb_instance = NULL;

    if (gb_instance == NULL) {
        gb_instance = malloc(sizeof(*gb_instance));

        gb_instance->in_bios = 1;
        gb_instance->ime = 1;
    }
    
    return gb_instance;
}