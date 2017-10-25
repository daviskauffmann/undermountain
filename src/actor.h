#ifndef ACTOR_H
#define ACTOR_H

#include <stdint.h>
#include <libtcod.h>

#include "world.h"

actor_t *actor_create(map_t *map, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color, uint8_t sight_radius);
void actor_destroy(map_t *map, actor_t *actor);
void actor_move(map_t *map, actor_t *actor, uint8_t x, uint8_t y);

#endif