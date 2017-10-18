#ifndef MAP_H
#define MAP_H

#include "entity.h"
#include "tile.h"

#define MAP_WIDTH 80
#define MAP_HEIGHT 50
#define MAX_ENTITIES 255

typedef struct
{
    Tile tiles[MAP_WIDTH][MAP_HEIGHT];
    Entity entities[MAX_ENTITIES];
} Map;

void map_init(Map *map);
Entity *map_createEntity(Map *map, int x, int y, char glyph, TCOD_color_t color);
void map_generate(Map *map);
void map_update(Map *map);
void map_draw(Map *map);

#endif