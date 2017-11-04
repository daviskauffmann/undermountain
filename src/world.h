#ifndef WORLD_H
#define WORLD_H

#include <libtcod.h>

#define SIMULATE_ALL_MAPS 1

TCOD_list_t maps;

void world_initialize(void);
void world_turn(void);
void world_tick(void);
void world_finalize(void);

#endif