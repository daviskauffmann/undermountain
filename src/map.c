#include <libtcod/libtcod.h>

#include "map.h"
#include "tile.h"
#include "room.h"
#include "entity.h"
#include "assemblage.h"
#include "game.h"

void map_init(map_t *map, game_t *game, int level)
{
    map->game = game;
    map->level = level;
    map->rooms = TCOD_list_new();
    map->entities = TCOD_list_new();

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile_init(tile, TILE_EMPTY, false);
        }
    }
}

#define CUSTOM_NUM_ROOM_ATTEMPTS 20
#define CUSTOM_MIN_ROOM_SIZE 5
#define CUSTOM_MAX_ROOM_SIZE 15
#define CUSTOM_PREVENT_OVERLAP 0
#define CUSTOM_DOOR_CHANCE 0.5f

void map_generate_custom(map_t *map)
{
    for (int i = 0; i < CUSTOM_NUM_ROOM_ATTEMPTS; i++)
    {
        room_t *room = room_create(
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
                if (map->tiles[x][y].type == TILE_FLOOR)
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
                map->tiles[x][y].type = TILE_FLOOR;
            }
        }

        TCOD_list_push(map->rooms, room);
    }

    for (int i = 0; i < TCOD_list_size(map->rooms) - 1; i++)
    {
        room_t *room = TCOD_list_get(map->rooms, i);
        room_t *next_room = TCOD_list_get(map->rooms, i + 1);

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
                    map->tiles[x][y].type = TILE_FLOOR;
                }
            }
        }
    }

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            if (map->tiles[x][y].type == TILE_FLOOR)
            {
                if (map->tiles[x][y - 1].type == TILE_EMPTY)
                {
                    map->tiles[x][y - 1].type = TILE_WALL;
                }

                if (map->tiles[x + 1][y - 1].type == TILE_EMPTY)
                {
                    map->tiles[x + 1][y - 1].type = TILE_WALL;
                }

                if (map->tiles[x + 1][y].type == TILE_EMPTY)
                {
                    map->tiles[x + 1][y].type = TILE_WALL;
                }

                if (map->tiles[x + 1][y + 1].type == TILE_EMPTY)
                {
                    map->tiles[x + 1][y + 1].type = TILE_WALL;
                }

                if (map->tiles[x][y + 1].type == TILE_EMPTY)
                {
                    map->tiles[x][y + 1].type = TILE_WALL;
                }

                if (map->tiles[x - 1][y - 1].type == TILE_EMPTY)
                {
                    map->tiles[x - 1][y - 1].type = TILE_WALL;
                }

                if (map->tiles[x - 1][y].type == TILE_EMPTY)
                {
                    map->tiles[x - 1][y].type = TILE_WALL;
                }

                if (map->tiles[x - 1][y + 1].type == TILE_EMPTY)
                {
                    map->tiles[x - 1][y + 1].type = TILE_WALL;
                }
            }
        }
    }

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            if (map->tiles[x][y].type == TILE_FLOOR && TCOD_random_get_float(NULL, 0, 1) < CUSTOM_DOOR_CHANCE)
            {

                if (map->tiles[x][y - 1].type == TILE_FLOOR && map->tiles[x + 1][y - 1].type == TILE_FLOOR && map->tiles[x - 1][y - 1].type == TILE_FLOOR)
                {
                    if (map->tiles[x - 1][y].type == TILE_WALL && map->tiles[x + 1][y].type == TILE_WALL)
                    {
                        map->tiles[x][y].type = TILE_DOOR_CLOSED;
                    }
                }
                if (map->tiles[x + 1][y].type == TILE_FLOOR && map->tiles[x + 1][y - 1].type == TILE_FLOOR && map->tiles[x + 1][y + 1].type == TILE_FLOOR)
                {
                    if (map->tiles[x][y + 1].type == TILE_WALL && map->tiles[x][y - 1].type == TILE_WALL)
                    {
                        map->tiles[x][y].type = TILE_DOOR_CLOSED;
                    }
                }
                if (map->tiles[x][y + 1].type == TILE_FLOOR && map->tiles[x + 1][y + 1].type == TILE_FLOOR && map->tiles[x - 1][y + 1].type == TILE_FLOOR)
                {
                    if (map->tiles[x - 1][y].type == TILE_WALL && map->tiles[x + 1][y].type == TILE_WALL)
                    {
                        map->tiles[x][y].type = TILE_DOOR_CLOSED;
                    }
                }
                if (map->tiles[x - 1][y].type == TILE_FLOOR && map->tiles[x - 1][y - 1].type == TILE_FLOOR && map->tiles[x - 1][y + 1].type == TILE_FLOOR)
                {
                    if (map->tiles[x][y + 1].type == TILE_WALL && map->tiles[x][y - 1].type == TILE_WALL)
                    {
                        map->tiles[x][y].type = TILE_DOOR_CLOSED;
                    }
                }
            }
        }
    }

    room_t *stair_down_room = map_get_random_room(map);
    room_get_random_pos(stair_down_room, &map->stair_down_x, &map->stair_down_y);
    map->tiles[map->stair_down_x][map->stair_down_y].type = TILE_STAIR_DOWN;

    room_t *stair_up_room = map_get_random_room(map);
    room_get_random_pos(stair_up_room, &map->stair_up_x, &map->stair_up_y);
    map->tiles[map->stair_up_x][map->stair_up_y].type = TILE_STAIR_UP;
}

#define BSP_MIN_ROOM_SIZE 4
#define BSP_DEPTH 8
#define BSP_RANDOM_ROOMS 0
#define BSP_ROOM_WALLS 1

void map_generate_bsp(map_t *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

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

    room_t *stair_down_room = map_get_random_room(map);
    room_get_random_pos(stair_down_room, &map->stair_down_x, &map->stair_down_y);
    map->tiles[map->stair_down_x][map->stair_down_y].type = TILE_STAIR_DOWN;

    room_t *stair_up_room = map_get_random_room(map);
    room_get_random_pos(stair_up_room, &map->stair_up_x, &map->stair_up_y);
    map->tiles[map->stair_up_x][map->stair_up_y].type = TILE_STAIR_UP;
}

void hline(map_t *map, int x1, int y, int x2)
{
    int x = x1;
    int dx = (x1 > x2 ? -1 : 1);

    map->tiles[x][y].type = TILE_FLOOR;

    if (x1 != x2)
    {
        do
        {
            x += dx;

            map->tiles[x][y].type = TILE_FLOOR;
        } while (x != x2);
    }
}

void hline_left(map_t *map, int x, int y)
{
    while (x >= 0 && map->tiles[x][y].type != TILE_FLOOR)
    {
        map->tiles[x][y].type = TILE_FLOOR;

        x--;
    }
}

void hline_right(map_t *map, int x, int y)
{
    while (x < MAP_WIDTH && map->tiles[x][y].type != TILE_FLOOR)
    {
        map->tiles[x][y].type = TILE_FLOOR;

        x++;
    }
}

void vline(map_t *map, int x, int y1, int y2)
{
    int y = y1;
    int dy = (y1 > y2 ? -1 : 1);

    map->tiles[x][y].type = TILE_FLOOR;

    if (y1 != y2)
    {
        do
        {
            y += dy;

            map->tiles[x][y].type = TILE_FLOOR;
        } while (y != y2);
    }
}

void vline_up(map_t *map, int x, int y)
{
    while (y >= 0 && map->tiles[x][y].type != TILE_FLOOR)
    {
        map->tiles[x][y].type = TILE_FLOOR;

        y--;
    }
}

void vline_down(map_t *map, int x, int y)
{
    while (y < MAP_HEIGHT && map->tiles[x][y].type != TILE_FLOOR)
    {
        map->tiles[x][y].type = TILE_FLOOR;

        y++;
    }
}

bool traverse_node(TCOD_bsp_t *node, map_t *map)
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
                map->tiles[x][y].type = TILE_FLOOR;
            }
        }

        room_t *room = room_create(node->x, node->y, node->w, node->h);

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

#define NUM_MONSTERS 20
#define NUM_ADVENTURERS 5
#define NUM_ITEMS 5
#define NUM_BRAZIERS 5

void map_populate(map_t *map)
{
    for (int i = 0; i < NUM_MONSTERS; i++)
    {
        room_t *room = map_get_random_room(map);

        int x, y;
        room_get_random_pos(room, &x, &y);

        entity_t *monster = NULL;
        switch (TCOD_random_get_int(NULL, 0, 3))
        {
        case 0:
        {
            monster = create_skeleton(map, x, y);

            break;
        }
        case 1:
        {
            monster = create_skeleton_captain(map, x, y);

            break;
        }
        case 2:
        {
            monster = create_zombie(map, x, y);

            break;
        }
        case 3:
        {
            monster = create_jackal(map, x, y);

            break;
        }
        }

        if (TCOD_random_get_int(NULL, 0, 100) == 0)
        {
            light_t *light = (light_t *)component_add(monster, COMPONENT_LIGHT);
            light->radius = 10;
            light->color = TCOD_light_amber;
            light->flicker = true;
            light->priority = LIGHT_PRIORITY_2;
            light->fov_map = NULL;
        }
    }

    for (int i = 0; i < NUM_ADVENTURERS; i++)
    {
        room_t *room = map_get_random_room(map);

        int x, y;
        room_get_random_pos(room, &x, &y);

        entity_t *adventurer = create_adventurer(map, x, y);

        if (TCOD_random_get_int(NULL, 0, 100) == 0)
        {
            light_t *light = (light_t *)component_add(adventurer, COMPONENT_LIGHT);
            light->radius = 10;
            light->color = TCOD_light_amber;
            light->flicker = true;
            light->priority = LIGHT_PRIORITY_2;
            light->fov_map = NULL;
        }
    }

    for (int i = 0; i < NUM_ITEMS; i++)
    {
        room_t *room = map_get_random_room(map);

        int x, y;
        room_get_random_pos(room, &x, &y);

        entity_t *longsword = create_longsword(map, x, y);
    }

    for (int i = 0; i < NUM_BRAZIERS; i++)
    {
        room_t *room = map_get_random_room(map);

        int x, y;
        room_get_random_pos(room, &x, &y);

        entity_t *brazier = create_brazier(map, x, y);
    }
}

bool map_is_inside(int x, int y)
{
    return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
}

room_t *
map_get_random_room(map_t *map)
{
    return TCOD_list_get(map->rooms, TCOD_random_get_int(NULL, 0, TCOD_list_size(map->rooms) - 1));
}

bool map_is_transparent(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    bool is_transparent = map->game->tile_info[tile->type].is_transparent;

    for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
    {
        entity_t *entity = *iterator;

        component_t *opaque = component_get(entity, COMPONENT_OPAQUE);

        if (opaque)
        {
            is_transparent = false;

            break;
        }
    }

    return is_transparent;
}

bool map_is_walkable(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    bool is_walkable = map->game->tile_info[tile->type].is_walkable;

    for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
    {
        entity_t *entity = *iterator;

        component_t *solid = component_get(entity, COMPONENT_SOLID);

        if (solid)
        {
            is_walkable = false;

            break;
        }
    }

    return is_walkable;
}

TCOD_map_t
map_to_TCOD_map(map_t *map)
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

TCOD_map_t
map_to_fov_map(map_t *map, int x, int y, int radius)
{
    TCOD_map_t fov_map = map_to_TCOD_map(map);

    TCOD_map_compute_fov(fov_map, x, y, radius, true, FOV_BASIC);

    return fov_map;
}

void map_reset(map_t *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile_reset(tile);
        }
    }

    for (void **iterator = TCOD_list_begin(map->rooms); iterator != TCOD_list_end(map->rooms); iterator++)
    {
        room_t *room = *iterator;

        room_destroy(room);
    }

    TCOD_list_delete(map->rooms);

    TCOD_list_delete(map->entities);
}
