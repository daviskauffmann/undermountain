#ifndef MAP_H
#define MAP_H

#include <libtcod/libtcod.h>

#include "tile.h"

typedef struct game_s game_t;
typedef struct room_s room_t;

#define NUM_MAPS 256
#define MAP_WIDTH 50
#define MAP_HEIGHT 50

typedef struct map_s
{
    game_t *game;
    int level;
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
    TCOD_list_t entities;
} map_t;

void map_init(map_t *map, game_t *game, int level);
void map_generate_custom(map_t *map);
void map_generate_bsp(map_t *map);
void hline(map_t *map, int x1, int y, int x2);
void hline_left(map_t *map, int x, int y);
void hline_right(map_t *map, int x, int y);
void vline(map_t *map, int x, int y1, int y2);
void vline_up(map_t *map, int x, int y);
void vline_down(map_t *map, int x, int y);
bool traverse_node(TCOD_bsp_t *node, map_t *map);
void map_populate(map_t *map);
bool map_is_inside(int x, int y);
room_t *map_get_random_room(map_t *map);
TCOD_map_t map_to_TCOD_map(map_t *map);
bool map_is_transparent(map_t *map, int x, int y);
bool map_is_walkable(map_t *map, int x, int y);
TCOD_map_t map_to_fov_map(map_t *map, int x, int y, int radius);
void map_reset(map_t *map);

#endif
