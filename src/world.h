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
    uint8_t current_map_index;
    map_t *current_map;
    actor_t *player;
} world_t;

extern world_t *world;

world_t *world_create(void);
void world_destroy(world_t *world);

map_t *map_create(world_t *world);
void map_update(map_t *map, actor_t *player);
TCOD_map_t map_to_TCOD_map(map_t *map);
void map_calc_fov(TCOD_map_t TCOD_map, int x, int y, int radius);
TCOD_path_t map_calc_path(TCOD_map_t TCOD_map, int ox, int oy, int dx, int dy);

actor_t *actor_create(map_t *map, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color, uint8_t sight_radius);
void actor_destroy(map_t *map, actor_t *actor);
void actor_move(map_t *map, actor_t *actor, uint8_t x, uint8_t y);

#endif