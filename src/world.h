#ifndef WORLD_H
#define WORLD_H

#include <libtcod.h>

#define MAP_WIDTH 100
#define MAP_HEIGHT 100
#define BSP_DEPTH 10
#define MIN_ROOM_SIZE 5
#define NUM_ACTORS 50

typedef enum tiletype_e {
    TILETYPE_EMPTY = 0,
    TILETYPE_FLOOR,
    TILETYPE_WALL,
    TILETYPE_STAIR_DOWN,
    TILETYPE_STAIR_UP,
    NUM_TILETYPES
} tiletype_t;

typedef struct tile_s
{
    tiletype_t type;
    bool seen;
} tile_t;

typedef struct tileinfo_s
{
    unsigned char glyph;
    TCOD_color_t color;
    bool is_transparent;
    bool is_walkable;
} tileinfo_t;

typedef struct room_s
{
    int x;
    int y;
    int w;
    int h;
} room_t;

typedef enum actortype_e {
    ACTORTYPE_NONE = 0,
    ACTORTYPE_PLAYER,
    ACTORTYPE_MONSTER,
    NUM_ACTORTYPES
} actortype_t;

typedef struct actor_s
{
    actortype_t type;
    int x;
    int y;
} actor_t;

typedef struct actorinfo_s
{
    unsigned char glyph;
    TCOD_color_t color;
    int sight_radius;
} actorinfo_t;

typedef struct map_s
{
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
    TCOD_list_t actors;
} map_t;

TCOD_list_t maps;

void world_init(void);
void world_update(void);
void world_destroy(void);

map_t *map_create(void);
void map_update(map_t *map);
room_t *map_get_random_room(map_t *map);
TCOD_map_t map_to_TCOD_map(map_t *map);
TCOD_map_t map_calc_fov(TCOD_map_t TCOD_map, int x, int y, int radius);
TCOD_path_t map_calc_path(TCOD_map_t TCOD_map, int ox, int oy, int dx, int dy);
void map_destroy(map_t *map);

void room_get_random_pos(room_t *room, int *x, int *y);

actor_t *actor_create(map_t *map, actortype_t type, int x, int y, unsigned char glyph, TCOD_color_t color, int sight_radius);
void actor_update(map_t *map, actor_t *actor);
void actor_move(map_t *map, actor_t *actor, int x, int y);
void actor_destroy(map_t *map, actor_t *actor);

#endif