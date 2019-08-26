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

// TODO: resuurect corpses

// TODO: optimize
// decide on a target ms per turn, maybe 16ms
// the world_turn() function should never take longer than that to run

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

// TODO: implement processing of maps the player is not on

// TODO: not storing all maps in memory?
// if we have a small number of maps, this might not be a problem
// might interfere with the above todo

// TODO: have different TCOD_random_t instances for different things

// TODO: possibly redo input to be more like commit 67aa306?
// look at world.c, line 180 in the world_update() function
// this type of input allows for multiple player characters or playable character switching on the fly
// a practical worldplay application might be a mind control spell
// unfortunately, with the implementation i did before, it resulted in each actor taking an entire frame to process
// not because of computation time, but because the actor would do their ai that frame, and then on the next update, it would hit the next actor to process them, and so on
// some research and testing needs to be done to see if this is just a necessary consequence or maybe there is a smarter way to accomplish this
// here is my proposal:
// give all actors a bool to say whether they are input controlled or not
// world_update()
//   if waiting_for_input is true
//     return
//   loop through all the maps
//     loop through all the actors
//       if the current actor has enough energy
//         remove some energy
//         if the current actor is input controlled
//           set waiting_for_input to true
//           set the world->player to the current actor (so we need to independently track the main character for win/lose state reasons?)
//           break out of the loop
//         otherwise
//           process ai for this actor
// scene_game->input()
//   took_turn can be local now, set to false at the beginning
//   do everything as normal, applying input to the current world->player and storing the result in took_turn
//   if took_turn is true and world->player->energy is over a threshold
//     instead of calling world_turn(), just set waiting_for_input to false

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
            struct actor *player = actor_new("Blinky", RACE_HUMAN, CLASS_FIGHTER, FACTION_GOOD, floor + 1, floor, x, y);
            player->torch = false;
            world->player = player;
            TCOD_list_push(map->actors, player);
            struct tile *tile = &map->tiles[x][y];
            tile->actor = player;
            struct item *longsword = item_new(ITEM_TYPE_LONGSWORD, floor, x, y);
            TCOD_list_push(map->items, longsword);
            TCOD_list_push(player->items, longsword);
            struct item *greatsword = item_new(ITEM_TYPE_GREATSWORD, floor, x, y);
            TCOD_list_push(map->items, greatsword);
            TCOD_list_push(player->items, greatsword);
            struct item *longbow = item_new(ITEM_TYPE_LONGBOW, floor, x, y);
            TCOD_list_push(map->items, longbow);
            TCOD_list_push(player->items, longbow);
            struct item *tower_shield = item_new(ITEM_TYPE_TOWER_SHIELD, floor, x, y);
            TCOD_list_push(map->items, tower_shield);
            TCOD_list_push(player->items, tower_shield);
            struct item *potion_cure_light_wounds = item_new(ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS, floor, x, y);
            TCOD_list_push(map->items, potion_cure_light_wounds);
            TCOD_list_push(player->items, potion_cure_light_wounds);

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
            pet->speed = 1.0f;
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
            iterator = TCOD_list_remove_iterator(map->actors, iterator);
            TCOD_list_push(map->corpses, actor);
            struct tile *tile = &map->tiles[actor->x][actor->y];
            TCOD_list_push(tile->corpses, actor);
            tile->actor = NULL;
            if (actor != world->player)
            {
                for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
                {
                    struct item *equipment = actor->equipment[i];
                    if (equipment)
                    {
                        TCOD_list_push(actor->items, equipment);
                        actor->equipment[i] = NULL;
                    }
                }
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
        actor_ai(actor);
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
    int size = vsnprintf(NULL, 0, fmt, args);
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
