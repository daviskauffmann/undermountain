#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

#include "map.h"
#include "actor.h"

typedef struct tiledata_s
{
    tiletype_t type;
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
    actortype_t type;
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

extern TCOD_list_t maps;
extern int current_map_index;
extern map_t *current_map;
extern actor_t *player;

void game_init(void);
void game_load(void);
void game_save(void);
void game_update(void);
static void map_update(map_t *map);
static void actor_update(map_t *map, actor_t *actor);
void game_destroy(void);

#endif