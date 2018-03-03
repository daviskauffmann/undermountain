#ifndef TILE_H
#define TILE_H

#include <libtcod/libtcod.h>

typedef enum tile_type_e tile_type_t;
typedef struct tile_common_s tile_common_t;
typedef struct tile_info_s tile_info_t;
typedef struct tile_s tile_t;

enum tile_type_e
{
    TILE_EMPTY,
    TILE_FLOOR,
    TILE_WALL,
    TILE_DOOR_CLOSED,
    TILE_DOOR_OPEN,
    TILE_STAIR_DOWN,
    TILE_STAIR_UP,

    NUM_TILES
};

struct tile_common_s
{
    TCOD_color_t shadow_color;
};

struct tile_info_s
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
    bool is_walkable;
    bool is_transparent;
};

struct tile_s
{
    tile_type_t type;
    bool seen;
    TCOD_list_t entities;
};

void tile_init(tile_t *tile, tile_type_t type, bool seen);
void tile_reset(tile_t *tile);

#endif
