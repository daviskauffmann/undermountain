#ifndef UM_GAME_MAP_H
#define UM_GAME_MAP_H

#include "list.h"
#include "tile.h"
#include <libtcod.h>

#define MAP_WIDTH 50
#define MAP_HEIGHT 50

enum map_type
{
    MAP_TYPE_LARGE_DUNGEON,
    MAP_TYPE_SMALL_DUNGEON,
    MAP_TYPE_CAVES,
    MAP_TYPE_GRASSY_CAVES,
    MAP_TYPE_RUINS,

    NUM_MAP_TYPES,
};

struct map
{
    uint8_t floor;
    uint8_t stair_down_x;
    uint8_t stair_down_y;
    uint8_t stair_up_x;
    uint8_t stair_up_y;
    struct tile tiles[MAP_WIDTH][MAP_HEIGHT];
    struct list *rooms;
    struct list *objects;
    struct list *actors;
    struct list *corpses;
    struct list *items;
    struct list *projectiles;
    struct list *explosions;
    struct list *surfaces;
    int current_actor_index;
};

void map_init(struct map *map, uint8_t floor);
void map_uninit(struct map *map);
void map_generate(struct map *map, enum map_type map_type);
bool map_is_inside(int x, int y);
struct room *map_get_random_room(const struct map *map);
bool map_is_transparent(
    const struct map *map,
    int x,
    int y);
bool map_is_walkable(
    const struct map *map,
    int x,
    int y);
bool map_is_animation_playing(const struct map *map);
TCOD_Map *map_to_TCOD_map(const struct map *map);
TCOD_Map *map_to_fov_map(
    const struct map *map,
    int x,
    int y,
    int radius);

#endif
