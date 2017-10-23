#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include <libtcod.h>

#define MAP_WIDTH 80
#define MAP_HEIGHT 50

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
    TCOD_list_t actors;
} map_t;

map_t *map_create();
void map_update(map_t *map);
void map_draw(map_t *map);

actor_t *actor_create(map_t *map, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color, uint8_t sight_radius);
void actor_destroy(map_t *map, actor_t *actor);
void actor_move(map_t *map, actor_t *actor, uint8_t x, uint8_t y);

#endif