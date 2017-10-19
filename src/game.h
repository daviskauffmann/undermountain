#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

#define ID_UNUSED -1
#define ID_PLAYER 254

#define MAP_WIDTH 80
#define MAP_HEIGHT 50
#define MAX_ENTITIES 255

typedef enum {
    TILETYPE_EMPTY = 0,
    TILETYPE_FLOOR,
    TILETYPE_WALL
} tiletype_t;

typedef struct
{
    tiletype_t type;
    bool seen;
} tile_t;

typedef struct
{
    int id;
    int x;
    int y;
    char glyph;
    TCOD_color_t color;
} entity_t;

typedef struct
{
    int x1;
    int y1;
    int x2;
    int y2;
} room_t;

typedef struct
{
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    entity_t entities[MAX_ENTITIES];
} map_t;

void tile_init(tile_t *tile, tiletype_t type);
void tile_draw(tile_t *tile, int x, int y);

void entity_init(entity_t *entity, int id, int x, int y, char glyph, TCOD_color_t color);
void entity_think(map_t *map, entity_t *entity);
void entity_move(map_t *map, entity_t *entity, int dx, int dy);
void entity_draw(entity_t *entity);

void map_init(map_t *map);
void map_generate(map_t *map);
void map_update(map_t *map);
void map_draw(map_t *map);

#endif