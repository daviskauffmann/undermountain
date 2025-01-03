#include "map.h"

#include "actor.h"
#include "base_item.h"
#include "class.h"
#include "corpse.h"
#include "explosion.h"
#include "faction.h"
#include "list.h"
#include "monster_pack.h"
#include "object.h"
#include "projectile.h"
#include "race.h"
#include "room.h"
#include "surface.h"
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
#define MIN_OBJECT_CHANCE 0
#define MAX_OBJECT_CHANCE 1
#define MIN_ITEM_CHANCE 0
#define MAX_ITEM_CHANCE 1
#define MIN_ADVENTURER_CHANCE 0
#define MAX_ADVENTURER_CHANCE 0.1f
#define MIN_MONSTER_PACK_CHANCE 0
#define MAX_MONSTER_PACK_CHANCE 1

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
    for (size_t surface_index = 0; surface_index < map->surfaces->size; surface_index++)
    {
        surface_delete(list_get(map->surfaces, surface_index));
    }
    list_delete(map->surfaces);

    for (size_t explosion_index = 0; explosion_index < map->explosions->size; explosion_index++)
    {
        explosion_delete(list_get(map->explosions, explosion_index));
    }
    list_delete(map->explosions);

    for (size_t projectile_index = 0; projectile_index < map->projectiles->size; projectile_index++)
    {
        projectile_delete(list_get(map->projectiles, projectile_index));
    }
    list_delete(map->projectiles);

    for (size_t item_index = 0; item_index < map->items->size; item_index++)
    {
        item_delete(list_get(map->items, item_index));
    }
    list_delete(map->items);

    for (size_t corpse_index = 0; corpse_index < map->corpses->size; corpse_index++)
    {
        corpse_delete(list_get(map->corpses, corpse_index));
    }
    list_delete(map->corpses);

    for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
    {
        actor_delete(list_get(map->actors, actor_index));
    }
    list_delete(map->actors);

    for (size_t object_index = 0; object_index < map->objects->size; object_index++)
    {
        object_delete(list_get(map->objects, object_index));
    }
    list_delete(map->objects);

    for (size_t room_index = 0; room_index < map->rooms->size; room_index++)
    {
        room_delete(list_get(map->rooms, room_index));
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

void map_generate(struct map *const map, TCOD_Random *random)
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
    const int num_room_attempts = TCOD_random_get_int(random, MIN_ROOM_ATTEMPTS, MAX_ROOM_ATTEMPTS);
    const int min_room_size = TCOD_random_get_int(random, MIN_ROOM_SIZE_VARIATION, MID_ROOM_SIZE_VARIATION);
    const int max_room_size = TCOD_random_get_int(random, MID_ROOM_SIZE_VARIATION, MAX_ROOM_SIZE_VARIATION);
    const int room_buffer = TCOD_random_get_int(random, MIN_ROOM_BUFFER, MAX_ROOM_BUFFER);
    const float prevent_overlap_chance = TCOD_random_get_float(random, MIN_PREVENT_OVERLAP_CHANCE, MAX_PREVENT_OVERLAP_CHANCE);

    for (size_t room_attempt = 0; room_attempt < num_room_attempts; room_attempt++)
    {
        const int room_x = TCOD_random_get_int(random, 0, MAP_WIDTH);
        const int room_y = TCOD_random_get_int(random, 0, MAP_HEIGHT);
        const int room_w = TCOD_random_get_int(random, min_room_size, max_room_size);
        const int room_h = TCOD_random_get_int(random, min_room_size, max_room_size);
        if (room_x < room_buffer ||
            room_x + room_w > MAP_WIDTH - room_buffer ||
            room_y < room_buffer ||
            room_y + room_h > MAP_HEIGHT - room_buffer)
        {
            continue;
        }

        if (TCOD_random_get_float(random, 0, 1) < prevent_overlap_chance)
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

    for (size_t room_index = 0; room_index < map->rooms->size - 1; room_index++)
    {
        int x1, y1;
        room_get_random_pos(
            list_get(map->rooms, room_index),
            &x1, &y1);

        int x2, y2;
        room_get_random_pos(
            list_get(map->rooms, room_index + 1),
            &x2, &y2);

        if (TCOD_random_get_int(random, 0, 1) == 0)
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
    const float door_chance = TCOD_random_get_float(random, MIN_DOOR_CHANCE, MAX_DOOR_CHANCE);

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

            if (put_door && TCOD_random_get_float(random, 0, 1) < door_chance)
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
    const struct room *stair_down_room;
    {
        do
        {
            stair_down_room = map_get_random_room(map);

            room_get_random_pos(stair_down_room, &map->stair_down_x, &map->stair_down_y);
        } while (map->tiles[map->stair_down_x][map->stair_down_y].type != TILE_TYPE_FLOOR ||
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
    const struct room *stair_up_room;
    {
        do
        {
            stair_up_room = map_get_random_room(map);

            room_get_random_pos(stair_up_room, &map->stair_up_x, &map->stair_up_y);
        } while (map->tiles[map->stair_up_x][map->stair_up_y].type != TILE_TYPE_FLOOR ||
                 map->tiles[map->stair_up_x][map->stair_up_y].object != NULL);

        struct object *const stair_up = object_new(
            OBJECT_TYPE_STAIR_UP,
            map->floor,
            map->stair_up_x,
            map->stair_up_y);

        list_add(map->objects, stair_up);

        map->tiles[stair_up->x][stair_up->y].object = stair_up;
    }

    for (size_t room_index = 0; room_index < map->rooms->size; room_index++)
    {
        const struct room *const room = list_get(map->rooms, room_index);

        // spawn object
        const float object_chance = TCOD_random_get_float(random, MIN_OBJECT_CHANCE, MAX_OBJECT_CHANCE);
        if (TCOD_random_get_float(random, 0, 1) < object_chance)
        {
            int x, y;
            do
            {
                room_get_random_pos(room, &x, &y);
            } while (map->tiles[x][y].type != TILE_TYPE_FLOOR ||
                     map->tiles[x][y].object != NULL);

            enum object_type object_type = 0;
            switch (TCOD_random_get_int(random, 0, 5))
            {
            case 0:
            {
                object_type = OBJECT_TYPE_ALTAR;
            }
            break;
            case 1:
            {
                object_type = OBJECT_TYPE_BRAZIER;
            }
            break;
            case 2:
            {
                object_type = OBJECT_TYPE_CHEST;
            }
            break;
            case 3:
            {
                object_type = OBJECT_TYPE_FOUNTAIN;
            }
            break;
            case 4:
            {
                object_type = OBJECT_TYPE_THRONE;
            }
            break;
            case 5:
            {
                object_type = OBJECT_TYPE_TRAP;
            }
            break;
            }

            struct object *const object = object_new(
                object_type,
                map->floor,
                x, y);

            if (object_type == OBJECT_TYPE_TRAP)
            {
                object->trap_detection_state = OBJECT_TRAP_DETECTION_STATE_UNCHECKED;
            }

            list_add(map->objects, object);

            map->tiles[x][y].object = object;
        }

        // spawn item
        const float item_chance = TCOD_random_get_float(random, MIN_ITEM_CHANCE, MAX_ITEM_CHANCE);
        if (TCOD_random_get_float(random, 0, 1) < item_chance)
        {
            int x, y;
            do
            {
                room_get_random_pos(room, &x, &y);
            } while (map->tiles[x][y].type != TILE_TYPE_FLOOR ||
                     map->tiles[x][y].object != NULL ||
                     map->tiles[x][y].item != NULL);

            enum item_type type;
            do
            {
                type = TCOD_random_get_int(random, ITEM_TYPE_NONE + 1, NUM_ITEM_TYPES - 1);
            } while (item_database[type].level > map->floor + 1 ||
                     (item_database[type].unique && list_contains(world->spawned_unique_item_types, (void *)(size_t)type)));

            int stack = TCOD_random_get_int(random, 1, base_item_database[item_database[type].type].max_stack);

            if (type == ITEM_TYPE_GOLD)
            {
                const int min_stack = 10 * (map->floor + 1);
                const int max_stack = 100 * (map->floor + 1);
                stack = TCOD_random_get_int(random, min_stack, max_stack);
            }

            struct item *const item = item_new(
                type,
                map->floor,
                x, y,
                stack);

            list_add(map->items, item);

            map->tiles[x][y].item = item;
        }

        // spawn adventurer
        const float adventurer_chance = TCOD_random_get_float(random, MIN_ADVENTURER_CHANCE, MAX_ADVENTURER_CHANCE);
        if (TCOD_random_get_float(random, 0, 1) < adventurer_chance)
        {
            int x, y;
            do
            {
                room_get_random_pos(
                    map_get_random_room(map),
                    &x, &y);
            } while (map->tiles[x][y].type != TILE_TYPE_FLOOR ||
                     map->tiles[x][y].actor != NULL ||
                     map->tiles[x][y].object != NULL);

            enum race race = TCOD_random_get_int(random, PLAYER_RACE_BEGIN, PLAYER_RACE_END);
            char *name;
            switch (race)
            {
            case RACE_DWARF:
            {
                if (TCOD_random_get_int(random, 0, 1) == 0)
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
                if (TCOD_random_get_int(random, 0, 1) == 0)
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

            const enum class class = TCOD_random_get_int(random, PLAYER_CLASS_BEGIN, PLAYER_CLASS_END);

            struct actor *const actor = actor_new(
                TCOD_namegen_generate(name, false),
                race,
                class,
                race_database[race].size,
                FACTION_ADVENTURER,
                map->floor + 1,
                class_database[class].default_ability_scores,
                (bool[NUM_SPECIAL_ABILITIES]){false},
                (bool[NUM_SPECIAL_ATTACKS]){false},
                (bool[NUM_FEATS]){false},
                map->floor,
                x, y);

            list_add(map->actors, actor);

            map->tiles[x][y].actor = actor;
        }

        // spawn monster pack if not the entrance room
        if (room != stair_up_room)
        {
            const float monster_pack_chance = TCOD_random_get_float(random, MIN_MONSTER_PACK_CHANCE, MAX_MONSTER_PACK_CHANCE);
            if (TCOD_random_get_float(random, 0, 1) < monster_pack_chance)
            {
                const struct monster_pack_data *monster_pack_data;
                do
                {
                    const enum monster_pack_type monster_pack_type = TCOD_random_get_int(random, 0, NUM_MONSTER_PACK_TYPES - 1);
                    monster_pack_data = &monster_pack_database[monster_pack_type];
                } while (map->floor < monster_pack_data->min_floor ||
                         map->floor > monster_pack_data->max_floor);

                for (enum monster_type monster_type = 0; monster_type < NUM_MONSTER_TYPES; monster_type++)
                {
                    const int min_count = monster_pack_data->monsters[monster_type].min_count;
                    const int max_count = monster_pack_data->monsters[monster_type].max_count;
                    const int count = TCOD_random_get_int(random, min_count, max_count);

                    if (count > 0)
                    {
                        const struct monster_data *const monster_data = &monster_database[monster_type];

                        for (size_t current_monster = 0; current_monster < count; current_monster++)
                        {
                            int x, y;
                            do
                            {
                                room_get_random_pos(room, &x, &y);
                            } while (map->tiles[x][y].type != TILE_TYPE_FLOOR ||
                                     map->tiles[x][y].actor != NULL ||
                                     map->tiles[x][y].object != NULL);

                            struct actor *const actor = actor_new(
                                monster_data->name,
                                monster_data->race,
                                monster_data->class,
                                monster_data->size,
                                monster_data->faction,
                                monster_data->level,
                                monster_data->ability_scores,
                                monster_data->special_abilities,
                                monster_data->special_attacks,
                                monster_data->feats,
                                map->floor,
                                x, y);

                            for (enum equip_slot equip_slot = EQUIP_SLOT_NONE + 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
                            {
                                const enum item_type item_type = monster_data->equipment[equip_slot].type;

                                if (item_type != EQUIP_SLOT_NONE)
                                {
                                    const int min_stack = monster_data->equipment[equip_slot].min_stack;
                                    const int max_stack = monster_data->equipment[equip_slot].max_stack;
                                    const int stack = TCOD_random_get_int(random, min_stack, max_stack);

                                    if (stack > 0)
                                    {
                                        actor->equipment[equip_slot] = item_new(
                                            item_type,
                                            0,
                                            0, 0,
                                            stack);
                                    }
                                }
                            }

                            for (enum item_type item_type = ITEM_TYPE_NONE + 1; item_type < NUM_ITEM_TYPES; item_type++)
                            {
                                const int min_stack = monster_data->items[item_type].min_stack;
                                const int max_stack = monster_data->items[item_type].max_stack;
                                const int stack = TCOD_random_get_int(random, min_stack, max_stack);

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
                    }
                }
            }
        }
    }
}

int map_get_dungeon_level(const struct map *map)
{
    return map->floor + 1;
}

bool map_is_inside(const int x, const int y)
{
    return x >= 0 && x < MAP_WIDTH &&
           y >= 0 && y < MAP_HEIGHT;
}

struct room *map_get_random_room(const struct map *const map)
{
    return list_get(map->rooms, TCOD_random_get_int(NULL, 0, (int)map->rooms->size - 1));
}

void map_find_empty_tile(const struct map *map, int *x, int *y)
{
    while (!map_is_inside(*x, *y) ||
           map->tiles[*x][*y].type != TILE_TYPE_FLOOR ||
           map->tiles[*x][*y].object != NULL ||
           map->tiles[*x][*y].item != NULL ||
           map->tiles[*x][*y].corpse != NULL)
    {
        *x += TCOD_random_get_int(NULL, -1, 1);
        *y += TCOD_random_get_int(NULL, -1, 1);
    }
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
    TCOD_Map *const TCOD_map = TCOD_map_new(MAP_WIDTH, MAP_HEIGHT);

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
    TCOD_map_compute_fov(fov_map, x, y, radius, true, FOV_BASIC);
    return fov_map;
}
