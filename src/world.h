#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>
#include <libtcod.h>

#include "map.h"

typedef struct
{
    tiletype_t type;
    bool seen;
} tiledata_t;

typedef struct
{
    bool valid;
    bool is_player;
    uint8_t x;
    uint8_t y;
    uint8_t glyph;
    TCOD_color_t color;
    uint8_t sight_radius;
} actordata_t;

typedef struct
{
    bool valid;
    uint8_t stair_down_x;
    uint8_t stair_down_y;
    uint8_t stair_up_x;
    uint8_t stair_up_y;
    tiledata_t tiledata[MAP_WIDTH][MAP_HEIGHT];
    actordata_t actordata[255];
} mapdata_t;

typedef struct
{
    uint8_t current_map_index;
    TCOD_random_t random;
    mapdata_t mapdata[255];
} worlddata_t;

extern TCOD_list_t maps;
extern uint8_t current_map_index;
extern map_t *current_map;
extern actor_t *player;

void world_init(void);
void world_save(void);
void world_load(void);
void world_destroy(void);

#endif