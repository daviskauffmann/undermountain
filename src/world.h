#ifndef WORLD_H
#define WORLD_H

#include <libtcod.h>

#define MAP_WIDTH 100
#define MAP_HEIGHT 100

typedef enum {
    TILETYPE_EMPTY = 0,
    TILETYPE_FLOOR,
    TILETYPE_WALL,
    TILETYPE_STAIR_DOWN,
    TILETYPE_STAIR_UP,
    NUM_TILETYPES
} tiletype_t;

typedef struct
{
    tiletype_t type;
    bool seen;
} tile_t;

typedef struct
{
    int x;
    int y;
    int w;
    int h;
} room_t;

typedef enum {
    ACTORTYPE_NONE = 0,
    ACTORTYPE_PLAYER,
    ACTORTYPE_MONSTER,
    NUM_ACTORTYPES
} actortype_t;

typedef struct
{
    actortype_t type;
    int x;
    int y;
} actor_t;

typedef struct
{
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
    TCOD_list_t actors;
} map_t;

typedef struct
{
    TCOD_list_t maps;
} world_t;

void world_create(void);
void world_update(void);
void world_destroy(void);

#endif