#ifndef TILE_H
#define TILE_H

#include <stdint.h>
#include <libtcod.h>

typedef enum {
    TILETYPE_EMPTY = 0,
    TILETYPE_FLOOR,
    TILETYPE_WALL,
    TILETYPE_COUNT
} tiletype_t;

typedef struct
{
    uint8_t glyph;
    TCOD_color_t color;
    bool opaque;
    bool solid;
} tileinfo_t;

typedef struct
{
    tiletype_t type;
    bool seen;
} tile_t;

extern tileinfo_t tileinfo[TILETYPE_COUNT];

void tileinfo_init(void);

void tile_draw(tile_t *tile, int x, int y);

#endif