#ifndef GAME_TILE_H
#define GAME_TILE_H

#include <libtcod.h>

enum tile_type
{
    TILE_TYPE_EMPTY,
    TILE_TYPE_FLOOR,
    TILE_TYPE_WALL,

    NUM_TILE_TYPES
};

struct tile_common
{
    TCOD_color_t ambient_color;
    float ambient_intensity;
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
    struct object *object;
    struct actor *actor;
    TCOD_list_t items;
};

void tile_init(struct tile *tile, enum tile_type type, bool seen);
void tile_reset(struct tile *tile);

#endif
