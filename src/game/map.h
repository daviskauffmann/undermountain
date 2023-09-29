#ifndef UM_GAME_MAP_H
#define UM_GAME_MAP_H

#include "actor.h"
#include "list.h"
#include "tile.h"
#include <libtcod.h>

#define MAP_WIDTH 50
#define MAP_HEIGHT 50

enum monster_pack
{
    MONSTER_PACK_BATS,
    MONSTER_PACK_BUGBEARS,
    MONSTER_PACK_DIRE_RAT,
    MONSTER_PACK_RED_DRAGON,
    MONSTER_PACK_GOBLINS,
    MONSTER_PACK_KOBOLDS,
    MONSTER_PACK_RATS,
    MONSTER_PACK_TROLLS,
    MONSTER_PACK_SKELETONS,

    NUM_MONSTER_PACKS,
};

struct monster_pack_data
{
    int min_floor;
    int max_floor;

    struct
    {
        int min_count;
        int max_count;
    } monsters[NUM_MONSTERS];
};

struct map
{
    int floor;
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
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

void map_init(struct map *map, int floor);
void map_uninit(struct map *map);

void map_generate(struct map *map);

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
