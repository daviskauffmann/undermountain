#ifndef ACTOR_H
#define ACTOR_H

#include <libtcod.h>

#include "tile.h"

#define LIT_ROOMS 0

typedef struct map_s map_t;

typedef struct actor_s
{
    map_t *map;
    int x;
    int y;
    unsigned char glyph;
    TCOD_color_t color;
    bool torch;
    TCOD_list_t items;
    int fov_radius;
    TCOD_map_t fov_map;
    bool mark_for_delete;
} actor_t;

actor_t *actor_create(map_t *map, int x, int y, unsigned char glyph, TCOD_color_t color, int fov_radius);
void actor_turn(actor_t *actor);
void actor_tick(actor_t *actor);
void actor_calc_fov(actor_t *actor);
bool actor_move_towards(actor_t *actor, int x, int y);
bool actor_move(actor_t *actor, int x, int y);
void actor_pick_item(actor_t *actor, tile_t *tile);
void actor_destroy(actor_t *actor);

#endif