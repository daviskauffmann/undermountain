#include "world.h"

#include <assert.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "actor.h"
#include "message.h"
#include "object.h"
#include "projectile.h"
#include "util.h"

// TODO: items and equipment that are inside inventories have their positions painstakingly updated when their carrier moves
// is this necessary?

// TODO: there is a lot of repetition of things with light properties
// pack them into a light struct?

// TODO: resurrect corpses

// TODO: optimize
// pathfinding takes a while

// TODO: traps

// TOOD: chests

// TODO: sound and sound propogation
// play "sounds" in the game such as footsteps and monster noises
// calculate whether the player can hear and identify the sound and put it in the log if so
// sound "reflection"?

// TOOD: redo map generation
// no need for overworld map, world will start on the first dungeon
// we need to define a win condition for the world, probably just grabbing an amulet or something and returning to the entrance?
// ascending the first floor stairs will end the world
// maps should support multiple stairs
// the stairs should geographically connect to the above/below floors, so that will have to be taken into account during map generation
// thus, the stair object should be a little more sophisticated. it should hold a stair_id which connects it to another stair with the same id
// using the stair will just teleport the player to the other stair
// this simplifies the code a little bit because we no longer need to have ascend() and descend() functions, just one that handles moving from one stair to the other
// making a system that is aware of the layout of the entire dungeon allows for the possibility of "pit traps", which make the player (or creatures) fall to a lower floor
// these pit traps will be generated in a similar way to the stairs, in that they can only appear if the same tile coordinate on the map below is a floor
// in addition to all that, there should be ladders that connect to smaller maps that contain loot and enemies
// these maps have no way out other than the entrance and are smaller

// TODO: implement processing of maps the player is not on?

// TODO: not storing all maps in memory?
// if we have a small number of maps, this might not be a problem
// might interfere with the above todo

// TODO: have different TCOD_random_t instances for different things

// TODO: stop using TCOD_list_t for things that maps store
// a dynamically growing contiguous array would be a lot better
// linked lists are faster for insertion/deletion, but that happens very rarely in games compared to traversal/random access
// that way, the CPU cache will be more efficiently used when looping over stuff in maps
// the lists of pointers that tiles store could probably stay the way it is, since it contains pointers that would be spread out in memory anyways

struct world *world;

void world_init(void)
{
    world = malloc(sizeof(struct world));
    assert(world);
    world->state = WORLD_STATE_PLAY;
    for (int floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *map = &world->maps[floor];
        map_init(map, floor);
    }
    world->messages = TCOD_list_new();
    world->player = NULL;
    world->turn = 0;
}

void world_quit(void)
{
    TCOD_LIST_FOREACH(world->messages)
    {
        struct message *message = *iterator;
        message_delete(message);
    }
    TCOD_list_delete(world->messages);
    for (int i = 0; i < NUM_MAPS; i++)
    {
        struct map *map = &world->maps[i];
        map_reset(map);
    }
    free(world);
    world = NULL;
}

// TODO: this should accept an actor which will become the player, presumably passed from a character creation menu
void world_new(void)
{
    for (int floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *map = &world->maps[floor];
        map_generate(map);
    }

    // DEBUG: spawn stuff
    {
        int floor = 0;
        struct map *map = &world->maps[floor];

        // DEBUG: create player
        {
            int x = map->stair_up_x;
            int y = map->stair_up_y;
            struct actor *player = actor_new("Blinky", RACE_HUMAN, CLASS_WARRIOR, FACTION_GOOD, floor + 1, floor, x, y);
            player->torch = false;
            world->player = player;
            TCOD_list_push(map->actors, player);
            struct tile *tile = &map->tiles[x][y];
            tile->actor = player;
            struct item *bodkin_arrow = item_new(ITEM_TYPE_BODKIN_ARROW, floor, x, y, 50);
            TCOD_list_push(map->items, bodkin_arrow);
            TCOD_list_push(player->items, bodkin_arrow);
            struct item *bolt = item_new(ITEM_TYPE_BOLT, floor, x, y, 50);
            TCOD_list_push(map->items, bolt);
            TCOD_list_push(player->items, bolt);
            struct item *crossbow = item_new(ITEM_TYPE_CROSSBOW, floor, x, y, 1);
            TCOD_list_push(map->items, crossbow);
            TCOD_list_push(player->items, crossbow);
            struct item *iron_armor = item_new(ITEM_TYPE_IRON_ARMOR, floor, x, y, 1);
            TCOD_list_push(map->items, iron_armor);
            TCOD_list_push(player->items, iron_armor);
            struct item *greatsword = item_new(ITEM_TYPE_GREATSWORD, floor, x, y, 1);
            TCOD_list_push(map->items, greatsword);
            TCOD_list_push(player->items, greatsword);
            struct item *longsword = item_new(ITEM_TYPE_LONGSWORD, floor, x, y, 1);
            TCOD_list_push(map->items, longsword);
            TCOD_list_push(player->items, longsword);
            struct item *longbow = item_new(ITEM_TYPE_LONGBOW, floor, x, y, 1);
            TCOD_list_push(map->items, longbow);
            TCOD_list_push(player->items, longbow);
            struct item *kite_shield = item_new(ITEM_TYPE_KITE_SHIELD, floor, x, y, 1);
            TCOD_list_push(map->items, kite_shield);
            TCOD_list_push(player->items, kite_shield);
            struct item *healing_potion = item_new(ITEM_TYPE_HEALING_POTION, floor, x, y, 1);
            TCOD_list_push(map->items, healing_potion);
            TCOD_list_push(player->items, healing_potion);

            world_log(
                floor,
                x,
                y,
                TCOD_white,
                "Hail, %s!",
                world->player->name);
        }

        // DEBUG: create pet
        {
            int x = map->stair_up_x + 1;
            int y = map->stair_up_y + 1;
            struct actor *pet = actor_new("Spot", RACE_ANIMAL, CLASS_ANIMAL, FACTION_GOOD, floor + 1, floor, x, y);
            pet->leader = world->player;
            pet->glow = false;
            pet->torch = false;
            TCOD_list_push(map->actors, pet);
            struct tile *tile = &map->tiles[x][y];
            tile->actor = pet;
        }
    }
}

void world_save(const char *filename)
{
    TCOD_zip_t zip = TCOD_zip_new();
    TCOD_zip_save_to_file(zip, filename);
    // TODO: save world to zip
    TCOD_zip_delete(zip);
    printf("World saved\n");
}

void world_load(const char *filename)
{
    TCOD_zip_t zip = TCOD_zip_new();
    TCOD_zip_load_from_file(zip, filename);
    // TODO: load world from zip
    TCOD_zip_delete(zip);
    world_new(); // DEBUG: just start a new world
    printf("World loaded\n");
}

void world_update(void)
{
    world->state = world->player->dead ? WORLD_STATE_LOSE : WORLD_STATE_PLAY;

    struct map *map = &world->maps[world->player->floor];
    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;
        if (actor->dead)
        {
            actor->current_hp = 0;
            // move actor to corpses array
            iterator = TCOD_list_remove_iterator(map->actors, iterator);
            TCOD_list_push(map->corpses, actor);
            struct tile *tile = &map->tiles[actor->x][actor->y];
            TCOD_list_push(tile->corpses, actor);
            tile->actor = NULL;
            if (actor != world->player)
            {
                // move equipment to inventory
                for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
                {
                    struct item *equipment = actor->equipment[i];
                    if (equipment)
                    {
                        TCOD_list_push(actor->items, equipment);
                        actor->equipment[i] = NULL;
                    }
                }
                // move inventory to ground
                TCOD_LIST_FOREACH(actor->items)
                {
                    struct item *item = *iterator;
                    struct map *map = &world->maps[item->floor];
                    struct tile *tile = &map->tiles[item->x][item->y];
                    TCOD_list_push(tile->items, item);
                    iterator = TCOD_list_remove_iterator(actor->items, iterator);
                }
            }
            if (actor == world->player)
            {
                // let the player see whats going on while they're dead
                actor_calc_fov(actor);
            }
            continue;
        }
        actor_update_flash(actor);
    }
    TCOD_LIST_FOREACH(map->projectiles)
    {
        struct projectile *projectile = *iterator;
        projectile_update(projectile);
        if (projectile->destroyed)
        {
            iterator = TCOD_list_remove_iterator(map->projectiles, iterator);
            projectile_delete(projectile);
            continue;
        }
    }
}

void world_turn(void)
{
    world->turn++;

    struct map *map = &world->maps[world->player->floor];
    TCOD_LIST_FOREACH(map->objects)
    {
        struct object *object = *iterator;
        if (object->destroyed)
        {
            struct tile *tile = &map->tiles[object->x][object->y];
            tile->object = NULL;
            iterator = TCOD_list_remove_iterator(map->objects, iterator);
            object_delete(object);
            continue;
        }
        object_calc_light(object);
    }
    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;
        actor_calc_light(actor);
    }
    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;
        actor_calc_fov(actor);
    }
    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;
		if (actor != world->player) {
			actor_ai(actor);
		}
    }
    if (world->player->dead)
    {
        // let the player see whats going on while they're dead
        actor_calc_fov(world->player);
    }
}

// TODO: we should proably redo the logging system
// instead of logging messages directly, the world should store all the events that have happened
// the renderer can read the last few events to generate a message (using assets as template strings)
// this allows us to read the entire history of the world and do anything with it
// also, we'd need to store whether the event was initially seen by the player
void world_log(int floor, int x, int y, TCOD_color_t color, char *fmt, ...)
{
    // if (!world->player ||
    //     floor != world->player->floor ||
    //     !world->player->fov ||
    //     !TCOD_map_is_in_fov(world->player->fov, x, y))
    // {
    //     return;
    // }

    va_list args;
    va_start(args, fmt);
    size_t size = vsnprintf(NULL, 0, fmt, args);
    char *buffer = malloc(size + 1);
    assert(buffer);
    vsprintf(buffer, fmt, args);
    va_end(args);
    char *line_begin = buffer;
    char *line_end;
    do
    {
        if (TCOD_list_size(world->messages) == (50 / 4) - 2) // TODO: hardcoded console_height for now because we're gonna throw all this away soon
        {
            struct message *message = TCOD_list_get(world->messages, 0);
            TCOD_list_remove(world->messages, message);
            message_delete(message);
        }

        line_end = strchr(line_begin, '\n');
        if (line_end)
        {
            *line_end = '\0';
        }
        printf("%s\n", line_begin);
        struct message *message = message_new(line_begin, color);
        TCOD_list_push(world->messages, message);
        line_begin = line_end + 1;
    } while (line_end);
    free(buffer);
}
