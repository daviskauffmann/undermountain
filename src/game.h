#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

#include "world.h"

typedef struct
{
    tiletype_t type;
    bool seen;
} tiledata_t;

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} roomdata_t;

typedef struct
{
    bool is_player;
    int x;
    int y;
    unsigned char glyph;
    TCOD_color_t color;
    int sight_radius;
} actordata_t;

typedef struct
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

typedef struct
{
    int map_count;
    mapdata_t *mapdata;
} worlddata_t;

typedef struct
{
    TCOD_random_t random;
    int current_map_index;
    worlddata_t *worlddata;
} gamedata_t;

extern world_t *world;
extern int current_map_index;
extern map_t *current_map;
extern actor_t *player;

void game_init(void);
void game_load(void);
void game_save(void);

#endif