#include <stdlib.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

static bool traverse_node(TCOD_bsp_t *node, map_t *map);
static void vline(map_t *map, int x, int y1, int y2);
static void vline_up(map_t *map, int x, int y);
static void vline_down(map_t *map, int x, int y);
static void hline(map_t *map, int x1, int y, int x2);
static void hline_left(map_t *map, int x, int y);
static void hline_right(map_t *map, int x, int y);

map_t *map_create(void)
{
    map_t *map = (map_t *)malloc(sizeof(map_t));

    map->stair_down_x = -1;
    map->stair_down_y = -1;
    map->stair_up_x = -1;
    map->stair_up_y = -1;
    map->rooms = TCOD_list_new();
    map->lights = TCOD_list_new();
    map->actors = TCOD_list_new();

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile_initialize(tile, TILE_TYPE_WALL);
        }
    }

    TCOD_bsp_t *bsp = TCOD_bsp_new_with_size(0, 0, MAP_WIDTH, MAP_HEIGHT);
    TCOD_bsp_split_recursive(bsp, NULL, BSP_DEPTH, MIN_ROOM_SIZE + 1, MIN_ROOM_SIZE + 1, 1.5f, 1.5f);
    TCOD_bsp_traverse_inverted_level_order(bsp, traverse_node, map);
    TCOD_bsp_delete(bsp);

    room_t *stair_down_room = map_get_random_room(map);
    room_get_random_pos(stair_down_room, &map->stair_down_x, &map->stair_down_y);
    map->tiles[map->stair_down_x][map->stair_down_y].type = TILE_TYPE_STAIR_DOWN;

    room_t *stair_up_room = map_get_random_room(map);
    room_get_random_pos(stair_up_room, &map->stair_up_x, &map->stair_up_y);
    map->tiles[map->stair_up_x][map->stair_up_y].type = TILE_TYPE_STAIR_UP;

    for (int i = 0; i < NUM_LIGHTS; i++)
    {
        room_t *room = map_get_random_room(map);

        int x, y;
        room_get_random_pos(room, &x, &y);

        light_t *light = light_create(map, x, y, 10, TCOD_color_RGB(TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255)));

        TCOD_list_push(map->lights, light);
        map->tiles[x][y].light = light;
    }

    for (int i = 0; i < NUM_ACTORS; i++)
    {
        room_t *room = map_get_random_room(map);

        if (room == stair_up_room)
        {
            i--;

            continue;
        }

        int x, y;
        room_get_random_pos(room, &x, &y);

        if (map->tiles[x][y].actor != NULL)
        {
            i--;

            continue;
        }

        actor_t *actor = actor_create(map, x, y, '@', TCOD_red, 10);

        TCOD_list_push(map->actors, actor);
        map->tiles[x][y].actor = actor;

        if (TCOD_random_get_int(NULL, 0, 4) == 0)
        {
            item_t *item = item_create_random();

            TCOD_list_push(actor->items, item);
        }
    }

    for (int i = 0; i < NUM_ITEMS; i++)
    {
        room_t *room = map_get_random_room(map);

        int x, y;
        room_get_random_pos(room, &x, &y);

        item_t *item = item_create_random();

        TCOD_list_push(map->tiles[x][y].items, item);
    }

    TCOD_list_push(maps, map);

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

#if !FULL_ROOMS
        min_x = TCOD_random_get_int(NULL, min_x, max_x - MIN_ROOM_SIZE + 1);
        min_y = TCOD_random_get_int(NULL, min_y, max_y - MIN_ROOM_SIZE + 1);
        max_x = TCOD_random_get_int(NULL, min_x + MIN_ROOM_SIZE - 2, max_x);
        max_y = TCOD_random_get_int(NULL, min_y + MIN_ROOM_SIZE - 2, max_y);
#endif

        node->x = min_x;
        node->y = min_y;
        node->w = max_x - min_x + 1;
        node->h = max_y - min_y + 1;

        for (int x = min_x; x < max_x + 1; x++)
        {
            for (int y = min_y; y < max_y + 1; y++)
            {
                tile_t *tile = &map->tiles[x][y];

                tile->type = TILE_TYPE_FLOOR;
            }
        }

        room_t *room = room_create(node->x, node->y, node->w, node->h);

        TCOD_list_push(map->rooms, room);
    }
    else
    {
        TCOD_bsp_t *left = TCOD_bsp_left(node);
        TCOD_bsp_t *right = TCOD_bsp_right(node);

        node->x = min(left->x, right->x);
        node->y = min(left->y, right->y);
        node->w = max(left->x + left->w, right->x + right->w) - node->x;
        node->h = max(left->y + left->h, right->y + right->h) - node->y;

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
                int min_x = max(left->x, right->x);
                int max_x = min(left->x + left->w - 1, right->x + right->w - 1);
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
                int min_y = max(left->y, right->y);
                int max_y = min(left->y + left->h - 1, right->y + right->h - 1);
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

        tile->type = TILE_TYPE_FLOOR;
    }
}

static void vline_up(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y >= 0 && tile->type != TILE_TYPE_FLOOR)
    {
        tile->type = TILE_TYPE_FLOOR;

        y--;
    }
}

static void vline_down(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y < MAP_HEIGHT && tile->type != TILE_TYPE_FLOOR)
    {
        tile->type = TILE_TYPE_FLOOR;

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

        tile->type = TILE_TYPE_FLOOR;
    }
}

static void hline_left(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (x >= 0 && tile->type != TILE_TYPE_FLOOR)
    {
        tile->type = TILE_TYPE_FLOOR;

        x--;
    }
}

static void hline_right(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (x < MAP_WIDTH && tile->type != TILE_TYPE_FLOOR)
    {
        tile->type = TILE_TYPE_FLOOR;

        x++;
    }
}

void map_turn(map_t *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile_turn(tile);
        }
    }

    for (void **i = TCOD_list_begin(map->lights); i != TCOD_list_end(map->lights); i++)
    {
        light_t *light = *i;

        light_turn(light);
    }

    for (void **i = TCOD_list_begin(map->actors); i != TCOD_list_end(map->actors); i++)
    {
        actor_t *actor = *i;

        actor_turn(actor);
    }

    for (void **i = TCOD_list_begin(map->actors); i != TCOD_list_end(map->actors); i++)
    {
        actor_t *actor = *i;
        tile_t *tile = &actor->map->tiles[actor->x][actor->y];

        if (!actor->mark_for_delete)
        {
            continue;
        }

        corpse_t *corpse = corpse_create(actor);

        TCOD_list_push(tile->items, corpse);

        for (void **i = TCOD_list_begin(actor->items); i != TCOD_list_end(actor->items); i++)
        {
            item_t *item = *i;

            i = TCOD_list_remove_iterator(actor->items, i);
            TCOD_list_push(tile->items, item);
        }

        tile->actor = NULL;
        i = TCOD_list_remove_iterator(actor->map->actors, i);

        actor_destroy(actor);
    }
}

void map_tick(map_t *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile_tick(tile);
        }
    }

    for (void **i = TCOD_list_begin(map->lights); i != TCOD_list_end(map->lights); i++)
    {
        light_t *light = *i;

        light_tick(light);
    }

    for (void **i = TCOD_list_begin(map->actors); i != TCOD_list_end(map->actors); i++)
    {
        actor_t *actor = *i;

        actor_tick(actor);
    }
}

room_t *map_get_random_room(map_t *map)
{
    return TCOD_list_get(map->rooms, TCOD_random_get_int(NULL, 0, TCOD_list_size(map->rooms) - 1));
}

TCOD_map_t map_to_TCOD_map(map_t *map)
{
    TCOD_map_t TCOD_map = TCOD_map_new(MAP_WIDTH, MAP_HEIGHT);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            TCOD_map_set_properties(TCOD_map, x, y, tile_transparent[tile->type], tile->actor == NULL ? tile_walkable[tile->type] : false);
        }
    }

    return TCOD_map;
}

void map_draw_turn(map_t *map)
{
    for (int x = view_x; x < view_x + view_width; x++)
    {
        for (int y = view_y; y < view_y + view_height; y++)
        {
            if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
            {
                continue;
            }

            tile_t *tile = &map->tiles[x][y];

            tile_draw_turn(tile, x, y);
        }
    }

    for (void **i = TCOD_list_begin(map->lights); i != TCOD_list_end(map->lights); i++)
    {
        light_t *light = *i;

        light_draw_turn(light);
    }

    for (void **i = TCOD_list_begin(map->actors); i != TCOD_list_end(map->actors); i++)
    {
        actor_t *actor = *i;

        actor_draw_turn(actor);
    }
}

void map_draw_tick(map_t *map)
{
    if (sfx)
    {
        static TCOD_noise_t noise = NULL;
        if (noise == NULL)
        {
            noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
        }

        static float noise_x = 0.0f;

        noise_x += 0.2f;
        float noise_dx = noise_x + 20.0f;
        float dx = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        noise_dx += 30.0f;
        float dy = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        float di = 0.2f * TCOD_noise_get(noise, &noise_x);

        for (int x = view_x; x < view_x + view_width; x++)
        {
            for (int y = view_y; y < view_y + view_height; y++)
            {
                if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
                {
                    continue;
                }

                tile_t *tile = &map->tiles[x][y];

                tile_draw_tick(tile, x, y, dx, dy, di);
            }
        }
    }

    for (void **i = TCOD_list_begin(map->lights); i != TCOD_list_end(map->lights); i++)
    {
        light_t *light = *i;

        light_draw_tick(light);
    }

    for (void **i = TCOD_list_begin(map->actors); i != TCOD_list_end(map->actors); i++)
    {
        actor_t *actor = *i;

        actor_draw_tick(actor);
    }
}

void map_destroy(map_t *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile_finalize(tile);
        }
    }

    for (void **i = TCOD_list_begin(map->rooms); i != TCOD_list_end(map->rooms); i++)
    {
        room_t *room = *i;

        room_destroy(room);
    }

    TCOD_list_delete(map->rooms);

    for (void **i = TCOD_list_begin(map->lights); i != TCOD_list_end(map->lights); i++)
    {
        light_t *light = *i;

        light_destroy(light);
    }

    TCOD_list_delete(map->lights);

    for (void **i = TCOD_list_begin(map->actors); i != TCOD_list_end(map->actors); i++)
    {
        actor_t *actor = *i;

        actor_destroy(actor);
    }

    TCOD_list_delete(map->actors);

    free(map);
}