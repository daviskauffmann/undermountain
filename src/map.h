#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <libtcod.h>

#define MAP_WIDTH 80
#define MAP_HEIGHT 50
#define MAP_MAX_ROOMS 256
#define MAP_MAX_ACTORS 256

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
    bool is_active;
    uint8_t x;
    uint8_t y;
    uint8_t glyph;
    TCOD_color_t color;
} actor_t;

// TODO: use TCOD lists?
typedef struct
{
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    room_t rooms[MAP_MAX_ROOMS];
    actor_t actors[MAP_MAX_ACTORS];
} map_t;

extern tileinfo_t tileinfo[NB_TILETYPES];

void tileinfo_init(void);

void map_init(map_t *map);
void map_generate(map_t *map);
room_t *map_room_create(map_t *map, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
actor_t *map_actor_create(map_t *map, bool is_player, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color);
void map_update(map_t *map);
void map_actor_move(map_t *map, actor_t *actor, int dx, int dy);
void map_actor_change_map(map_t *mapFrom, map_t *mapTo, actor_t *actor);
TCOD_map_t map_actor_calc_fov(map_t *map, actor_t *actor);
void map_draw(map_t *map, actor_t *player);

#endif