#include <libtcod/libtcod.h>

#include "game.h"
#include "map.h"
#include "tile.h"

#define CUSTOM_NUM_ROOM_ATTEMPTS 20
#define CUSTOM_MIN_ROOM_SIZE 5
#define CUSTOM_MAX_ROOM_SIZE 15
#define CUSTOM_PREVENT_OVERLAP 0
#define CUSTOM_DOOR_CHANCE 0.5

#define BSP_MIN_ROOM_SIZE 4
#define BSP_DEPTH 8
#define BSP_RANDOM_ROOMS 0
#define BSP_ROOM_WALLS 1

#define NUM_MONSTERS 20
#define NUM_ADVENTURERS 5
#define NUM_ITEMS 5
#define NUM_BRAZIERS 5

static void hline(struct map *map, int x1, int y, int x2);
static void hline_left(struct map *map, int x, int y);
static void hline_right(struct map *map, int x, int y);
static void vline(struct map *map, int x, int y1, int y2);
static void vline_up(struct map *map, int x, int y);
static void vline_down(struct map *map, int x, int y);
static bool traverse_node(TCOD_bsp_t *node, struct map *map);

void map_init(struct map *map, struct game *game, int level)
{
    map->game = game;
    map->level = level;

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            struct tile *tile = &map->tiles[x + y * MAP_WIDTH];

            tile_init(tile, TILE_EMPTY, false);
        }
    }

    map->objects = TCOD_list_new();
    map->actors = TCOD_list_new();
    map->items = TCOD_list_new();
}

void map_reset(struct map *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_reset(&map->tiles[x + y * MAP_WIDTH]);
        }
    }

    TCOD_list_delete(map->objects);
    TCOD_list_delete(map->actors);
    TCOD_list_delete(map->items);
}
