#ifndef WORLD_H
#define WORLD_H

#include <libtcod.h>

#include "tile.h"

#define SIMULATE_ALL_MAPS 1

TCOD_list_t maps;

TCOD_color_t tile_color_light;
TCOD_color_t tile_color_dark;
TCOD_color_t torch_color;
tile_info_t tile_info[NUM_TILE_TYPES];

void world_initialize(void);
void world_turn(void);
void world_tick(void);
void world_finalize(void);

#endif