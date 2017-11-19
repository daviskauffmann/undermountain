#include <libtcod.h>

#include "world.h"
#include "CMemLeak.h"
#include "ECS.h"

/* World */
void world_init(void)
{
    maps = TCOD_list_new();

    tile_common = (tile_common_t){
        .shadow_color = TCOD_color_RGB(16, 16, 32)};

    tile_info[TILE_FLOOR] = (tile_info_t){
        .glyph = '.',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};
    tile_info[TILE_WALL] = (tile_info_t){
        .glyph = '#',
        .color = TCOD_white,
        .is_transparent = false,
        .is_walkable = false};
    tile_info[TILE_STAIR_DOWN] = (tile_info_t){
        .glyph = '>',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};
    tile_info[TILE_STAIR_UP] = (tile_info_t){
        .glyph = '<',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};
}

void world_reset(void)
{
    for (void **iterator = TCOD_list_begin(maps); iterator != TCOD_list_end(maps); iterator++)
    {
        map_t *map = *iterator;

        map_destroy(map);
    }

    TCOD_list_delete(maps);
}

/* Tiles */
void tile_init(tile_t *tile, tile_type_t type, bool seen)
{
    tile->type = type;
    tile->seen = seen;
    tile->entities = TCOD_list_new();
}

void tile_reset(tile_t *tile)
{
    TCOD_list_delete(tile->entities);
}

/* Rooms */
room_t *room_create(int x, int y, int w, int h)
{
    room_t *room = (room_t *)malloc(sizeof(room_t));

    room->x = x;
    room->y = y;
    room->w = w;
    room->h = h;

    return room;
}

void room_get_random_pos(room_t *room, int *x, int *y)
{
    *x = TCOD_random_get_int(NULL, room->x, room->x + room->w - 1);
    *y = TCOD_random_get_int(NULL, room->y, room->y + room->h - 1);
}

void room_destroy(room_t *room)
{
    free(room);
}

/* Maps */
#define BSP_DEPTH 10
#define MIN_ROOM_SIZE 5
#define FULL_ROOMS 1
#define NUM_MONSTERS 50

static bool traverse_node(TCOD_bsp_t *node, map_t *map);
static void vline(map_t *map, int x, int y1, int y2);
static void vline_up(map_t *map, int x, int y);
static void vline_down(map_t *map, int x, int y);
static void hline(map_t *map, int x1, int y, int x2);
static void hline_left(map_t *map, int x, int y);
static void hline_right(map_t *map, int x, int y);

map_t *map_create(int level)
{
    map_t *map = (map_t *)malloc(sizeof(map_t));

    map->level = level;
    map->rooms = TCOD_list_new();

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile_init(tile, TILE_WALL, false);
        }
    }

    TCOD_bsp_t *bsp = TCOD_bsp_new_with_size(0, 0, MAP_WIDTH, MAP_HEIGHT);
    TCOD_bsp_split_recursive(bsp, NULL, BSP_DEPTH, MIN_ROOM_SIZE + 1, MIN_ROOM_SIZE + 1, 1.5f, 1.5f);
    TCOD_bsp_traverse_inverted_level_order(bsp, traverse_node, map);
    TCOD_bsp_delete(bsp);

    room_t *stair_down_room = map_get_random_room(map);
    room_get_random_pos(stair_down_room, &map->stair_down_x, &map->stair_down_y);
    map->tiles[map->stair_down_x][map->stair_down_y].type = TILE_STAIR_DOWN;

    room_t *stair_up_room = map_get_random_room(map);
    room_get_random_pos(stair_up_room, &map->stair_up_x, &map->stair_up_y);
    map->tiles[map->stair_up_x][map->stair_up_y].type = TILE_STAIR_UP;

    for (int i = 0; i < NUM_MONSTERS; i++)
    {
        room_t *room = map_get_random_room(map);

        int x, y;
        room_get_random_pos(room, &x, &y);

        if (room == stair_up_room)
        {
            i--;

            continue;
        }

        // TODO: better npc generation
        entity_t *entity = entity_create();
        position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
        position->map = map;
        position->x = x;
        position->y = y;
        TCOD_list_push(map->tiles[position->x][position->y].entities, entity);
        physics_t *physics = (physics_t *)component_add(entity, COMPONENT_PHYSICS);
        physics->is_walkable = false;
        physics->is_transparent = true;
        appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
        appearance->layer = LAYER_1;
        fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
        fov->radius = 5;
        if (fov->fov_map != NULL)
        {
            TCOD_map_delete(fov->fov_map);
        }
        fov->fov_map = NULL;
        ai_t *ai = (ai_t *)component_add(entity, COMPONENT_AI);
        ai->type = AI_MONSTER;
        ai->energy = 1.0f;
        health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
        health->max = TCOD_random_get_int(NULL, 10, 20);
        health->current = health->max;
        alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
        alignment->type = ALIGNMENT_EVIL;

        switch (TCOD_random_get_int(NULL, 0, 3))
        {
        case 0:
        {
            appearance->name = "Skeleton";
            appearance->glyph = 's';
            appearance->color = TCOD_white;
            ai->energy_per_turn = 0.5f;

            break;
        }
        case 1:
        {
            appearance->name = "Skeleton Captain";
            appearance->glyph = 'S';
            appearance->color = TCOD_white;
            ai->energy_per_turn = 0.75f;

            break;
        }
        case 2:
        {
            appearance->name = "Zombie";
            appearance->glyph = 'z';
            appearance->color = TCOD_dark_green;
            ai->energy_per_turn = 0.25f;

            break;
        }
        case 3:
        {
            appearance->name = "Jackal";
            appearance->glyph = 'j';
            appearance->color = TCOD_dark_orange;
            ai->energy_per_turn = 1.5f;

            break;
        }
        }

        if (TCOD_random_get_int(NULL, 0, 100) == 0)
        {
            light_t *light = (light_t *)component_add(entity, COMPONENT_LIGHT);
            light->radius = 10;
            light->color = TCOD_light_amber;
            light->flicker = true;
            light->priority = LIGHT_PRIORITY_1;
            if (light->fov_map != NULL)
            {
                TCOD_map_delete(light->fov_map);
            }
            light->fov_map = NULL;
        }
    }

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

                tile->type = TILE_FLOOR;
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
                int min_x = MAX(left->x, right->x);
                int max_x = MIN(left->x + left->w - 1, right->x + right->w - 1);
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
                int min_y = MAX(left->y, right->y);
                int max_y = MIN(left->y + left->h - 1, right->y + right->h - 1);
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

        tile->type = TILE_FLOOR;
    }
}

static void vline_up(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y >= 0 && tile->type != TILE_FLOOR)
    {
        tile->type = TILE_FLOOR;

        y--;
    }
}

static void vline_down(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y < MAP_HEIGHT && tile->type != TILE_FLOOR)
    {
        tile->type = TILE_FLOOR;

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

        tile->type = TILE_FLOOR;
    }
}

static void hline_left(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (x >= 0 && tile->type != TILE_FLOOR)
    {
        tile->type = TILE_FLOOR;

        x--;
    }
}

static void hline_right(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (x < MAP_WIDTH && tile->type != TILE_FLOOR)
    {
        tile->type = TILE_FLOOR;

        x++;
    }
}

bool map_is_inside(int x, int y)
{
    return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
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

            bool is_walkable = tile_info[tile->type].is_walkable;

            for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
            {
                entity_t *entity = *iterator;

                if (entity->id != ID_UNUSED)
                {
                    physics_t *physics = (physics_t *)component_get(entity, COMPONENT_PHYSICS);

                    if (physics != NULL)
                    {
                        if (!physics->is_walkable)
                        {
                            is_walkable = false;

                            break;
                        }
                    }
                }
            }

            TCOD_map_set_properties(TCOD_map, x, y, tile_info[tile->type].is_transparent, is_walkable);
        }
    }

    return TCOD_map;
}

TCOD_map_t map_to_fov_map(map_t *map, int x, int y, int radius)
{
    TCOD_map_t fov_map = map_to_TCOD_map(map);

    TCOD_map_compute_fov(fov_map, x, y, radius, true, FOV_BASIC);

    return fov_map;
}

void map_destroy(map_t *map)
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

    free(map);
}