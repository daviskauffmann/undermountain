#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include <libtcod.h>

#define MAP_WIDTH 80
#define MAP_HEIGHT 50

typedef enum {
    TILETYPE_EMPTY = 0,
    TILETYPE_FLOOR,
    TILETYPE_WALL,
    NB_TILETYPES
} tiletype_t;

typedef struct
{
    uint8_t glyph;
    TCOD_color_t color;
    bool is_transparent;
    bool is_walkable;
} tileinfo_t;

typedef struct
{
    tiletype_t type;
    bool seen;
} tile_t;

typedef struct
{
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
    bool is_created;
} room_t;

typedef struct
{
    bool is_player;
    uint8_t x;
    uint8_t y;
    uint8_t glyph;
    TCOD_color_t color;
    uint8_t sight_radius;
} actor_t;

typedef struct
{
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
    TCOD_list_t actors;
} map_t;

tileinfo_t tileinfo[NB_TILETYPES];

void tileinfo_init(void);

map_t *map_create();
void map_update(map_t *map);
void map_draw(map_t *map, actor_t *player);

static room_t *room_create(map_t *map, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

actor_t *actor_create(map_t *map, bool is_player, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color, uint8_t sight_radius);
static void actor_destroy(map_t *map, actor_t *actor);
void actor_move(map_t *map, actor_t *actor, int dx, int dy);
static void actor_change_map(map_t *mapFrom, map_t *mapTo, actor_t *actor);
static TCOD_map_t actor_calc_fov(map_t *map, actor_t *actor);

#endif