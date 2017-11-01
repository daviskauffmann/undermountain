#ifndef WORLD_H
#define WORLD_H

#include <libtcod.h>

#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define BSP_DEPTH 10
#define MIN_ROOM_SIZE 5
#define FULL_ROOMS 0
#define NUM_ACTORS 50
#define LIT_ROOMS 0
#define SIMULATE_ALL_MAPS 1

typedef enum tile_type_e {
    TILETYPE_EMPTY = 0,
    TILETYPE_FLOOR,
    TILETYPE_WALL,
    TILETYPE_STAIR_DOWN,
    TILETYPE_STAIR_UP,
    NUM_TILETYPES
} tiletype_t;

typedef struct tile_info_s
{
    unsigned char glyph;
    TCOD_color_t light_color;
    TCOD_color_t dark_color;
    bool is_transparent;
    bool is_walkable;
} tileinfo_t;

typedef struct tile_s
{
    tiletype_t type;
    bool seen;
    struct actor_s *actor;
} tile_t;

typedef struct room_s
{
    int x;
    int y;
    int w;
    int h;
} room_t;

typedef enum actor_type_e {
    ACTORTYPE_NONE = 0,
    ACTORTYPE_PLAYER,
    ACTORTYPE_MONSTER,
    NUM_ACTORTYPES
} actortype_t;

typedef struct actor_info_s
{
    unsigned char glyph;
    TCOD_color_t color;
    int sight_radius;
} actorinfo_t;

typedef struct actor_s
{
    actortype_t type;
    struct map_s *map;
    int x;
    int y;
    TCOD_map_t fov_map;
    bool mark_for_delete;
} actor_t;

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
void world_turn(void);
void world_tick(void);
void world_destroy(void);

map_t *map_create(void);
void map_turn(map_t *map);
void map_tick(map_t *map);
room_t *map_get_random_room(map_t *map);
TCOD_map_t map_to_TCOD_map(map_t *map);
void map_destroy(map_t *map);

void tile_init(tile_t *tile, tiletype_t type, bool seen, actor_t *actor);
void tile_fini(tile_t *tile);

room_t *room_create(map_t *map, int x, int y, int w, int h);
void room_get_random_pos(room_t *room, int *x, int *y);
bool room_is_inside(room_t *room, int x, int y);
void room_destroy(room_t *room);

actor_t *actor_create(map_t *map, actortype_t type, int x, int y);
void actor_turn(actor_t *actor);
void actor_tick(actor_t *actor);
void actor_calc_fov(actor_t *actor);
bool actor_move_towards(actor_t *actor, int x, int y);
bool actor_move(actor_t *actor, int x, int y);
void actor_destroy(actor_t *actor);

#endif