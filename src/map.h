#ifndef MAP_H
#define MAP_H

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

map_t *map_create(void);
static bool traverse_node(TCOD_bsp_t *node, map_t *map);
static void vline(map_t *map, int x, int y1, int y2);
static void vline_up(map_t *map, int x, int y);
static void vline_down(map_t *map, int x, int y);
static void hline(map_t *map, int x1, int y, int x2);
static void hline_left(map_t *map, int x, int y);
static void hline_right(map_t *map, int x, int y);
room_t *map_get_random_room(map_t *map);
void room_get_random_pos(room_t *room, int *x, int *y);
TCOD_map_t map_to_TCOD_map(map_t *map);
void map_calc_fov(TCOD_map_t TCOD_map, int x, int y, int radius);
TCOD_path_t map_calc_path(TCOD_map_t TCOD_map, int ox, int oy, int dx, int dy);

#endif