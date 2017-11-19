#ifndef WORLD_H
#define WORLD_H

#include <libtcod.h>

#include "ECS.h"

TCOD_list_t maps;

void world_init(void);
void world_reset(void);

/* Tiles */
typedef enum tile_type_e {
    TILE_FLOOR,
    TILE_WALL,
    TILE_STAIR_DOWN,
    TILE_STAIR_UP,

    NUM_TILES
} tile_type_t;

typedef struct tile_common_s
{
    TCOD_color_t shadow_color;
} tile_common_t;

typedef struct tile_info_s
{
    unsigned char glyph;
    TCOD_color_t color;
    bool is_walkable;
    bool is_transparent;
} tile_info_t;

typedef struct tile_s
{
    tile_type_t type;
    bool seen;
    TCOD_list_t entities;
} tile_t;

tile_common_t tile_common;
tile_info_t tile_info[NUM_TILES];

void tile_init(tile_t *tile, tile_type_t type, bool seen);
void tile_reset(tile_t *tile);

/* Rooms */
typedef struct room_s
{
    int x;
    int y;
    int w;
    int h;
} room_t;

room_t *room_create(int x, int y, int w, int h);
void room_get_random_pos(room_t *room, int *x, int *y);
void room_destroy(room_t *room);

/* Maps */
#define MAP_WIDTH 50
#define MAP_HEIGHT 50

typedef struct map_s
{
    int level;
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
} map_t;

map_t *map_create(int level);
bool map_is_inside(int x, int y);
room_t *map_get_random_room(map_t *map);
TCOD_map_t map_to_TCOD_map(map_t *map);
TCOD_map_t map_to_fov_map(map_t *map, int x, int y, int radius);
void map_destroy(map_t *map);

#endif