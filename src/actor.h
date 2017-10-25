#ifndef ACTOR_H
#define ACTOR_H

#include <libtcod.h>

#include "world.h"

actor_t *actor_create(map_t *map, int x, int y, unsigned char glyph, TCOD_color_t color, int sight_radius);
void actor_destroy(map_t *map, actor_t *actor);
void actor_move(map_t *map, actor_t *actor, int x, int y);

#endif