#ifndef MAP_H
#define MAP_H

#include "tile.h"

#define MAP_WIDTH 50
#define MAP_HEIGHT 50

typedef struct
{
    Tile tiles[MAP_WIDTH * MAP_HEIGHT];
    TCOD_list_t objects;
    TCOD_list_t actors;
    TCOD_list_t items;
} Map;

#endif
