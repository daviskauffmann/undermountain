#ifndef MAP_H
#define MAP_H

#include <libtcod.h>

#include "tile.h"
#include "room.h"

#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define BSP_DEPTH 10
#define MIN_ROOM_SIZE 5
#define FULL_ROOMS 1
#define NUM_LIGHTS 0
#define NUM_ACTORS 50
#define NUM_ITEMS 100

typedef struct map_s
{
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
    TCOD_list_t lights;
    TCOD_list_t actors;
} map_t;

map_t *map_create(void);
void map_turn(map_t *map);
void map_tick(map_t *map);
room_t *map_get_random_room(map_t *map);
TCOD_map_t map_to_TCOD_map(map_t *map);
void map_destroy(map_t *map);

#endif