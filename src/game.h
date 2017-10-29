#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

#include "world.h"

typedef struct tiledata_s
{
    tile_type_t type;
    bool seen;
} tiledata_t;

typedef struct roomdata_s
{
    int x;
    int y;
    int w;
    int h;
} roomdata_t;

typedef struct actordata_s
{
    actor_type_t type;
    int x;
    int y;
} actordata_t;

typedef struct mapdata_s
{
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    tiledata_t tiledata[MAP_WIDTH][MAP_HEIGHT];
    int room_count;
    roomdata_t *roomdata;
    int actor_count;
    actordata_t *actordata;
} mapdata_t;

typedef struct gamedata_s
{
    TCOD_random_t random;
    int map_count;
    mapdata_t *mapdata;
    int current_map_index;
} gamedata_t;

int current_map_index;
map_t *current_map;
actor_t *player;

void game_init(void);
void game_load(void);
void game_save(void);
void game_update(void);

#endif