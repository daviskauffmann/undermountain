#ifndef TILE_H
#define TILE_H

#include <stdint.h>
#include <libtcod.h>

typedef enum {
    TILETYPE_EMPTY = 0,
    TILETYPE_FLOOR,
    TILETYPE_WALL,
    NB_TILETYPES
} tiletype_t;

typedef struct
{
    uint8_t glyph;
    TCOD_color_t color;
    bool is_transparent;
    bool is_walkable;
} tileinfo_t;

typedef struct
{
    tiletype_t type;
    bool seen;
} tile_t;

extern tileinfo_t tileinfo[NB_TILETYPES];

void tileinfo_init(void);

#endif