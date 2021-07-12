#ifndef MAP_H
#define MAP_H

#include <libtcod.h>

#include "tile.h"

#define MAP_WIDTH 50
#define MAP_HEIGHT 50

enum map_type
{
    MAP_TYPE_TOWN,
    MAP_TYPE_LARGE_DUNGEON,
    MAP_TYPE_SMALL_DUNGEON,
    MAP_TYPE_CAVES,
    MAP_TYPE_RUINS,

    NUM_MAP_TYPES,
};

struct map
{
    unsigned int floor;
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    struct tile tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
    TCOD_list_t objects;
    TCOD_list_t actors;
    TCOD_list_t corpses;
    TCOD_list_t items;
    TCOD_list_t projectiles;
    TCOD_list_t explosions;
    int current_actor_index;
};

void map_setup(struct map *map, unsigned int floor);
void map_cleanup(struct map *map);
void map_generate(struct map *map, enum map_type map_type);
bool map_is_inside(int x, int y);
struct room *map_get_random_room(struct map *map);
bool map_is_transparent(struct map *map, int x, int y);
bool map_is_walkable(struct map *map, int x, int y);
TCOD_map_t map_to_TCOD_map(struct map *map);
TCOD_map_t map_to_fov_map(struct map *map, int x, int y, int radius);

#endif
