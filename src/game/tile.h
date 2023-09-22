#ifndef UM_GAME_TILE_H
#define UM_GAME_TILE_H

#include "list.h"
#include <libtcod.h>

enum tile_type
{
    TILE_TYPE_EMPTY,
    TILE_TYPE_FLOOR,
    TILE_TYPE_GRASS,
    TILE_TYPE_WALL,

    NUM_TILE_TYPES
};

struct tile_common
{
    TCOD_ColorRGB ambient_light_color;
    float ambient_light_intensity;
};

struct tile_data
{
    const char *name;
    unsigned char glyph;
    TCOD_ColorRGB color;
    bool is_walkable;
    bool is_transparent;
};

struct tile
{
    enum tile_type type;
    bool explored;
    struct object *object;
    struct actor *actor;
    struct list *corpses;
    struct list *items;
    struct surface *surface;
};

void tile_init(
    struct tile *tile,
    enum tile_type type,
    bool seen);
void tile_uninit(struct tile *tile);

#endif
