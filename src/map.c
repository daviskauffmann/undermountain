#include <libtcod/libtcod.h>

#include "game.h"
#include "map.h"
#include "room.h"
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

    map->rooms = TCOD_list_new();
    map->objects = TCOD_list_new();
    map->actors = TCOD_list_new();
    map->items = TCOD_list_new();
}

void map_generate_custom(struct map *map)
{
    for (int i = 0; i < CUSTOM_NUM_ROOM_ATTEMPTS; i++)
    {
        struct room *room = room_create(
            TCOD_random_get_int(NULL, 0, MAP_WIDTH),
            TCOD_random_get_int(NULL, 0, MAP_HEIGHT),
            TCOD_random_get_int(NULL, CUSTOM_MIN_ROOM_SIZE, CUSTOM_MAX_ROOM_SIZE),
            TCOD_random_get_int(NULL, CUSTOM_MIN_ROOM_SIZE, CUSTOM_MAX_ROOM_SIZE));

        if (room->x < 2 ||
            room->x + room->w > MAP_WIDTH - 2 ||
            room->y < 2 ||
            room->y + room->h > MAP_HEIGHT - 2)
        {
            continue;
        }

#if CUSTOM_PREVENT_OVERLAP
        bool overlap = false;

        for (int x = room->x - 2; x < room->x + room->w + 2; x++)
        {
            for (int y = room->y - 2; y < room->y + room->h + 2; y++)
            {
                struct tile *tile = &map->tiles[x + y * MAP_WIDTH];

                if (tile->type == TILE_FLOOR)
                {
                    overlap = true;
                }
            }
        }

        if (overlap)
        {
            continue;
        }
#endif

        for (int x = room->x; x < room->x + room->w; x++)
        {
            for (int y = room->y; y < room->y + room->h; y++)
            {
                struct tile *tile = &map->tiles[x + y * MAP_WIDTH];

                tile->type = TILE_FLOOR;
            }
        }

        TCOD_list_push(map->rooms, room);
    }

    for (int i = 0; i < TCOD_list_size(map->rooms) - 1; i++)
    {
        struct room *room = TCOD_list_get(map->rooms, i);
        struct room *next_room = TCOD_list_get(map->rooms, i + 1);

        int x1 = TCOD_random_get_int(NULL, room->x, room->x + room->w);
        int y1 = TCOD_random_get_int(NULL, room->y, room->y + room->h);
        int x2 = TCOD_random_get_int(NULL, next_room->x, next_room->x + next_room->w);
        int y2 = TCOD_random_get_int(NULL, next_room->y, next_room->y + next_room->h);

        if (x1 > x2)
        {
            int t = x1;
            x1 = x2;
            x2 = t;
        }
        if (y1 > y2)
        {
            int t = y1;
            y1 = y2;
            y2 = t;
        }

        for (int x = x1; x <= x2; x++)
        {
            for (int y = y1; y <= y2; y++)
            {
                if (x == x1 || x == x2 || y == y1 || y == y2)
                {
                    struct tile *tile = &map->tiles[x + y * MAP_WIDTH];

                    tile->type = TILE_FLOOR;
                }
            }
        }
    }

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            struct tile *tile = &map->tiles[x + y * MAP_WIDTH];

            if (tile->type == TILE_FLOOR)
            {
                struct tile *neighbors[] = {
                    &map->tiles[(x + 0) + (y - 1) * MAP_WIDTH],
                    &map->tiles[(x + 1) + (y - 1) * MAP_WIDTH],
                    &map->tiles[(x + 1) + (y + 0) * MAP_WIDTH],
                    &map->tiles[(x + 1) + (y + 1) * MAP_WIDTH],
                    &map->tiles[(x + 0) + (y + 1) * MAP_WIDTH],
                    &map->tiles[(x - 1) + (y + 1) * MAP_WIDTH],
                    &map->tiles[(x - 1) + (y + 0) * MAP_WIDTH],
                    &map->tiles[(x - 1) + (y - 1) * MAP_WIDTH]};

                for (int i = 0; i < 8; i++)
                {
                    struct tile *neighbor = neighbors[i];

                    if (neighbor->type == TILE_EMPTY)
                    {
                        neighbor->type = TILE_WALL;
                    }
                }
            }
        }
    }

    // for (int x = 0; x < MAP_WIDTH; x++)
    // {
    //     for (int y = 0; y < MAP_HEIGHT; y++)
    //     {
    //         if (map->tiles[x][y].type == TILE_FLOOR && TCOD_random_get_float(NULL, 0, 1) < CUSTOM_DOOR_CHANCE)
    //         {
    //             if (map->tiles[x][y - 1].type == TILE_FLOOR && map->tiles[x + 1][y - 1].type == TILE_FLOOR && map->tiles[x - 1][y - 1].type == TILE_FLOOR)
    //             {
    //                 if (map->tiles[x - 1][y].type == TILE_WALL && map->tiles[x + 1][y].type == TILE_WALL)
    //                 {
    //                     map->tiles[x][y].type = TILE_DOOR_CLOSED;
    //                 }
    //             }
    //             if (map->tiles[x + 1][y].type == TILE_FLOOR && map->tiles[x + 1][y - 1].type == TILE_FLOOR && map->tiles[x + 1][y + 1].type == TILE_FLOOR)
    //             {
    //                 if (map->tiles[x][y + 1].type == TILE_WALL && map->tiles[x][y - 1].type == TILE_WALL)
    //                 {
    //                     map->tiles[x][y].type = TILE_DOOR_CLOSED;
    //                 }
    //             }
    //             if (map->tiles[x][y + 1].type == TILE_FLOOR && map->tiles[x + 1][y + 1].type == TILE_FLOOR && map->tiles[x - 1][y + 1].type == TILE_FLOOR)
    //             {
    //                 if (map->tiles[x - 1][y].type == TILE_WALL && map->tiles[x + 1][y].type == TILE_WALL)
    //                 {
    //                     map->tiles[x][y].type = TILE_DOOR_CLOSED;
    //                 }
    //             }
    //             if (map->tiles[x - 1][y].type == TILE_FLOOR && map->tiles[x - 1][y - 1].type == TILE_FLOOR && map->tiles[x - 1][y + 1].type == TILE_FLOOR)
    //             {
    //                 if (map->tiles[x][y + 1].type == TILE_WALL && map->tiles[x][y - 1].type == TILE_WALL)
    //                 {
    //                     map->tiles[x][y].type = TILE_DOOR_CLOSED;
    //                 }
    //             }
    //         }
    //     }
    // }

    struct room *stair_down_room = map_get_random_room(map);
    room_get_random_pos(stair_down_room, &map->stair_down_x, &map->stair_down_y);
    map->tiles[map->stair_down_x + map->stair_down_y * MAP_WIDTH].type = TILE_STAIR_DOWN;

    struct room *stair_up_room = map_get_random_room(map);
    room_get_random_pos(stair_up_room, &map->stair_up_x, &map->stair_up_y);
    map->tiles[map->stair_up_x + map->stair_up_y * MAP_WIDTH].type = TILE_STAIR_UP;
}

void map_generate_bsp(struct map *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            struct tile *tile = &map->tiles[x + y * MAP_WIDTH];

            tile->type = TILE_WALL;
        }
    }

    TCOD_bsp_t *bsp = TCOD_bsp_new_with_size(0, 0, MAP_WIDTH, MAP_HEIGHT);
    TCOD_bsp_split_recursive(
        bsp,
        NULL,
        BSP_DEPTH,
        BSP_MIN_ROOM_SIZE + (BSP_ROOM_WALLS ? 1 : 0),
        BSP_MIN_ROOM_SIZE + (BSP_ROOM_WALLS ? 1 : 0),
        1.5f,
        1.5f);
    TCOD_bsp_traverse_inverted_level_order(bsp, traverse_node, map);
    TCOD_bsp_delete(bsp);

    struct room *stair_down_room = map_get_random_room(map);
    room_get_random_pos(stair_down_room, &map->stair_down_x, &map->stair_down_y);
    map->tiles[map->stair_down_x + map->stair_down_y * MAP_WIDTH].type = TILE_STAIR_DOWN;

    struct room *stair_up_room = map_get_random_room(map);
    room_get_random_pos(stair_up_room, &map->stair_up_x, &map->stair_up_y);
    map->tiles[map->stair_up_x + map->stair_up_y * MAP_WIDTH].type = TILE_STAIR_UP;
}

void map_populate(struct map *map)
{
}

bool map_is_inside(int x, int y)
{
    return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
}

struct room *map_get_random_room(struct map *map)
{
    return TCOD_list_get(map->rooms, TCOD_random_get_int(NULL, 0, TCOD_list_size(map->rooms) - 1));
}

bool map_is_transparent(struct map *map, int x, int y)
{
    struct tile *tile = &map->tiles[x + y * MAP_WIDTH];

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (!map->game->object_info[object->type].is_transparent)
        {
            return false;
        }
    }

    return map->game->tile_info[tile->type].is_transparent;
}

bool map_is_walkable(struct map *map, int x, int y)
{
    struct tile *tile = &map->tiles[x + y * MAP_WIDTH];

    if (TCOD_list_size(tile->actors) > 0)
    {
        return false;
    }

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (!map->game->object_info[object->type].is_walkable)
        {
            return false;
        }
    }

    return map->game->tile_info[tile->type].is_walkable;
}

TCOD_map_t map_to_TCOD_map(struct map *map)
{
    TCOD_map_t TCOD_map = TCOD_map_new(MAP_WIDTH, MAP_HEIGHT);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            TCOD_map_set_properties(TCOD_map, x, y, map_is_transparent(map, x, y), map_is_walkable(map, x, y));
        }
    }

    return TCOD_map;
}

TCOD_map_t map_to_fov_map(struct map *map, int x, int y, int radius)
{
    TCOD_map_t fov_map = map_to_TCOD_map(map);

    TCOD_map_compute_fov(fov_map, x, y, radius, true, FOV_BASIC);

    return fov_map;
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

    TCOD_list_delete(map->rooms);
    TCOD_list_delete(map->objects);
    TCOD_list_delete(map->actors);
    TCOD_list_delete(map->items);
}

static void hline(struct map *map, int x1, int y, int x2)
{
    int x = x1;
    int dx = (x1 > x2 ? -1 : 1);

    map->tiles[x + y * MAP_WIDTH].type = TILE_FLOOR;

    if (x1 != x2)
    {
        do
        {
            x += dx;

            map->tiles[x + y * MAP_WIDTH].type = TILE_FLOOR;
        } while (x != x2);
    }
}

static void hline_left(struct map *map, int x, int y)
{
    while (x >= 0 && map->tiles[x + y * MAP_WIDTH].type != TILE_FLOOR)
    {
        map->tiles[x + y * MAP_WIDTH].type = TILE_FLOOR;

        x--;
    }
}

static void hline_right(struct map *map, int x, int y)
{
    while (x < MAP_WIDTH && map->tiles[x + y * MAP_WIDTH].type != TILE_FLOOR)
    {
        map->tiles[x + y * MAP_WIDTH].type = TILE_FLOOR;

        x++;
    }
}

static void vline(struct map *map, int x, int y1, int y2)
{
    int y = y1;
    int dy = (y1 > y2 ? -1 : 1);

    map->tiles[x + y * MAP_WIDTH].type = TILE_FLOOR;

    if (y1 != y2)
    {
        do
        {
            y += dy;

            map->tiles[x + y * MAP_WIDTH].type = TILE_FLOOR;
        } while (y != y2);
    }
}

static void vline_up(struct map *map, int x, int y)
{
    while (y >= 0 && map->tiles[x + y * MAP_WIDTH].type != TILE_FLOOR)
    {
        map->tiles[x + y * MAP_WIDTH].type = TILE_FLOOR;

        y--;
    }
}

static void vline_down(struct map *map, int x, int y)
{
    while (y < MAP_HEIGHT && map->tiles[x + y * MAP_WIDTH].type != TILE_FLOOR)
    {
        map->tiles[x + y * MAP_WIDTH].type = TILE_FLOOR;

        y++;
    }
}

static bool traverse_node(TCOD_bsp_t *node, struct map *map)
{
    if (TCOD_bsp_is_leaf(node))
    {
        int minx = node->x + 1;
        int maxx = node->x + node->w - 1;
        int miny = node->y + 1;
        int maxy = node->y + node->h - 1;

#if !BSP_ROOM_WALLS
        if (minx > 1)
        {
            minx--;
        }

        if (miny > 1)
        {
            miny--;
        }
#endif

        if (maxx == MAP_WIDTH - 1)
        {
            maxx--;
        }

        if (maxy == MAP_HEIGHT - 1)
        {
            maxy--;
        }

#if BSP_RANDOM_ROOMS
        minx = TCOD_random_get_int(NULL, minx, maxx - BSP_MIN_ROOM_SIZE + 1);
        miny = TCOD_random_get_int(NULL, miny, maxy - BSP_MIN_ROOM_SIZE + 1);
        maxx = TCOD_random_get_int(NULL, minx + BSP_MIN_ROOM_SIZE - 1, maxx);
        maxy = TCOD_random_get_int(NULL, miny + BSP_MIN_ROOM_SIZE - 1, maxy);
#endif

        node->x = minx;
        node->y = miny;
        node->w = maxx - minx + 1;
        node->h = maxy - miny + 1;

        for (int x = minx; x <= maxx; x++)
        {
            for (int y = miny; y <= maxy; y++)
            {
                map->tiles[x + y * MAP_WIDTH].type = TILE_FLOOR;
            }
        }

        struct room *room = room_create(node->x, node->y, node->w, node->h);

        TCOD_list_push(map->rooms, room);
    }
    else
    {
        TCOD_bsp_t *left = TCOD_bsp_left(node);
        TCOD_bsp_t *right = TCOD_bsp_right(node);

        node->x = MIN(left->x, right->x);
        node->y = MIN(left->y, right->y);
        node->w = MAX(left->x + left->w, right->x + right->w) - node->x;
        node->h = MAX(left->y + left->h, right->y + right->h) - node->y;

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
                int minx = MAX(left->x, right->x);
                int maxx = MIN(left->x + left->w - 1, right->x + right->w - 1);
                int x = TCOD_random_get_int(NULL, minx, maxx);

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
                int miny = MAX(left->y, right->y);
                int maxy = MIN(left->y + left->h - 1, right->y + right->h - 1);
                int y = TCOD_random_get_int(NULL, miny, maxy);

                hline_left(map, right->x - 1, y);
                hline_right(map, right->x, y);
            }
        }
    }

    return true;
}
