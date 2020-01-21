#include "world.h"

#include <assert.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "actor.h"
#include "message.h"
#include "object.h"
#include "projectile.h"
#include "room.h"
#include "util.h"

// TODO: there is a lot of repetition of things with light properties
// pack them into a light struct?

// TODO: resurrect corpses

// TODO: optimize
// pathfinding takes a while

// TODO: traps
// traps should be invisible (unless the actor can see them though skills or magic)
// different trap types

// TOOD: chests
// should chests just give items and disappear?
// or maybe they should be containers that actors can store items in

// TODO: wands and staves?

// TODO: sound and sound propogation
// play "sounds" in the game such as footsteps and monster noises
// calculate whether the player can hear and identify the sound and put it in the log if so
// sound "reflection"?

// TODO: redo map generation
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

struct world *world;

void world_init(void)
{
    world = malloc(sizeof(struct world));
    assert(world);
    world->state = WORLD_STATE_PLAY;
    world->seed = 0;
    world->random = NULL;
    world->turn = 0;
    for (int floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *map = &world->maps[floor];
        map_init(map, floor);
    }
    world->messages = TCOD_list_new();
    world->player = NULL;
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
    TCOD_random_delete(world->random);
    free(world);
    world = NULL;
}

// TODO: this should accept an actor which will become the player, presumably passed from a character creation menu
void world_create(void)
{
    world->seed = (unsigned int)time(0);
    world->random = TCOD_random_new_from_seed(TCOD_RNG_MT, world->seed);
    printf("World created with seed %d.\n", world->seed);

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
            world->player = player;
            TCOD_list_push(map->actors, player);
            struct tile *tile = &map->tiles[x][y];
            tile->actor = player;
            struct item *bodkin_arrow = item_new(ITEM_TYPE_BODKIN_ARROW, floor, x, y, 50);
            TCOD_list_push(player->items, bodkin_arrow);
            struct item *bolt = item_new(ITEM_TYPE_BOLT, floor, x, y, 50);
            TCOD_list_push(player->items, bolt);
            struct item *crossbow = item_new(ITEM_TYPE_CROSSBOW, floor, x, y, 1);
            TCOD_list_push(player->items, crossbow);
            struct item *iron_armor = item_new(ITEM_TYPE_IRON_ARMOR, floor, x, y, 1);
            TCOD_list_push(player->items, iron_armor);
            struct item *greatsword = item_new(ITEM_TYPE_GREATSWORD, floor, x, y, 1);
            TCOD_list_push(player->items, greatsword);
            struct item *longsword = item_new(ITEM_TYPE_LONGSWORD, floor, x, y, 1);
            TCOD_list_push(player->items, longsword);
            struct item *longbow = item_new(ITEM_TYPE_LONGBOW, floor, x, y, 1);
            TCOD_list_push(player->items, longbow);
            struct item *kite_shield = item_new(ITEM_TYPE_KITE_SHIELD, floor, x, y, 1);
            TCOD_list_push(player->items, kite_shield);
            struct item *healing_potion = item_new(ITEM_TYPE_HEALING_POTION, floor, x, y, 10);
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
            TCOD_list_push(map->actors, pet);
            struct tile *tile = &map->tiles[x][y];
            tile->actor = pet;
        }
    }

    struct map *map = &world->maps[world->player->floor];
    TCOD_LIST_FOREACH(map->objects)
    {
        struct object *object = *iterator;
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
}

void world_save(const char *filename)
{
    TCOD_zip_t zip = TCOD_zip_new();
    TCOD_zip_put_int(zip, world->state);
    TCOD_zip_put_int(zip, world->seed);
    // TODO: save random state
    TCOD_zip_put_int(zip, world->turn);
    int player_map = -1;
    int player_index = -1;
    for (int floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *map = &world->maps[floor];
        TCOD_zip_put_int(zip, map->stair_down_x);
        TCOD_zip_put_int(zip, map->stair_down_y);
        TCOD_zip_put_int(zip, map->stair_up_x);
        TCOD_zip_put_int(zip, map->stair_up_y);
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                struct tile *tile = &map->tiles[x][y];
                TCOD_zip_put_int(zip, tile->type);
                TCOD_zip_put_int(zip, tile->seen);
            }
        }
        TCOD_zip_put_int(zip, TCOD_list_size(map->rooms));
        TCOD_LIST_FOREACH(map->rooms)
        {
            struct room *room = *iterator;
            TCOD_zip_put_int(zip, room->x);
            TCOD_zip_put_int(zip, room->y);
            TCOD_zip_put_int(zip, room->w);
            TCOD_zip_put_int(zip, room->h);
        }
        TCOD_zip_put_int(zip, TCOD_list_size(map->objects));
        TCOD_LIST_FOREACH(map->objects)
        {
            struct object *object = *iterator;
            TCOD_zip_put_int(zip, object->type);
            TCOD_zip_put_int(zip, object->x);
            TCOD_zip_put_int(zip, object->y);
            TCOD_zip_put_color(zip, object->color);
            TCOD_zip_put_int(zip, object->light_radius);
            TCOD_zip_put_color(zip, object->light_color);
            TCOD_zip_put_float(zip, object->light_intensity);
            TCOD_zip_put_int(zip, object->light_flicker);
            TCOD_zip_put_int(zip, object->destroyed);
        }
        TCOD_zip_put_int(zip, TCOD_list_size(map->actors));
        int index = 0;
        TCOD_LIST_FOREACH(map->actors)
        {
            struct actor *actor = *iterator;
            TCOD_zip_put_string(zip, actor->name);
            TCOD_zip_put_int(zip, actor->race);
            TCOD_zip_put_int(zip, actor->class);
            TCOD_zip_put_int(zip, actor->faction);
            TCOD_zip_put_int(zip, actor->level);
            TCOD_zip_put_int(zip, actor->experience);
            TCOD_zip_put_int(zip, actor->max_hp);
            TCOD_zip_put_int(zip, actor->current_hp);
            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                struct item *item = actor->equipment[equip_slot];
                if (item)
                {
                    TCOD_zip_put_int(zip, 1);
                    TCOD_zip_put_int(zip, item->type);
                    TCOD_zip_put_int(zip, item->x);
                    TCOD_zip_put_int(zip, item->y);
                    TCOD_zip_put_int(zip, item->current_durability);
                    TCOD_zip_put_int(zip, item->current_stack);
                }
                else
                {
                    TCOD_zip_put_int(zip, 0);
                }
            }
            TCOD_zip_put_int(zip, TCOD_list_size(actor->items));
            TCOD_LIST_FOREACH(actor->items)
            {
                struct item *item = *iterator;
                TCOD_zip_put_int(zip, item->type);
                TCOD_zip_put_int(zip, item->x);
                TCOD_zip_put_int(zip, item->y);
                TCOD_zip_put_int(zip, item->current_durability);
                TCOD_zip_put_int(zip, item->current_stack);
            }
            TCOD_zip_put_int(zip, actor->readied_spell);
            TCOD_zip_put_int(zip, actor->x);
            TCOD_zip_put_int(zip, actor->y);
            TCOD_zip_put_int(zip, actor->last_seen_x);
            TCOD_zip_put_int(zip, actor->last_seen_y);
            TCOD_zip_put_int(zip, actor->turns_chased);
            // TODO: save leader
            TCOD_zip_put_int(zip, actor->light_radius);
            TCOD_zip_put_color(zip, actor->light_color);
            TCOD_zip_put_float(zip, actor->light_intensity);
            TCOD_zip_put_int(zip, actor->light_flicker);
            TCOD_zip_put_color(zip, actor->flash_color);
            TCOD_zip_put_float(zip, actor->flash_fade_coef);
            TCOD_zip_put_int(zip, actor->dead);

            // remember where the player is in the array
            if (actor == world->player)
            {
                player_map = floor;
                player_index = index;
            }
            index++;
        }
        TCOD_zip_put_int(zip, TCOD_list_size(map->corpses));
        TCOD_LIST_FOREACH(map->corpses)
        {
            struct actor *corpse = *iterator;
            TCOD_zip_put_string(zip, corpse->name);
            TCOD_zip_put_int(zip, corpse->race);
            TCOD_zip_put_int(zip, corpse->class);
            TCOD_zip_put_int(zip, corpse->faction);
            TCOD_zip_put_int(zip, corpse->level);
            TCOD_zip_put_int(zip, corpse->experience);
            TCOD_zip_put_int(zip, corpse->max_hp);
            TCOD_zip_put_int(zip, corpse->current_hp);
            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                struct item *item = corpse->equipment[equip_slot];
                if (item)
                {
                    TCOD_zip_put_int(zip, 1);
                    TCOD_zip_put_int(zip, item->type);
                    TCOD_zip_put_int(zip, item->x);
                    TCOD_zip_put_int(zip, item->y);
                    TCOD_zip_put_int(zip, item->current_durability);
                    TCOD_zip_put_int(zip, item->current_stack);
                }
                else
                {
                    TCOD_zip_put_int(zip, 0);
                }
            }
            TCOD_zip_put_int(zip, TCOD_list_size(corpse->items));
            TCOD_LIST_FOREACH(corpse->items)
            {
                struct item *item = *iterator;
                TCOD_zip_put_int(zip, item->type);
                TCOD_zip_put_int(zip, item->x);
                TCOD_zip_put_int(zip, item->y);
                TCOD_zip_put_int(zip, item->current_durability);
                TCOD_zip_put_int(zip, item->current_stack);
            }
            TCOD_zip_put_int(zip, corpse->readied_spell);
            TCOD_zip_put_int(zip, corpse->x);
            TCOD_zip_put_int(zip, corpse->y);
            TCOD_zip_put_int(zip, corpse->last_seen_x);
            TCOD_zip_put_int(zip, corpse->last_seen_y);
            TCOD_zip_put_int(zip, corpse->turns_chased);
            // TODO: save leader
            TCOD_zip_put_int(zip, corpse->light_radius);
            TCOD_zip_put_color(zip, corpse->light_color);
            TCOD_zip_put_float(zip, corpse->light_intensity);
            TCOD_zip_put_int(zip, corpse->light_flicker);
            TCOD_zip_put_color(zip, corpse->flash_color);
            TCOD_zip_put_float(zip, corpse->flash_fade_coef);
            TCOD_zip_put_int(zip, corpse->dead);
        }
        TCOD_zip_put_int(zip, TCOD_list_size(map->items));
        TCOD_LIST_FOREACH(map->items)
        {
            struct item *item = *iterator;
            TCOD_zip_put_int(zip, item->type);
            TCOD_zip_put_int(zip, item->x);
            TCOD_zip_put_int(zip, item->y);
            TCOD_zip_put_int(zip, item->current_durability);
            TCOD_zip_put_int(zip, item->current_stack);
        }
        // TODO: save projectiles?
    }
    TCOD_zip_put_int(zip, player_map);
    TCOD_zip_put_int(zip, player_index);
    TCOD_zip_save_to_file(zip, filename);
    TCOD_zip_delete(zip);
    printf("World saved.\n");
}

void world_load(const char *filename)
{
    TCOD_zip_t zip = TCOD_zip_new();
    TCOD_zip_load_from_file(zip, filename);
    world->state = TCOD_zip_get_int(zip);
    world->seed = TCOD_zip_get_int(zip);
    // TODO: load random state
    world->random = TCOD_random_new_from_seed(TCOD_RNG_MT, world->seed);
    world->turn = TCOD_zip_get_int(zip);
    for (int floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *map = &world->maps[floor];
        map->stair_down_x = TCOD_zip_get_int(zip);
        map->stair_down_y = TCOD_zip_get_int(zip);
        map->stair_up_x = TCOD_zip_get_int(zip);
        map->stair_up_y = TCOD_zip_get_int(zip);
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                struct tile *tile = &map->tiles[x][y];
                tile->type = TCOD_zip_get_int(zip);
                tile->seen = TCOD_zip_get_int(zip);
            }
        }
        int num_rooms = TCOD_zip_get_int(zip);
        for (int i = 0; i < num_rooms; i++)
        {
            int x = TCOD_zip_get_int(zip);
            int y = TCOD_zip_get_int(zip);
            int w = TCOD_zip_get_int(zip);
            int h = TCOD_zip_get_int(zip);
            struct room *room = room_new(x, y, w, h);
            TCOD_list_push(map->rooms, room);
        }
        int num_objects = TCOD_zip_get_int(zip);
        for (int i = 0; i < num_objects; i++)
        {
            enum object_type type = TCOD_zip_get_int(zip);
            int x = TCOD_zip_get_int(zip);
            int y = TCOD_zip_get_int(zip);
            TCOD_color_t color = TCOD_zip_get_color(zip);
            int light_radius = TCOD_zip_get_int(zip);
            TCOD_color_t light_color = TCOD_zip_get_color(zip);
            float light_intensity = TCOD_zip_get_float(zip);
            bool light_flicker = TCOD_zip_get_int(zip);
            bool destroyed = TCOD_zip_get_int(zip);
            struct object *object = object_new(type, floor, x, y, color, light_radius, light_color, light_intensity, light_flicker);
            object->destroyed = destroyed;
            TCOD_list_push(map->objects, object);
            struct tile *tile = &map->tiles[x][y];
            tile->object = object;
        }
        int num_actors = TCOD_zip_get_int(zip);
        for (int i = 0; i < num_actors; i++)
        {
            const char *name = TCOD_zip_get_string(zip);
            int race = TCOD_zip_get_int(zip);
            int class = TCOD_zip_get_int(zip);
            int faction = TCOD_zip_get_int(zip);
            int level = TCOD_zip_get_int(zip);
            int experience = TCOD_zip_get_int(zip);
            int max_hp = TCOD_zip_get_int(zip);
            int current_hp = TCOD_zip_get_int(zip);
            struct item *equipment[NUM_EQUIP_SLOTS];
            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                bool exists = TCOD_zip_get_int(zip);
                if (exists)
                {
                    enum item_type type = TCOD_zip_get_int(zip);
                    int x = TCOD_zip_get_int(zip);
                    int y = TCOD_zip_get_int(zip);
                    int current_durability = TCOD_zip_get_int(zip);
                    int current_stack = TCOD_zip_get_int(zip);
                    struct item *item = item_new(type, floor, x, y, current_stack);
                    item->current_durability = current_durability;
                    equipment[equip_slot] = item;
                }
                else
                {
                    equipment[equip_slot] = NULL;
                }
            }
            TCOD_list_t items = TCOD_list_new();
            int num_items = TCOD_zip_get_int(zip);
            for (int i = 0; i < num_items; i++)
            {
                enum item_type type = TCOD_zip_get_int(zip);
                int x = TCOD_zip_get_int(zip);
                int y = TCOD_zip_get_int(zip);
                int current_durability = TCOD_zip_get_int(zip);
                int current_stack = TCOD_zip_get_int(zip);
                struct item *item = item_new(type, floor, x, y, current_stack);
                item->current_durability = current_durability;
                TCOD_list_push(items, item);
            }
            enum spell_type readied_spell = TCOD_zip_get_int(zip);
            int x = TCOD_zip_get_int(zip);
            int y = TCOD_zip_get_int(zip);
            int last_seen_x = TCOD_zip_get_int(zip);
            int last_seen_y = TCOD_zip_get_int(zip);
            int turns_chased = TCOD_zip_get_int(zip);
            // TODO: load leader
            int light_radius = TCOD_zip_get_int(zip);
            TCOD_color_t light_color = TCOD_zip_get_color(zip);
            float light_intensity = TCOD_zip_get_float(zip);
            int light_flicker = TCOD_zip_get_int(zip);
            TCOD_color_t flash_color = TCOD_zip_get_color(zip);
            float flash_fade_coef = TCOD_zip_get_float(zip);
            bool dead = TCOD_zip_get_int(zip);
            struct actor *actor = actor_new(name, race, class, faction, level, floor, x, y);
            actor->experience = experience;
            actor->max_hp = max_hp;
            actor->current_hp = current_hp;
            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                actor->equipment[equip_slot] = equipment[equip_slot];
            }
            TCOD_list_delete(actor->items);
            actor->items = items;
            actor->readied_spell = readied_spell;
            actor->last_seen_x = last_seen_x;
            actor->last_seen_y = last_seen_y;
            actor->turns_chased = turns_chased;
            actor->light_radius = light_radius;
            actor->light_color = light_color;
            actor->light_intensity = light_intensity;
            actor->light_flicker = light_flicker;
            actor->flash_color = flash_color;
            actor->flash_fade_coef = flash_fade_coef;
            actor->dead = dead;
            TCOD_list_push(map->actors, actor);
            struct tile *tile = &map->tiles[x][y];
            tile->actor = actor;
        }
        int num_corpses = TCOD_zip_get_int(zip);
        for (int i = 0; i < num_corpses; i++)
        {
            const char *name = TCOD_zip_get_string(zip);
            int race = TCOD_zip_get_int(zip);
            int class = TCOD_zip_get_int(zip);
            int faction = TCOD_zip_get_int(zip);
            int level = TCOD_zip_get_int(zip);
            int experience = TCOD_zip_get_int(zip);
            int max_hp = TCOD_zip_get_int(zip);
            int current_hp = TCOD_zip_get_int(zip);
            struct item *equipment[NUM_EQUIP_SLOTS];
            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                bool exists = TCOD_zip_get_int(zip);
                if (exists)
                {
                    enum item_type type = TCOD_zip_get_int(zip);
                    int x = TCOD_zip_get_int(zip);
                    int y = TCOD_zip_get_int(zip);
                    int current_durability = TCOD_zip_get_int(zip);
                    int current_stack = TCOD_zip_get_int(zip);
                    struct item *item = item_new(type, floor, x, y, current_stack);
                    item->current_durability = current_durability;
                    equipment[equip_slot] = item;
                }
                else
                {
                    equipment[equip_slot] = NULL;
                }
            }
            TCOD_list_t items = TCOD_list_new();
            int num_items = TCOD_zip_get_int(zip);
            for (int i = 0; i < num_items; i++)
            {
                enum item_type type = TCOD_zip_get_int(zip);
                int x = TCOD_zip_get_int(zip);
                int y = TCOD_zip_get_int(zip);
                int current_durability = TCOD_zip_get_int(zip);
                int current_stack = TCOD_zip_get_int(zip);
                struct item *item = item_new(type, floor, x, y, current_stack);
                item->current_durability = current_durability;
                TCOD_list_push(items, item);
            }
            enum spell_type readied_spell = TCOD_zip_get_int(zip);
            int x = TCOD_zip_get_int(zip);
            int y = TCOD_zip_get_int(zip);
            int last_seen_x = TCOD_zip_get_int(zip);
            int last_seen_y = TCOD_zip_get_int(zip);
            int turns_chased = TCOD_zip_get_int(zip);
            // TODO: load leader
            int light_radius = TCOD_zip_get_int(zip);
            TCOD_color_t light_color = TCOD_zip_get_color(zip);
            float light_intensity = TCOD_zip_get_float(zip);
            int light_flicker = TCOD_zip_get_int(zip);
            TCOD_color_t flash_color = TCOD_zip_get_color(zip);
            float flash_fade_coef = TCOD_zip_get_float(zip);
            bool dead = TCOD_zip_get_int(zip);
            struct actor *corpse = actor_new(name, race, class, faction, level, floor, x, y);
            corpse->experience = experience;
            corpse->max_hp = max_hp;
            corpse->current_hp = current_hp;
            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                corpse->equipment[equip_slot] = equipment[equip_slot];
            }
            TCOD_list_delete(corpse->items);
            corpse->items = items;
            corpse->readied_spell = readied_spell;
            corpse->last_seen_x = last_seen_x;
            corpse->last_seen_y = last_seen_y;
            corpse->turns_chased = turns_chased;
            corpse->light_radius = light_radius;
            corpse->light_color = light_color;
            corpse->light_intensity = light_intensity;
            corpse->light_flicker = light_flicker;
            corpse->flash_color = flash_color;
            corpse->flash_fade_coef = flash_fade_coef;
            corpse->dead = dead;
            TCOD_list_push(map->corpses, corpse);
            struct tile *tile = &map->tiles[x][y];
            TCOD_list_push(tile->corpses, corpse);
        }
        int num_items = TCOD_zip_get_int(zip);
        for (int i = 0; i < num_items; i++)
        {
            enum item_type type = TCOD_zip_get_int(zip);
            int x = TCOD_zip_get_int(zip);
            int y = TCOD_zip_get_int(zip);
            int current_durability = TCOD_zip_get_int(zip);
            int current_stack = TCOD_zip_get_int(zip);
            struct item *item = item_new(type, floor, x, y, current_stack);
            item->current_durability = current_durability;
            TCOD_list_push(map->items, item);
            struct tile *tile = &map->tiles[x][y];
            TCOD_list_push(tile->items, item);
        }
    }
    int player_map = TCOD_zip_get_int(zip);
    int player_index = TCOD_zip_get_int(zip);
    world->player = TCOD_list_get(world->maps[player_map].actors, player_index);
    TCOD_zip_delete(zip);
    struct map *map = &world->maps[world->player->floor];
    TCOD_LIST_FOREACH(map->objects)
    {
        struct object *object = *iterator;
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
    printf("World loaded with seed %d.\n", world->seed);
}

void world_update(float delta_time)
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
        actor_update_flash(actor, delta_time);
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
        if (!actor->dead)
        {
            actor_calc_fov(actor);
            if (actor != world->player)
            {
                actor_ai(actor);
            }
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
        if (TCOD_list_size(world->messages) == (TCOD_console_get_height(NULL) / 4) - 2) // TODO: hardcoded message_log_rect.height for now because we're gonna throw all this away soon
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
