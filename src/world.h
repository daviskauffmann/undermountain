#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>
#include <libtcod.h>

#define MAP_WIDTH 100
#define MAP_HEIGHT 100

typedef enum {
    TILETYPE_EMPTY = 0,
    TILETYPE_FLOOR,
    TILETYPE_WALL,
    TILETYPE_STAIR_DOWN,
    TILETYPE_STAIR_UP,
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

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} room_t;

typedef struct
{
    uint8_t x;
    uint8_t y;
    uint8_t glyph;
    TCOD_color_t color;
    uint8_t sight_radius;
} actor_t;

typedef struct
{
    uint8_t stair_down_x;
    uint8_t stair_down_y;
    uint8_t stair_up_x;
    uint8_t stair_up_y;
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
    TCOD_list_t actors;
} map_t;

typedef struct
{
    TCOD_list_t maps;
} world_t;

world_t *world_create(void);
void world_update(void);
void world_destroy(void);

#endif