#ifndef TIMER_H
#define TIMER_H

#include <gb.h>
#include <gb_memory.h>

void timer_init();

void timer_tick(gb_t *gb);

#endif