#ifndef UM_GAME_TILE_H
#define UM_GAME_TILE_H

#include <libtcod.h>

enum tile_type
{
    TILE_TYPE_EMPTY,
    TILE_TYPE_FLOOR,
    TILE_TYPE_GRASS,
    TILE_TYPE_WALL,

    NUM_TILE_TYPES
};

struct tile_metadata
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
    struct corpse *corpse;
    struct item *item;
    struct surface *surface;
};

extern const struct tile_metadata tile_metadata;
extern const struct tile_data tile_database[NUM_TILE_TYPES];

void tile_init(
    struct tile *tile,
    enum tile_type type,
    bool seen);
void tile_uninit(struct tile *tile);

#endif
