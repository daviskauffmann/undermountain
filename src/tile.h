#ifndef TILE_H
#define TILE_H

#include <libtcod/libtcod.h>

enum tile_type
{
    TILE_EMPTY,
    TILE_FLOOR,
    TILE_WALL,
    TILE_DOOR_CLOSED,
    TILE_DOOR_OPEN,
    TILE_STAIR_DOWN,
    TILE_STAIR_UP,

    NUM_TILE_TYPES
};

struct tile_common
{
    TCOD_color_t shadow_color;
};

struct tile_info
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
    bool is_walkable;
    bool is_transparent;
};

struct tile
{
    enum tile_type type;
    bool seen;
    TCOD_list_t objects;
    TCOD_list_t actors;
    TCOD_list_t items;
};

void tile_init(struct tile *tile, enum tile_type type, bool seen);
void tile_reset(struct tile *tile);

#endif
