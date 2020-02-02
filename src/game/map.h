#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <libtcod.h>

#include "tile.h"

#define MAP_WIDTH 50
#define MAP_HEIGHT 50

struct map
{
    unsigned int floor;
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    struct tile tiles[MAP_WIDTH][MAP_HEIGHT];
    int num_rooms;
    struct room *rooms;
    int num_objects;
    struct object *objects;
    TCOD_list_t actors;
    TCOD_list_t corpses;
    TCOD_list_t items;
    TCOD_list_t projectiles;
};

void map_init(struct map *map, unsigned int floor);
void map_reset(struct map *map);
void map_generate(struct map *map);
bool map_is_inside(int x, int y);
struct room *map_get_random_room(struct map *map);
int map_get_object_index(struct map *map, struct object *object);
struct object *map_get_object_at(struct map *map, int x, int y);
bool map_is_transparent(struct map *map, int x, int y);
bool map_is_walkable(struct map *map, int x, int y);
TCOD_map_t map_to_TCOD_map(struct map *map);
TCOD_map_t map_to_fov_map(struct map *map, int x, int y, int radius);

#endif
