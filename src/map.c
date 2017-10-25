#include <stdio.h>
#include <stdint.h>
#include <libtcod.h>

#include "config.h"
#include "game.h"
#include "world.h"

#define BSP_DEPTH 10
#define MIN_ROOM_SIZE 5
// TODO: fix this when false
#define FULL_ROOMS true

bool traverse_node(TCOD_bsp_t *node, map_t *map);
void vline(map_t *map, int x, int y1, int y2);
void vline_up(map_t *map, int x, int y);
void vline_down(map_t *map, int x, int y);
void hline(map_t *map, int x1, int y, int x2);
void hline_left(map_t *map, int x, int y);
void hline_right(map_t *map, int x, int y);

map_t *map_create()
{
    map_t *map = (map_t *)malloc(sizeof(map_t));

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];
            tile->type = TILETYPE_WALL;
            tile->seen = false;
        }
    }

    map->rooms = TCOD_list_new();
    TCOD_bsp_t *bsp = TCOD_bsp_new_with_size(0, 0, MAP_WIDTH, MAP_HEIGHT);
    TCOD_bsp_split_recursive(bsp, NULL, BSP_DEPTH, MIN_ROOM_SIZE + 1, MIN_ROOM_SIZE + 1, 1.5f, 1.5f);
    TCOD_bsp_traverse_inverted_level_order(bsp, traverse_node, map);
    TCOD_bsp_delete(bsp);

    room_t *stair_down_room = TCOD_list_get(map->rooms, TCOD_random_get_int(NULL, 0, TCOD_list_size(map->rooms) - 1));
    map->stair_down_x = TCOD_random_get_int(NULL, stair_down_room->x, stair_down_room->x + stair_down_room->w - 1);
    map->stair_down_y = TCOD_random_get_int(NULL, stair_down_room->y, stair_down_room->y + stair_down_room->h - 1);
    tile_t *stair_down_tile = &map->tiles[map->stair_down_x][map->stair_down_y];
    stair_down_tile->type = TILETYPE_STAIR_DOWN;

    room_t *stair_up_room = TCOD_list_get(map->rooms, TCOD_random_get_int(NULL, 0, TCOD_list_size(map->rooms) - 1));
    map->stair_up_x = TCOD_random_get_int(NULL, stair_up_room->x, stair_up_room->x + stair_up_room->w - 1);
    map->stair_up_y = TCOD_random_get_int(NULL, stair_up_room->y, stair_up_room->y + stair_up_room->h - 1);
    tile_t *stair_up_tile = &map->tiles[map->stair_up_x][map->stair_up_y];
    stair_up_tile->type = TILETYPE_STAIR_UP;

    map->actors = TCOD_list_new();
    for (int i = 0; i < 0; i++)
    {
        room_t *actor_room = TCOD_list_get(map->rooms, TCOD_random_get_int(NULL, 0, TCOD_list_size(map->rooms) - 1));

        if (actor_room == stair_up_room)
        {
            continue;
        }

        uint8_t x = TCOD_random_get_int(NULL, actor_room->x, actor_room->x + actor_room->w - 1);
        uint8_t y = TCOD_random_get_int(NULL, actor_room->y, actor_room->y + actor_room->h - 1);
        actor_create(map, x, y, '@', TCOD_yellow, 10);
    }

    TCOD_list_push(world->maps, map);

    return map;
}

bool traverse_node(TCOD_bsp_t *node, map_t *map)
{
    if (TCOD_bsp_is_leaf(node))
    {
        int min_x = node->x + 1;
        int max_x = node->x + node->w - 1;
        int min_y = node->y + 1;
        int max_y = node->y + node->h - 1;

        if (max_x == MAP_WIDTH - 1)
        {
            max_x--;
        }
        if (max_y == MAP_HEIGHT - 1)
        {
            max_y--;
        }

        if (!FULL_ROOMS)
        {
            min_x = TCOD_random_get_int(NULL, min_x, max_x - MIN_ROOM_SIZE + 1);
            min_y = TCOD_random_get_int(NULL, min_y, max_y - MIN_ROOM_SIZE + 1);
            max_x = TCOD_random_get_int(NULL, min_x + MIN_ROOM_SIZE - 2, max_x);
            max_y = TCOD_random_get_int(NULL, min_y + MIN_ROOM_SIZE - 2, max_y);
        }

        node->x = min_x;
        node->y = min_y;
        node->w = max_x - min_x + 1;
        node->h = max_y - min_y + 1;

        for (int x = min_x; x < max_x + 1; x++)
        {
            for (int y = min_y; y < max_y + 1; y++)
            {
                tile_t *tile = &map->tiles[x][y];
                tile->type = TILETYPE_FLOOR;
            }
        }

        room_t *room = (room_t *)malloc(sizeof(room_t));

        room->x = node->x;
        room->y = node->y;
        room->w = node->w;
        room->h = node->h;

        TCOD_list_push(map->rooms, room);
    }
    else
    {
        TCOD_bsp_t *left = TCOD_bsp_left(node);
        TCOD_bsp_t *right = TCOD_bsp_right(node);

        node->x = left->x < right->x ? left->x : right->x;
        node->y = left->y < right->y ? left->y : right->y;
        node->w = (left->x + left->w > right->x + right->w ? left->x + left->w : right->x + right->w) - node->x;
        node->h = (left->y + left->h > right->y + right->h ? left->y + left->h : right->y + right->h) - node->y;

        if (node->horizontal)
        {
            if (left->x + left->w - 1 < right->x || right->x + right->w - 1 < left->x)
            {
                int x1 = TCOD_random_get_int(NULL, left->x, left->x + left->w - 1);
                int x2 = TCOD_random_get_int(NULL, right->x, right->x + right->w - 1);
                int y = TCOD_random_get_int(NULL, left->y + left->h, right->y);

                vline_up(map, x1, y - 1);
                hline(map, x1, y, x2);
                vline_down(map, x2, y + 1);
            }
            else
            {
                int min_x = left->x > right->x ? left->x : right->x;
                int max_x = left->x + left->w - 1 < right->x + right->w - 1 ? left->x + left->w - 1 : right->x + right->w - 1;
                int x = TCOD_random_get_int(NULL, min_x, max_x);

                while (x > MAP_WIDTH - 1)
                {
                    x--;
                }

                vline_down(map, x, right->y);
                vline_up(map, x, right->y - 1);
            }
        }
        else
        {
            if (left->y + left->h - 1 < right->y || right->y + right->h - 1 < left->y)
            {
                int y1 = TCOD_random_get_int(NULL, left->y, left->y + left->h - 1);
                int y2 = TCOD_random_get_int(NULL, right->y, right->y + right->h - 1);
                int x = TCOD_random_get_int(NULL, left->x + left->w, right->x);

                hline_left(map, x - 1, y1);
                vline(map, x, y1, y2);
                hline_right(map, x + 1, y2);
            }
            else
            {
                int min_y = left->y > right->y ? left->y : right->y;
                int max_y = left->y + left->h - 1 < right->y + right->h - 1 ? left->y + left->h - 1 : right->y + right->h - 1;
                int y = TCOD_random_get_int(NULL, min_y, max_y);

                while (y > MAP_HEIGHT - 1)
                {
                    y--;
                }

                hline_left(map, right->x - 1, y);
                hline_right(map, right->x, y);
            }
        }
    }

    return true;
}

void vline(map_t *map, int x, int y1, int y2)
{
    if (y1 > y2)
    {
        int t = y1;
        y1 = y2;
        y2 = t;
    }

    for (int y = y1; y < y2 + 1; y++)
    {
        tile_t *tile = &map->tiles[x][y];
        tile->type = TILETYPE_FLOOR;
    }
}

void vline_up(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y >= 0 && tile->type != TILETYPE_FLOOR)
    {
        tile->type = TILETYPE_FLOOR;
        y--;
    }
}

void vline_down(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y < MAP_HEIGHT && tile->type != TILETYPE_FLOOR)
    {
        tile->type = TILETYPE_FLOOR;
        y++;
    }
}

void hline(map_t *map, int x1, int y, int x2)
{
    if (x1 > x2)
    {
        int t = x1;
        x1 = x2;
        x2 = t;
    }

    for (int x = x1; x < x2 + 1; x++)
    {
        tile_t *tile = &map->tiles[x][y];
        tile->type = TILETYPE_FLOOR;
    }
}

void hline_left(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];
    while (x >= 0 && tile->type != TILETYPE_FLOOR)
    {
        tile->type = TILETYPE_FLOOR;
        x--;
    }
}

void hline_right(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];
    while (x < MAP_WIDTH && tile->type != TILETYPE_FLOOR)
    {
        tile->type = TILETYPE_FLOOR;
        x++;
    }
}

TCOD_map_t map_to_TCOD_map(map_t *map)
{
    TCOD_map_t TCOD_map = TCOD_map_new(MAP_WIDTH, MAP_HEIGHT);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            TCOD_map_set_properties(TCOD_map, x, y, tileinfo[tile->type].is_transparent, tileinfo[tile->type].is_walkable);
        }
    }

    for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
         iterator != (actor_t **)TCOD_list_end(map->actors);
         iterator++)
    {
        actor_t *actor = *iterator;

        TCOD_map_set_properties(TCOD_map, actor->x, actor->y, TCOD_map_is_transparent(TCOD_map, actor->x, actor->y), false);
    }

    return TCOD_map;
}

void map_calc_fov(TCOD_map_t TCOD_map, int x, int y, int radius)
{
    TCOD_map_compute_fov(TCOD_map, x, y, radius, true, FOV_DIAMOND);
}

TCOD_path_t map_calc_path(TCOD_map_t TCOD_map, int ox, int oy, int dx, int dy)
{
    TCOD_map_set_properties(TCOD_map, dx, dy, TCOD_map_is_transparent(TCOD_map, dx, dy), true);

    TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1.0f);
    TCOD_path_compute(path, ox, oy, dx, dy);

    return path;
}