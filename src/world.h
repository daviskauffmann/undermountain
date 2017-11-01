#ifndef WORLD_H
#define WORLD_H

#include <libtcod.h>

#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define BSP_DEPTH 10
#define MIN_ROOM_SIZE 5
#define FULL_ROOMS 1
#define NUM_ACTORS 50
#define LIT_ROOMS 0
#define SIMULATE_ALL_MAPS 1

typedef enum tile_type_e {
    TILE_EMPTY = 0,
    TILE_FLOOR,
    TILE_WALL,
    TILE_STAIR_DOWN,
    TILE_STAIR_UP,
    NUM_TILE_TYPES
} tile_type_t;

typedef struct tile_info_s
{
    unsigned char glyph;
    TCOD_color_t light_color;
    TCOD_color_t dark_color;
    bool is_transparent;
    bool is_walkable;
} tile_info_t;

typedef struct tile_s
{
    tile_type_t type;
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
    ACTOR_NONE = 0,
    ACTOR_PLAYER,
    ACTOR_MONSTER,
    NUM_ACTOR_TYPES
} actor_type_t;

typedef struct actor_info_s
{
    unsigned char glyph;
    TCOD_color_t color;
    int sight_radius;
} actor_info_t;

typedef struct actor_s
{
    actor_type_t type;
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

void world_initialize(void);
void world_turn(void);
void world_tick(void);
void world_finalize(void);

map_t *map_create(void);
void map_turn(map_t *map);
void map_tick(map_t *map);
room_t *map_get_random_room(map_t *map);
TCOD_map_t map_to_TCOD_map(map_t *map);
void map_destroy(map_t *map);

void tile_initialize(tile_t *tile, tile_type_t type, bool seen, actor_t *actor);
void tile_finalize(tile_t *tile);

room_t *room_create(map_t *map, int x, int y, int w, int h);
void room_get_random_pos(room_t *room, int *x, int *y);
bool room_is_inside(room_t *room, int x, int y);
void room_destroy(room_t *room);

actor_t *actor_create(map_t *map, actor_type_t type, int x, int y);
void actor_turn(actor_t *actor);
void actor_tick(actor_t *actor);
void actor_calc_fov(actor_t *actor);
bool actor_move_towards(actor_t *actor, int x, int y);
bool actor_move(actor_t *actor, int x, int y);
void actor_destroy(actor_t *actor);

#endif