#ifndef MAP_H
#define MAP_H

#include "tile.h"

#define MAP_WIDTH 50
#define MAP_HEIGHT 50

struct map
{
    int level;
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    struct tile tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
    TCOD_list_t objects;
    TCOD_list_t actors;
    TCOD_list_t items;
    TCOD_list_t projectiles;
};

void map_init(struct map *map, int level);
void map_generate(struct map *map);
bool map_is_inside(int x, int y);
struct room *map_get_random_room(struct map *map);
bool map_is_transparent(struct map *map, int x, int y);
bool map_is_walkable(struct map *map, int x, int y);
TCOD_map_t map_to_TCOD_map(struct map *map);
TCOD_map_t map_to_fov_map(struct map *map, int x, int y, int radius);
void map_reset(struct map *map);

#endif
