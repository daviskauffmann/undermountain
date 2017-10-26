#ifndef ACTOR_H
#define ACTOR_H

#include <libtcod.h>

#include "map.h"

typedef struct map_s map_t;

typedef enum actortype_e {
    ACTORTYPE_NONE = 0,
    ACTORTYPE_PLAYER,
    ACTORTYPE_MONSTER,
    NUM_ACTORTYPES
} actortype_t;

typedef struct actor_s
{
    actortype_t type;
    int x;
    int y;
} actor_t;

typedef struct actorinfo_s
{
    unsigned char glyph;
    TCOD_color_t color;
    int sight_radius;
} actorinfo_t;

actor_t *actor_create(map_t *map, actortype_t type, int x, int y, unsigned char glyph, TCOD_color_t color, int sight_radius);
void actor_destroy(map_t *map, actor_t *actor);
void actor_move(map_t *map, actor_t *actor, int x, int y);

#endif