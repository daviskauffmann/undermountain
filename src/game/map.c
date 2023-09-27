#include "map.h"

#include "actor.h"
#include "data.h"
#include "explosion.h"
#include "object.h"
#include "projectile.h"
#include "room.h"
#include "util.h"
#include "world.h"

#define MIN_ROOM_ATTEMPTS 10
#define MAX_ROOM_ATTEMPTS 30
#define MIN_ROOM_SIZE_VARIATION 3
#define MID_ROOM_SIZE_VARIATION 6
#define MAX_ROOM_SIZE_VARIATION 9
#define MIN_ROOM_BUFFER 1
#define MAX_ROOM_BUFFER 3
#define MIN_PREVENT_OVERLAP_CHANCE 0
#define MAX_PREVENT_OVERLAP_CHANCE 1
#define MIN_DOOR_CHANCE 0
#define MAX_DOOR_CHANCE 1
#define MIN_OBJECTS 1
#define MAX_OBJECTS 3
#define MIN_ADVENTURERS 1
#define MAX_ADVENTURERS 3
#define MIN_MONSTERS 10
#define MAX_MONSTERS 20
#define MIN_ITEMS 2
#define MAX_ITEMS 8

void map_init(struct map *const map, const int floor)
{
    map->floor = floor;
    map->stair_down_x = 0;
    map->stair_down_y = 0;
    map->stair_up_x = 0;
    map->stair_up_y = 0;
    for (size_t x = 0; x < MAP_WIDTH; x++)
    {
        for (size_t y = 0; y < MAP_HEIGHT; y++)
        {
            tile_init(&map->tiles[x][y], TILE_TYPE_EMPTY, false);
        }
    }
    map->rooms = list_new();
    map->objects = list_new();
    map->actors = list_new();
    map->corpses = list_new();
    map->items = list_new();
    map->projectiles = list_new();
    map->explosions = list_new();
    map->surfaces = list_new();
    map->current_actor_index = 0;
}

void map_uninit(struct map *const map)
{
    for (size_t i = 0; i < map->surfaces->size; i++)
    {
        surface_delete(list_get(map->surfaces, i));
    }
    list_delete(map->surfaces);

    for (size_t i = 0; i < map->explosions->size; i++)
    {
        explosion_delete(list_get(map->explosions, i));
    }
    list_delete(map->explosions);

    for (size_t i = 0; i < map->projectiles->size; i++)
    {
        projectile_delete(list_get(map->projectiles, i));
    }
    list_delete(map->projectiles);

    for (size_t i = 0; i < map->items->size; i++)
    {
        item_delete(list_get(map->items, i));
    }
    list_delete(map->items);

    for (size_t i = 0; i < map->corpses->size; i++)
    {
        corpse_delete(list_get(map->corpses, i));
    }
    list_delete(map->corpses);

    for (size_t i = 0; i < map->actors->size; i++)
    {
        actor_delete(list_get(map->actors, i));
    }
    list_delete(map->actors);

    for (size_t i = 0; i < map->objects->size; i++)
    {
        object_delete(list_get(map->objects, i));
    }
    list_delete(map->objects);

    for (size_t i = 0; i < map->rooms->size; i++)
    {
        room_delete(list_get(map->rooms, i));
    }
    list_delete(map->rooms);

    for (size_t x = 0; x < MAP_WIDTH; x++)
    {
        for (size_t y = 0; y < MAP_HEIGHT; y++)
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

void map_generate(struct map *const map)
{
    // setup default tile state
    for (size_t x = 0; x < MAP_WIDTH; x++)
    {
        for (size_t y = 0; y < MAP_HEIGHT; y++)
        {
            struct tile *const tile = &map->tiles[x][y];

            tile->type = TILE_TYPE_EMPTY;
        }
    }

    // create rooms
    const int num_room_attempts = TCOD_random_get_int(world->random, MIN_ROOM_ATTEMPTS, MAX_ROOM_ATTEMPTS);
    const int min_room_size = TCOD_random_get_int(world->random, MIN_ROOM_SIZE_VARIATION, MID_ROOM_SIZE_VARIATION);
    const int max_room_size = TCOD_random_get_int(world->random, MID_ROOM_SIZE_VARIATION, MAX_ROOM_SIZE_VARIATION);
    const int room_buffer = TCOD_random_get_int(world->random, MIN_ROOM_BUFFER, MAX_ROOM_BUFFER);
    const float prevent_overlap_chance = TCOD_random_get_float(world->random, MIN_PREVENT_OVERLAP_CHANCE, MAX_PREVENT_OVERLAP_CHANCE);

    for (size_t i = 0; i < num_room_attempts; i++)
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

        list_add(
            map->rooms,
            room_new(room_x, room_y, room_w, room_h));
    }

    for (size_t i = 0; i < map->rooms->size - 1; i++)
    {
        int x1, y1;
        room_get_random_pos(
            list_get(map->rooms, i),
            &x1, &y1);

        int x2, y2;
        room_get_random_pos(
            list_get(map->rooms, i + 1),
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

    // populate doors
    const float door_chance = TCOD_random_get_float(world->random, MIN_DOOR_CHANCE, MAX_DOOR_CHANCE);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            bool put_door = false;

            struct tile *const tile = &map->tiles[x][y];
            if (tile->type == TILE_TYPE_FLOOR)
            {
                if (
                    map->tiles[x][y - 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x + 1][y - 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y - 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y].type == TILE_TYPE_WALL &&
                    map->tiles[x + 1][y].type == TILE_TYPE_WALL)
                {
                    put_door = true;
                }
                else if (
                    map->tiles[x + 1][y].type == TILE_TYPE_FLOOR &&
                    map->tiles[x + 1][y - 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x + 1][y + 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x][y + 1].type == TILE_TYPE_WALL &&
                    map->tiles[x][y - 1].type == TILE_TYPE_WALL)
                {
                    put_door = true;
                }
                else if (
                    map->tiles[x][y + 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x + 1][y + 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y + 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y].type == TILE_TYPE_WALL &&
                    map->tiles[x + 1][y].type == TILE_TYPE_WALL)
                {
                    put_door = true;
                }
                else if (
                    map->tiles[x - 1][y].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y - 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x - 1][y + 1].type == TILE_TYPE_FLOOR &&
                    map->tiles[x][y + 1].type == TILE_TYPE_WALL &&
                    map->tiles[x][y - 1].type == TILE_TYPE_WALL)
                {
                    put_door = true;
                }
            }

            if (put_door && TCOD_random_get_float(world->random, 0, 1) < door_chance)
            {
                struct object *const object = object_new(
                    OBJECT_TYPE_DOOR_CLOSED,
                    map->floor,
                    x, y);

                list_add(map->objects, object);

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
            map->stair_down_y);

        list_add(map->objects, stair_down);

        map->tiles[stair_down->x][stair_down->y].object = stair_down;
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
            map->stair_up_y);

        list_add(map->objects, stair_up);

        map->tiles[stair_up->x][stair_up->y].object = stair_up;
    }

    // spawn objects
    const int num_objects = TCOD_random_get_int(world->random, MIN_OBJECTS, MAX_OBJECTS);
    for (int i = 0; i < num_objects; i++)
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
        switch (TCOD_random_get_int(world->random, 0, 5))
        {
        case 0:
        {
            type = OBJECT_TYPE_ALTAR;
        }
        break;
        case 1:
        {
            type = OBJECT_TYPE_BRAZIER;
        }
        break;
        case 2:
        {
            type = OBJECT_TYPE_CHEST;
        }
        break;
        case 3:
        {
            type = OBJECT_TYPE_FOUNTAIN;
        }
        break;
        case 4:
        {
            type = OBJECT_TYPE_THRONE;
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
            x, y);

        list_add(map->objects, object);

        map->tiles[x][y].object = object;
    }

    // spawn adventurers
    const int num_adventurers = TCOD_random_get_int(world->random, MIN_ADVENTURERS, MAX_ADVENTURERS);
    for (int i = 0; i < num_adventurers; i++)
    {
        int x, y;
        do
        {
            room_get_random_pos(
                map_get_random_room(map),
                &x, &y);
        } while (map->tiles[x][y].type == TILE_TYPE_FLOOR &&
                 map->tiles[x][y].actor != NULL &&
                 map->tiles[x][y].object != NULL);

        enum race race = TCOD_random_get_int(world->random, PLAYER_RACE_BEGIN, PLAYER_RACE_END);
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

        const enum class class = TCOD_random_get_int(world->random, PLAYER_CLASS_BEGIN, PLAYER_CLASS_END);

        struct actor *const actor = actor_new(
            TCOD_namegen_generate(name, false),
            race,
            class,
            map->floor + 1,
            class_database[class].default_ability_scores,
            (bool[NUM_FEATS]){0},
            FACTION_ADVENTURER,
            map->floor,
            x, y);

        list_add(map->actors, actor);

        map->tiles[x][y].actor = actor;
    }

    // spawn monsters
    const int num_monsters = TCOD_random_get_int(world->random, MIN_MONSTERS, MAX_MONSTERS);
    for (int i = 0; i < num_monsters; i++)
    {
        int x, y;
        do
        {
            room_get_random_pos(
                map_get_random_room(map),
                &x, &y);
        } while (map->tiles[x][y].type == TILE_TYPE_FLOOR &&
                 map->tiles[x][y].actor != NULL &&
                 map->tiles[x][y].object != NULL);

        enum monster monster;
        const struct actor_prototype *monster_prototype;
        do
        {
            monster = TCOD_random_get_int(world->random, 0, NUM_MONSTERS - 1);
            monster_prototype = &monster_prototypes[monster];
        } while (monster_prototype->level > map->floor + 1);

        // TODO: monster packs
        struct actor *const actor = actor_new(
            monster_prototype->name,
            monster_prototype->race,
            monster_prototype->class,
            monster_prototype->level,
            monster_prototype->ability_scores,
            monster_prototype->feats,
            FACTION_MONSTER,
            map->floor,
            x, y);

        for (enum equip_slot equip_slot = EQUIP_SLOT_NONE + 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
        {
            if (monster_prototype->equipment[equip_slot] != EQUIP_SLOT_NONE)
            {
                actor->equipment[equip_slot] = item_new(
                    monster_prototype->equipment[equip_slot],
                    0,
                    0, 0,
                    base_item_database[item_database[monster_prototype->equipment[equip_slot]].type].max_stack);
            }
        }

        for (enum item_type item_type = ITEM_TYPE_NONE + 1; item_type < NUM_ITEM_TYPES; item_type++)
        {
            const int stack = monster_prototype->items[item_type];

            if (stack > 0)
            {
                struct item *const item = item_new(
                    item_type,
                    0,
                    0, 0,
                    stack);

                list_add(actor->items, item);
            }
        }

        list_add(map->actors, actor);

        map->tiles[x][y].actor = actor;
    }

    // spawn items
    const int num_items = TCOD_random_get_int(world->random, MIN_ITEMS, MAX_ITEMS);
    for (int i = 0; i < num_items; i++)
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
            type = TCOD_random_get_int(world->random, ITEM_TYPE_NONE + 1, NUM_ITEM_TYPES - 1);
        } while (item_database[type].level > map->floor + 1 ||
                 (item_database[type].unique && list_contains(world->spawned_unique_item_types, (void *)(size_t)type)));

        struct item *const item = item_new(
            type,
            map->floor,
            x, y,
            type == ITEM_TYPE_GOLD
                ? TCOD_random_get_int(world->random, 1, 10 * (map->floor + 1))
                : base_item_database[item_database[type].type].max_stack);

        if (item_database[type].unique)
        {
            list_add(world->spawned_unique_item_types, (void *)(size_t)type);
        }

        list_add(map->items, item);

        list_add(map->tiles[x][y].items, item);
    }
}

bool map_is_inside(const int x, const int y)
{
    return x >= 0 && x < MAP_WIDTH &&
           y >= 0 && y < MAP_HEIGHT;
}

struct room *map_get_random_room(const struct map *const map)
{
    return list_get(map->rooms, TCOD_random_get_int(world->random, 0, (int)map->rooms->size - 1));
}

bool map_is_transparent(
    const struct map *const map,
    const int x,
    const int y)
{
    const struct tile *const tile = &map->tiles[x][y];

    if (tile->object && !object_database[tile->object->type].is_transparent)
    {
        return false;
    }

    return tile_database[tile->type].is_transparent;
}

bool map_is_walkable(
    const struct map *const map,
    const int x,
    const int y)
{
    const struct tile *const tile = &map->tiles[x][y];

    if (tile->object &&
        !object_database[tile->object->type].is_walkable &&
        tile->object->type != OBJECT_TYPE_DOOR_CLOSED)
    {
        return false;
    }

    if (tile->actor)
    {
        return false;
    }

    return tile_database[tile->type].is_walkable;
}

bool map_is_animation_playing(const struct map *const map)
{
    if (map->projectiles->size > 0)
    {
        return true;
    }

    if (map->explosions->size > 0)
    {
        return true;
    }

    return false;
}

TCOD_Map *map_to_TCOD_map(const struct map *const map)
{
    TCOD_Map *TCOD_map = TCOD_map_new(MAP_WIDTH, MAP_HEIGHT);

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

TCOD_Map *map_to_fov_map(
    const struct map *const map,
    const int x,
    const int y,
    const int radius)
{
    TCOD_Map *fov_map = map_to_TCOD_map(map);
    TCOD_map_compute_fov(fov_map, x, y, radius, true, FOV_SHADOW);
    return fov_map;
}
