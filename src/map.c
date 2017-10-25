#include <stdio.h>
#include <libtcod.h>

#include "map.h"
#include "config.h"
#include "game.h"
#include "world.h"
#include "actor.h"

map_t *map_create(void)
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

    room_t *stair_down_room = map_get_random_room(map);
    room_get_random_pos(stair_down_room, &map->stair_down_x, &map->stair_down_y);
    map->tiles[map->stair_down_x][map->stair_down_y].type = TILETYPE_STAIR_DOWN;

    room_t *stair_up_room = map_get_random_room(map);
    room_get_random_pos(stair_up_room, &map->stair_up_x, &map->stair_up_y);
    map->tiles[map->stair_up_x][map->stair_up_y].type = TILETYPE_STAIR_UP;

    map->actors = TCOD_list_new();
    for (int i = 0; i < 20; i++)
    {
        room_t *actor_room = map_get_random_room(map);

        if (actor_room == stair_up_room)
        {
            continue;
        }

        actor_t *actor = actor_create(map, 0, 0, '@', TCOD_yellow, 10);
        // room_get_random_pos(actor_room, &actor->x, &actor->y);
    }

    TCOD_list_push(world->maps, map);

    return map;
}

static bool traverse_node(TCOD_bsp_t *node, map_t *map)
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

static void vline(map_t *map, int x, int y1, int y2)
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

static void vline_up(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y >= 0 && tile->type != TILETYPE_FLOOR)
    {
        tile->type = TILETYPE_FLOOR;
        y--;
    }
}

static void vline_down(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y < MAP_HEIGHT && tile->type != TILETYPE_FLOOR)
    {
        tile->type = TILETYPE_FLOOR;
        y++;
    }
}

static void hline(map_t *map, int x1, int y, int x2)
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

static void hline_left(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];
    while (x >= 0 && tile->type != TILETYPE_FLOOR)
    {
        tile->type = TILETYPE_FLOOR;
        x--;
    }
}

static void hline_right(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];
    while (x < MAP_WIDTH && tile->type != TILETYPE_FLOOR)
    {
        tile->type = TILETYPE_FLOOR;
        x++;
    }
}

room_t *map_get_random_room(map_t *map)
{
    return TCOD_list_get(map->rooms, TCOD_random_get_int(NULL, 0, TCOD_list_size(map->rooms) - 1));
}

void room_get_random_pos(room_t *room, int *x, int *y)
{
    *x = TCOD_random_get_int(NULL, room->x, room->x + room->w - 1);
    *y = TCOD_random_get_int(NULL, room->y, room->y + room->h - 1);
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