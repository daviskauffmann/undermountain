#ifndef TILE_H
#define TILE_H

#include <libtcod.h>

typedef struct light_s light_t;
typedef struct actor_s actor_t;

typedef enum tile_type_e {
    TILE_EMPTY = 0,
    TILE_FLOOR,
    TILE_WALL,
    TILE_STAIR_DOWN,
    TILE_STAIR_UP,

    NUM_TILE_TYPES
} tile_type_t;

typedef struct tile_info_s
{
    unsigned char glyph;
    TCOD_color_t light_color;
    TCOD_color_t dark_color;
    bool is_transparent;
    bool is_walkable;
} tile_info_t;

typedef struct tile_s
{
    tile_type_t type;
    bool seen;
    light_t *light;
    actor_t *actor;
    TCOD_list_t items;
} tile_t;

void tile_initialize(tile_t *tile, tile_type_t type);
void tile_finalize(tile_t *tile);

#endif