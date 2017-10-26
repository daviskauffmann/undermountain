#ifndef MAP_H
#define MAP_H

#include <libtcod.h>

#include "world.h"

#define BSP_DEPTH 10
#define MIN_ROOM_SIZE 5
#define NUM_ACTORS 50

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