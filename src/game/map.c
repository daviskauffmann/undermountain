#include "map.h"

#include "actor.h"
#include "assets.h"
#include "explosion.h"
#include "object.h"
#include "projectile.h"
#include "room.h"
#include "util.h"
#include "world.h"

#define DOOR_CHANCE 0.5f
#define SPAWN_OBJECTS 2
#define SPAWN_ADVENTURERS 1
#define SPAWN_MONSTERS 10
#define SPAWN_ITEMS 5

void map_init(struct map *const map, const uint8_t floor)
{
    map->floor = floor;
    map->stair_down_x = 0;
    map->stair_down_y = 0;
    map->stair_up_x = 0;
    map->stair_up_y = 0;
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            struct tile *tile = &map->tiles[x][y];
            tile_init(tile, TILE_TYPE_EMPTY, false);
        }
    }
    map->rooms = TCOD_list_new();
    map->objects = TCOD_list_new();
    map->actors = TCOD_list_new();
    map->corpses = TCOD_list_new();
    map->items = TCOD_list_new();
    map->projectiles = TCOD_list_new();
    map->explosions = TCOD_list_new();
    map->current_actor_index = 0;
}

void map_uninit(struct map *const map)
{
    TCOD_LIST_FOREACH(map->explosions)
    {
        explosion_delete(*iterator);
    }
    TCOD_list_delete(map->explosions);

    TCOD_LIST_FOREACH(map->projectiles)
    {
        projectile_delete(*iterator);
    }
    TCOD_list_delete(map->projectiles);

    TCOD_LIST_FOREACH(map->items)
    {
        item_delete(*iterator);
    }
    TCOD_list_delete(map->items);

    TCOD_LIST_FOREACH(map->corpses)
    {
        corpse_delete(*iterator);
    }
    TCOD_list_delete(map->corpses);

    TCOD_LIST_FOREACH(map->actors)
    {
        actor_delete(*iterator);
    }
    TCOD_list_delete(map->actors);

    TCOD_LIST_FOREACH(map->objects)
    {
        object_delete(*iterator);
    }
    TCOD_list_delete(map->objects);

    TCOD_LIST_FOREACH(map->rooms)
    {
        room_delete(*iterator);
    }
    TCOD_list_delete(map->rooms);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_uninit(&map->tiles[x][y]);
        }
    }
}

static void hline(
    struct map *const map,
    const int x1,
    const int y,
    const int x2)
{
    int x = x1;

    map->tiles[x][y].type = TILE_TYPE_FLOOR;

    if (x1 != x2)
    {
        const int dx = (x1 > x2 ? -1 : 1);

        do
        {
            x += dx;
            map->tiles[x][y].type = TILE_TYPE_FLOOR;
        } while (x != x2);
    }
}

static void vline(
    struct map *const map,
    const int x,
    const int y1,
    const int y2)
{
    int y = y1;

    map->tiles[x][y].type = TILE_TYPE_FLOOR;

    if (y1 != y2)
    {
        const int dy = (y1 > y2 ? -1 : 1);

        do
        {
            y += dy;
            map->tiles[x][y].type = TILE_TYPE_FLOOR;
        } while (y != y2);
    }
}

static void hline_left(
    struct map *const map,
    int x,
    const int y)
{
    while (x >= 0 && map->tiles[x][y].type != TILE_TYPE_FLOOR)
    {
        map->tiles[x--][y].type = TILE_TYPE_FLOOR;
    }
}

static void hline_right(
    struct map *const map,
    int x,
    const int y)
{
    while (x < MAP_WIDTH && map->tiles[x][y].type != TILE_TYPE_FLOOR)
    {
        map->tiles[x++][y].type = TILE_TYPE_FLOOR;
    }
}

static void vline_up(
    struct map *const map,
    const int x,
    int y)
{
    while (y >= 0 && map->tiles[x][y].type != TILE_TYPE_FLOOR)
    {
        map->tiles[x][y--].type = TILE_TYPE_FLOOR;
    }
}

static void vline_down(
    struct map *const map,
    const int x,
    int y)
{
    while (y < MAP_HEIGHT && map->tiles[x][y].type != TILE_TYPE_FLOOR)
    {
        map->tiles[x][y++].type = TILE_TYPE_FLOOR;
    }
}

struct traverse_node_data
{
    struct map *map;
    int min_room_size;
    bool room_walls;
    bool random_rooms;
};

static bool traverse_node(TCOD_bsp_t *const node, void *const data)
{
    struct traverse_node_data *const traverse_node_data = data;
    struct map *const map = traverse_node_data->map;
    const int min_room_size = traverse_node_data->min_room_size;
    const bool room_walls = traverse_node_data->room_walls;
    const bool random_rooms = traverse_node_data->random_rooms;

    if (TCOD_bsp_is_leaf(node))
    {
        int minx = node->x + 1;
        int maxx = node->x + node->w - 1;
        int miny = node->y + 1;
        int maxy = node->y + node->h - 1;

        if (!room_walls)
        {
            if (minx > 1)
            {
                minx--;
            }
            if (miny > 1)
            {
                miny--;
            }
        }

        if (maxx == MAP_WIDTH - 1)
        {
            maxx--;
        }
        if (maxy == MAP_HEIGHT - 1)
        {
            maxy--;
        }

        if (random_rooms)
        {
            minx = TCOD_random_get_int(world->random, minx, maxx - min_room_size + 1);
            miny = TCOD_random_get_int(world->random, miny, maxy - min_room_size + 1);
            maxx = TCOD_random_get_int(world->random, minx + min_room_size - 1, maxx);
            maxy = TCOD_random_get_int(world->random, miny + min_room_size - 1, maxy);
        }

        node->x = minx;
        node->y = miny;
        node->w = maxx - minx + 1;
        node->h = maxy - miny + 1;
        for (int x = minx; x <= maxx; x++)
        {
            for (int y = miny; y <= maxy; y++)
            {
                map->tiles[x][y].type = TILE_TYPE_FLOOR;
            }
        }

        struct room *const room = room_new(
            node->x,
            node->y,
            node->w,
            node->h);
        TCOD_list_push(map->rooms, room);
    }
    else
    {
        TCOD_bsp_t *const left = TCOD_bsp_left(node);
        TCOD_bsp_t *const right = TCOD_bsp_right(node);

        node->x = MIN(left->x, right->x);
        node->y = MIN(left->y, right->y);
        node->w = MAX(left->x + left->w, right->x + right->w) - node->x;
        node->h = MAX(left->y + left->h, right->y + right->h) - node->y;

        if (node->horizontal)
        {
            if (left->x + left->w - 1 < right->x || right->x + right->w - 1 < left->x)
            {
                const int x1 = TCOD_random_get_int(world->random, left->x, left->x + left->w - 1);
                const int x2 = TCOD_random_get_int(world->random, right->x, right->x + right->w - 1);
                const int y = TCOD_random_get_int(world->random, left->y + left->h, right->y);

                vline_up(map, x1, y - 1);
                hline(map, x1, y, x2);
                vline_down(map, x2, y + 1);
            }
            else
            {
                const int minx = MAX(left->x, right->x);
                const int maxx = MIN(left->x + left->w - 1, right->x + right->w - 1);
                const int x = TCOD_random_get_int(world->random, minx, maxx);

                vline_down(map, x, right->y);
                vline_up(map, x, right->y - 1);
            }
        }
        else
        {
            if (left->y + left->h - 1 < right->y || right->y + right->h - 1 < left->y)
            {
                const int y1 = TCOD_random_get_int(world->random, left->y, left->y + left->h - 1);
                const int y2 = TCOD_random_get_int(world->random, right->y, right->y + right->h - 1);
                const int x = TCOD_random_get_int(world->random, left->x + left->w, right->x);

                hline_left(map, x - 1, y1);
                vline(map, x, y1, y2);
                hline_right(map, x + 1, y2);
            }
            else
            {
                const int miny = MAX(left->y, right->y);
                const int maxy = MIN(left->y + left->h - 1, right->y + right->h - 1);
                const int y = TCOD_random_get_int(world->random, miny, maxy);

                hline_left(map, right->x - 1, y);
                hline_right(map, right->x, y);
            }
        }
    }

    return true;
}

void map_generate(struct map *const map, const enum map_type map_type)
{
    // setup default tile state
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            struct tile *const tile = &map->tiles[x][y];

            if (x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1)
            {
                tile->type = TILE_TYPE_WALL;
            }
            else
            {
                switch (map_type)
                {
                case MAP_TYPE_LARGE_DUNGEON:
                {
                    tile->type = TILE_TYPE_EMPTY;
                }
                break;
                case MAP_TYPE_SMALL_DUNGEON:
                {
                    tile->type = TILE_TYPE_WALL;
                }
                break;
                case MAP_TYPE_CAVES:
                {
                    tile->type = TILE_TYPE_FLOOR;
                }
                break;
                case MAP_TYPE_GRASSY_CAVES:
                {
                    tile->type = TILE_TYPE_GRASS;
                }
                break;
                case MAP_TYPE_RUINS:
                {
                    tile->type = TCOD_random_get_int(world->random, 0, 4) == 0 ? TILE_TYPE_WALL : TILE_TYPE_FLOOR;
                }
                break;
                case NUM_MAP_TYPES:
                    break;
                }
            }
        }
    }

    // create rooms
    switch (map_type)
    {
    case MAP_TYPE_LARGE_DUNGEON:
    {
        const int num_room_attempts = 20;
        const int min_room_size = 5;
        const int max_room_size = 15;
        const int room_buffer = 3;
        const float prevent_overlap_chance = 0.5f;

        for (int i = 0; i < num_room_attempts; i++)
        {
            const int room_x = TCOD_random_get_int(world->random, 0, MAP_WIDTH);
            const int room_y = TCOD_random_get_int(world->random, 0, MAP_HEIGHT);
            const int room_w = TCOD_random_get_int(world->random, min_room_size, max_room_size);
            const int room_h = TCOD_random_get_int(world->random, min_room_size, max_room_size);
            if (room_x < room_buffer ||
                room_x + room_w > MAP_WIDTH - room_buffer ||
                room_y < room_buffer ||
                room_y + room_h > MAP_HEIGHT - room_buffer)
            {
                continue;
            }

            if (TCOD_random_get_float(world->random, 0, 1) < prevent_overlap_chance)
            {
                bool overlap = false;

                for (int x = room_x - room_buffer; x < room_x + room_w + room_buffer; x++)
                {
                    for (int y = room_y - room_buffer; y < room_y + room_h + room_buffer; y++)
                    {
                        if (map->tiles[x][y].type == TILE_TYPE_FLOOR)
                        {
                            overlap = true;
                        }
                    }
                }

                if (overlap)
                {
                    continue;
                }
            }

            for (int x = room_x; x < room_x + room_w; x++)
            {
                for (int y = room_y; y < room_y + room_h; y++)
                {
                    map->tiles[x][y].type = TILE_TYPE_FLOOR;
                }
            }

            struct room *const room = room_new(
                room_x,
                room_y,
                room_w,
                room_h);
            TCOD_list_push(map->rooms, room);
        }

        for (int i = 0; i < TCOD_list_size(map->rooms) - 1; i++)
        {
            int x1, y1;
            room_get_random_pos(
                TCOD_list_get(map->rooms, i),
                &x1, &y1);

            int x2, y2;
            room_get_random_pos(
                TCOD_list_get(map->rooms, i + 1),
                &x2, &y2);

            if (TCOD_random_get_int(world->random, 0, 1) == 0)
            {
                vline(map, x1, y1, y2);
                hline(map, x1, y2, x2);
            }
            else
            {
                hline(map, x1, y1, x2);
                vline(map, x2, y1, y2);
            }
        }

        for (int x = 1; x < MAP_WIDTH - 1; x++)
        {
            for (int y = 1; y < MAP_HEIGHT - 1; y++)
            {
                struct tile *tile = &map->tiles[x][y];
                if (tile->type == TILE_TYPE_FLOOR)
                {
                    struct tile *neighbors[8];
                    neighbors[0] = &map->tiles[x + 0][y - 1];
                    neighbors[1] = &map->tiles[x + 1][y - 1];
                    neighbors[2] = &map->tiles[x + 1][y + 0];
                    neighbors[3] = &map->tiles[x + 1][y + 1];
                    neighbors[4] = &map->tiles[x + 0][y + 1];
                    neighbors[5] = &map->tiles[x - 1][y + 1];
                    neighbors[6] = &map->tiles[x - 1][y + 0];
                    neighbors[7] = &map->tiles[x - 1][y - 1];
                    for (int i = 0; i < 8; i++)
                    {
                        struct tile *neighbor = neighbors[i];
                        if (neighbor->type == TILE_TYPE_EMPTY)
                        {
                            neighbor->type = TILE_TYPE_WALL;
                        }
                    }
                }
            }
        }
    }
    break;
    case MAP_TYPE_SMALL_DUNGEON:
    {
        const int depth = 8;
        const int min_room_size = 4;
        const bool room_walls = true;
        const bool random_rooms = false;

        TCOD_bsp_t *const bsp = TCOD_bsp_new_with_size(0, 0, MAP_WIDTH, MAP_HEIGHT);
        TCOD_bsp_split_recursive(
            bsp,
            world->random,
            depth,
            min_room_size + (room_walls ? 1 : 0),
            min_room_size + (room_walls ? 1 : 0),
            1.5f,
            1.5f);

        struct traverse_node_data traverse_node_data;
        traverse_node_data.map = map;
        traverse_node_data.min_room_size = min_room_size;
        traverse_node_data.room_walls = room_walls;
        traverse_node_data.random_rooms = random_rooms;
        TCOD_bsp_traverse_inverted_level_order(bsp, traverse_node, &traverse_node_data);

        TCOD_bsp_delete(bsp);
    }
    break;
    case MAP_TYPE_CAVES:
    {
        struct room *const room = room_new(0, 0, MAP_WIDTH, MAP_HEIGHT);
        TCOD_list_push(map->rooms, room);
    }
    break;
    case MAP_TYPE_GRASSY_CAVES:
    {
        struct room *const room = room_new(0, 0, MAP_WIDTH, MAP_HEIGHT);
        TCOD_list_push(map->rooms, room);
    }
    break;
    case MAP_TYPE_RUINS:
    {
        struct room *const room = room_new(0, 0, MAP_WIDTH, MAP_HEIGHT);
        TCOD_list_push(map->rooms, room);
    }
    break;
    case NUM_MAP_TYPES:
        break;
    }

    // populate doors
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            bool put_door = false;

            struct tile *const tile = &map->tiles[x][y];
            if (tile->type == TILE_TYPE_FLOOR && TCOD_random_get_float(world->random, 0, 1) < DOOR_CHANCE)
            {
                if (map->tiles[x][y - 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x + 1][y - 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y - 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y].type == TILE_TYPE_WALL &&
                    map->tiles[x + 1][y].type == TILE_TYPE_WALL)
                {
                    put_door = true;
                }
                if (map->tiles[x + 1][y].type == TILE_TYPE_FLOOR &&
                    map->tiles[x + 1][y - 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x + 1][y + 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x][y + 1].type == TILE_TYPE_WALL &&
                    map->tiles[x][y - 1].type == TILE_TYPE_WALL)
                {
                    put_door = true;
                }
                if (map->tiles[x][y + 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x + 1][y + 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y + 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y].type == TILE_TYPE_WALL &&
                    map->tiles[x + 1][y].type == TILE_TYPE_WALL)
                {
                    put_door = true;
                }
                if (map->tiles[x - 1][y].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y - 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y + 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x][y + 1].type == TILE_TYPE_WALL &&
                    map->tiles[x][y - 1].type == TILE_TYPE_WALL)
                {
                    put_door = true;
                }
            }

            if (put_door)
            {
                struct object *const object = object_new(
                    OBJECT_TYPE_DOOR_CLOSED,
                    map->floor,
                    x,
                    y,
                    TCOD_white,
                    -1,
                    TCOD_white,
                    0.0f,
                    false);

                TCOD_list_push(map->objects, object);

                tile->object = object;
            }
        }
    }

    // create stairs down
    {
        do
        {
            room_get_random_pos(
                map_get_random_room(map),
                (int *)&map->stair_down_x,
                (int *)&map->stair_down_y);
        } while (map->tiles[map->stair_down_x][map->stair_down_y].type == TILE_TYPE_FLOOR &&
                 map->tiles[map->stair_down_x][map->stair_down_y].object != NULL);

        struct object *const stair_down = object_new(
            OBJECT_TYPE_STAIR_DOWN,
            map->floor,
            map->stair_down_x,
            map->stair_down_y,
            TCOD_white,
            -1,
            TCOD_white,
            0.0f,
            false);

        map->tiles[stair_down->x][stair_down->y].object = stair_down;

        TCOD_list_push(map->objects, stair_down);
    }

    // create stairs up
    {
        do
        {
            room_get_random_pos(
                map_get_random_room(map),
                (int *)&map->stair_up_x,
                (int *)&map->stair_up_y);
        } while (map->tiles[map->stair_up_x][map->stair_up_y].type == TILE_TYPE_FLOOR &&
                 map->tiles[map->stair_up_x][map->stair_up_y].object != NULL);

        struct object *const stair_up = object_new(
            OBJECT_TYPE_STAIR_UP,
            map->floor,
            map->stair_up_x,
            map->stair_up_y,
            TCOD_white,
            -1,
            TCOD_white,
            0.0f,
            false);

        map->tiles[stair_up->x][stair_up->y].object = stair_up;

        TCOD_list_push(map->objects, stair_up);
    }

    // spawn objects
    for (int i = 0; i < SPAWN_OBJECTS; i++)
    {
        int x, y;
        do
        {
            room_get_random_pos(
                map_get_random_room(map),
                &x, &y);
        } while (map->tiles[x][y].type == TILE_TYPE_FLOOR &&
                 map->tiles[x][y].object != NULL);

        enum object_type type = 0;
        TCOD_color_t color = TCOD_white;
        int light_radius = -1;
        TCOD_color_t light_color = TCOD_white;
        float light_intensity = 0.0f;
        bool light_flicker = false;
        switch (TCOD_random_get_int(world->random, 0, 5))
        {
        case 0:
        {
            type = OBJECT_TYPE_ALTAR;
            light_radius = 3;
            light_color = TCOD_white;
            light_intensity = 0.1f;
            light_flicker = false;
        }
        break;
        case 1:
        {
            TCOD_color_t random_color = TCOD_color_RGB(
                TCOD_random_get_int(world->random, 0, 255),
                TCOD_random_get_int(world->random, 0, 255),
                TCOD_random_get_int(world->random, 0, 255));
            type = OBJECT_TYPE_BRAZIER;
            color = random_color;
            light_radius = TCOD_random_get_int(world->random, 5, 20);
            light_color = random_color;
            light_intensity = TCOD_random_get_float(world->random, 0.1f, 0.2f);
            light_flicker = TCOD_random_get_int(world->random, 0, 1) == 0;
        }
        break;
        case 2:
        {
            type = OBJECT_TYPE_CHEST;
            color = TCOD_sepia;
        }
        break;
        case 3:
        {
            type = OBJECT_TYPE_FOUNTAIN;
            color = TCOD_blue;
        }
        break;
        case 4:
        {
            type = OBJECT_TYPE_THRONE;
            color = TCOD_yellow;
        }
        break;
        case 5:
        {
            type = OBJECT_TYPE_TRAP;
        }
        break;
        }

        struct object *const object = object_new(
            type,
            map->floor,
            x,
            y,
            color,
            light_radius,
            light_color,
            light_intensity,
            light_flicker);

        map->tiles[x][y].object = object;

        TCOD_list_push(map->objects, object);
    }

    // spawn adventurers
    for (int i = 0; i < SPAWN_ADVENTURERS; i++)
    {
        int x, y;
        do
        {
            room_get_random_pos(
                map_get_random_room(map),
                &x, &y);
        } while (map->tiles[x][y].type == TILE_TYPE_FLOOR &&
                 map->tiles[x][y].actor != NULL && map->tiles[x][y].object != NULL);

        enum race race = TCOD_random_get_int(world->random, RACE_DWARF, RACE_HUMAN);
        char *name;
        switch (race)
        {
        case RACE_DWARF:
        {
            if (TCOD_random_get_int(world->random, 0, 1) == 0)
            {
                name = "dwarf male";
            }
            else
            {
                name = "dwarf female";
            }
        }
        break;
        default:
        {
            if (TCOD_random_get_int(world->random, 0, 1) == 0)
            {
                name = "standard male";
            }
            else
            {
                name = "standard female";
            }
        }
        break;
        }

        struct actor *const actor = actor_new(
            TCOD_namegen_generate(name, false),
            race,
            TCOD_random_get_int(world->random, CLASS_MAGE, CLASS_WARRIOR),
            FACTION_ADVENTURER,
            map->floor + 1,
            map->floor,
            x,
            y,
            TCOD_random_get_int(world->random, 0, 20) == 0);

        map->tiles[x][y].actor = actor;

        TCOD_list_push(map->actors, actor);
    }

    // spawn monsters
    for (int i = 0; i < SPAWN_MONSTERS; i++)
    {
        int x, y;
        do
        {
            room_get_random_pos(
                map_get_random_room(map),
                &x, &y);
        } while (map->tiles[x][y].type == TILE_TYPE_FLOOR &&
                 map->tiles[x][y].actor != NULL && map->tiles[x][y].object != NULL);

        // TODO: leveled lists
        enum monster monster = TCOD_random_get_int(world->random, 0, NUM_MONSTERS - 1);
        struct actor_prototype *monster_prototype = &monster_prototypes[monster];

        // TODO: monster packs
        struct actor *const actor = actor_new(
            monster_prototype->name,
            monster_prototype->race,
            monster_prototype->class,
            FACTION_MONSTER,
            map->floor + 1,
            map->floor,
            x,
            y,
            TCOD_random_get_int(world->random, 0, 20) == 0);

        map->tiles[x][y].actor = actor;

        TCOD_list_push(map->actors, actor);

        // TODO: default inventory/equipment
        if (monster == MONSTER_BUGBEAR)
        {
            actor->equipment[EQUIP_SLOT_WEAPON] = item_new(ITEM_TYPE_LONGBOW, map->floor, x, y, 1);
            actor->equipment[EQUIP_SLOT_AMMUNITION] = item_new(ITEM_TYPE_ARROW, map->floor, x, y, 5);
        }
    }

    // spawn items
    for (int i = 0; i < SPAWN_ITEMS; i++)
    {
        int x, y;
        do
        {
            room_get_random_pos(
                map_get_random_room(map),
                &x, &y);
        } while (map->tiles[x][y].type == TILE_TYPE_FLOOR &&
                 map->tiles[x][y].object != NULL);

        enum item_type type;
        do
        {
            type = TCOD_random_get_int(world->random, 0, NUM_ITEM_TYPES - 1);
        } while (item_data[type].unique ? !item_data[type].spawned : false);

        struct item *const item = item_new(
            type,
            map->floor,
            x,
            y,
            type == ITEM_TYPE_GOLD
                ? TCOD_random_get_int(world->random, 1, 10 * map->floor)
                : base_item_data[item_data[type].type].max_stack);

        TCOD_list_push(map->tiles[x][y].items, item);

        TCOD_list_push(map->items, item);
    }
}

bool map_is_inside(const int x, const int y)
{
    return x >= 0 && x < MAP_WIDTH &&
           y >= 0 && y < MAP_HEIGHT;
}

struct room *map_get_random_room(const struct map *const map)
{
    return TCOD_list_get(map->rooms, TCOD_random_get_int(world->random, 0, TCOD_list_size(map->rooms) - 1));
}

bool map_is_transparent(
    const struct map *const map,
    const int x,
    const int y)
{
    const struct tile *const tile = &map->tiles[x][y];

    if (tile->object && !object_data[tile->object->type].is_transparent)
    {
        return false;
    }

    return tile_data[tile->type].is_transparent;
}

bool map_is_walkable(
    const struct map *const map,
    const int x,
    const int y)
{
    const struct tile *const tile = &map->tiles[x][y];

    if (tile->object &&
        !object_data[tile->object->type].is_walkable &&
        tile->object->type != OBJECT_TYPE_DOOR_CLOSED)
    {
        return false;
    }

    if (tile->actor)
    {
        return false;
    }

    return tile_data[tile->type].is_walkable;
}

TCOD_map_t map_to_TCOD_map(const struct map *const map)
{
    TCOD_map_t TCOD_map = TCOD_map_new(MAP_WIDTH, MAP_HEIGHT);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            TCOD_map_set_properties(
                TCOD_map,
                x,
                y,
                map_is_transparent(map, x, y),
                map_is_walkable(map, x, y));
        }
    }

    return TCOD_map;
}

TCOD_map_t map_to_fov_map(
    const struct map *const map,
    const int x,
    const int y,
    const int radius)
{
    TCOD_map_t fov_map = map_to_TCOD_map(map);
    TCOD_map_compute_fov(fov_map, x, y, radius, true, FOV_RESTRICTIVE);
    return fov_map;
}
