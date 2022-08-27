#include "world.h"

#include "actor.h"
#include "assets.h"
#include "corpse.h"
#include "explosion.h"
#include "message.h"
#include "object.h"
#include "projectile.h"
#include "room.h"
#include "spell.h"
#include "util.h"
#include <assert.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct world *world;

void world_init(void)
{
    world = malloc(sizeof(*world));
    assert(world);

    world->random = NULL;

    world->time = 0;

    for (enum item_type item_type = 0; item_type < NUM_ITEM_TYPES; item_type++)
    {
        item_data[item_type].spawned = false;
    }
    for (uint8_t floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *map = &world->maps[floor];
        map_init(map, floor);
    }

    world->player = NULL;

    world->hero = NULL;
    world->hero_dead = false;

    world->messages = TCOD_list_new();
}

void world_uninit(void)
{
    TCOD_LIST_FOREACH(world->messages)
    {
        message_delete(*iterator);
    }
    TCOD_list_delete(world->messages);

    for (int i = 0; i < NUM_MAPS; i++)
    {
        struct map *map = &world->maps[i];
        map_uninit(map);
    }

    if (world->random)
    {
        TCOD_random_delete(world->random);
        TCOD_namegen_destroy();
    }

    if (world->hero_dead)
    {
        actor_delete(world->hero);
    }

    free(world);
    world = NULL;
}

void world_create(struct actor *hero)
{
    world->random = TCOD_random_new_from_seed(TCOD_RNG_MT, (unsigned int)time(0));
    TCOD_namegen_parse("data/namegen.cfg", world->random);

    for (uint8_t floor = 0; floor < NUM_MAPS; floor++)
    {
        // map_generate(&world->maps[floor], TCOD_random_get_int(world->random, 0, NUM_MAP_TYPES - 1));
        map_generate(&world->maps[floor], MAP_TYPE_LARGE_DUNGEON);
    }

    world->hero = hero;

    {
        uint8_t floor = 0;
        struct map *map = &world->maps[floor];

        // init player
        {
            hero->floor = floor;
            hero->x = map->stair_up_x;
            hero->y = map->stair_up_y;
            hero->controllable = true;

            TCOD_list_push(map->actors, hero);
            map->tiles[hero->x][hero->y].actor = hero;
        }

        // create pet
        {
            struct actor *pet = actor_new("Spot", RACE_DOG, CLASS_ANIMAL, hero->faction, floor + 1, floor, map->stair_up_x + 1, map->stair_up_y + 1, LIGHT_TYPE_NONE);
            pet->leader = world->hero;

            TCOD_list_push(map->actors, pet);
            map->tiles[pet->x][pet->y].actor = pet;
        }
    }

    world_log(
        -1,
        -1,
        -1,
        TCOD_white,
        "Hail, %s!",
        world->hero->name);

    printf("World created.\n");
}

void world_save(const char *filename)
{
    TCOD_zip_t zip = TCOD_zip_new();

    TCOD_zip_put_random(zip, world->random);
    TCOD_zip_put_int(zip, world->time);

    int player_floor = -1;
    int player_index = -1;
    int hero_floor = -1;
    int hero_index = -1;

    for (uint8_t floor = 0; floor < NUM_MAPS; floor++)
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
            TCOD_zip_put_int(zip, actor->ability_points);
            for (enum ability ability = 0; ability < NUM_ABILITIES; ability++)
            {
                TCOD_zip_put_int(zip, actor->ability_scores[ability]);
            }
            TCOD_zip_put_int(zip, actor->base_hit_points);
            TCOD_zip_put_int(zip, actor->base_mana_points);
            TCOD_zip_put_int(zip, actor->hit_points);
            TCOD_zip_put_int(zip, actor->mana_points);
            TCOD_zip_put_int(zip, actor->gold);
            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                struct item *item = actor->equipment[equip_slot];
                if (item)
                {
                    TCOD_zip_put_int(zip, 1);
                    TCOD_zip_put_int(zip, item->type);
                    TCOD_zip_put_int(zip, item->x);
                    TCOD_zip_put_int(zip, item->y);
                    TCOD_zip_put_int(zip, item->durability);
                    TCOD_zip_put_int(zip, item->stack);
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
                TCOD_zip_put_int(zip, item->durability);
                TCOD_zip_put_int(zip, item->stack);
            }
            TCOD_zip_put_int(zip, TCOD_list_size(actor->known_spell_types));
            TCOD_LIST_FOREACH(actor->known_spell_types)
            {
                enum spell_type spell_type = (size_t)(*iterator);
                TCOD_zip_put_int(zip, spell_type);
            }
            TCOD_zip_put_int(zip, actor->readied_spell_type);
            TCOD_zip_put_int(zip, actor->x);
            TCOD_zip_put_int(zip, actor->y);
            TCOD_zip_put_int(zip, actor->took_turn);
            TCOD_zip_put_float(zip, actor->energy);
            TCOD_zip_put_int(zip, actor->last_seen_x);
            TCOD_zip_put_int(zip, actor->last_seen_y);
            TCOD_zip_put_int(zip, actor->turns_chased);
            TCOD_zip_put_int(zip, actor->light_type);
            TCOD_zip_put_color(zip, actor->flash_color);
            TCOD_zip_put_float(zip, actor->flash_fade_coef);
            TCOD_zip_put_int(zip, actor->controllable);

            if (actor == world->player)
            {
                player_floor = floor;
                player_index = index;
            }

            if (actor == world->hero)
            {
                hero_floor = floor;
                hero_index = index;
            }

            index++;
        }

        TCOD_LIST_FOREACH(map->actors)
        {
            struct actor *actor = *iterator;
            if (actor->leader)
            {
                int leader_index = 0;
                TCOD_LIST_FOREACH(map->actors)
                {
                    struct actor *other = *iterator;
                    if (other == actor->leader)
                    {
                        break;
                    }
                    leader_index++;
                }
                TCOD_zip_put_int(zip, leader_index);
            }
            else
            {
                TCOD_zip_put_int(zip, -1);
            }
        }

        TCOD_zip_put_int(zip, TCOD_list_size(map->corpses));
        TCOD_LIST_FOREACH(map->corpses)
        {
            struct corpse *corpse = *iterator;
            TCOD_zip_put_string(zip, corpse->name);
            TCOD_zip_put_int(zip, corpse->level);
            TCOD_zip_put_int(zip, corpse->x);
            TCOD_zip_put_int(zip, corpse->y);
        }

        TCOD_zip_put_int(zip, TCOD_list_size(map->items));
        TCOD_LIST_FOREACH(map->items)
        {
            struct item *item = *iterator;
            TCOD_zip_put_int(zip, item->type);
            TCOD_zip_put_int(zip, item->x);
            TCOD_zip_put_int(zip, item->y);
            TCOD_zip_put_int(zip, item->durability);
            TCOD_zip_put_int(zip, item->stack);
        }

        TCOD_zip_put_int(zip, TCOD_list_size(map->projectiles));
        TCOD_LIST_FOREACH(map->projectiles)
        {
            struct projectile *projectile = *iterator;
            TCOD_zip_put_int(zip, projectile->type);
            TCOD_zip_put_int(zip, projectile->origin_x);
            TCOD_zip_put_int(zip, projectile->origin_y);
            TCOD_zip_put_int(zip, projectile->target_x);
            TCOD_zip_put_int(zip, projectile->target_x);
            TCOD_zip_put_float(zip, projectile->x);
            TCOD_zip_put_float(zip, projectile->y);
            if (projectile->ammunition)
            {
                TCOD_zip_put_int(zip, 1);
                TCOD_zip_put_int(zip, projectile->ammunition->type);
                TCOD_zip_put_int(zip, projectile->ammunition->x);
                TCOD_zip_put_int(zip, projectile->ammunition->y);
                TCOD_zip_put_int(zip, projectile->ammunition->durability);
                TCOD_zip_put_int(zip, projectile->ammunition->stack);
            }
            else
            {
                TCOD_zip_put_int(zip, 0);
            }
        }

        TCOD_LIST_FOREACH(map->projectiles)
        {
            struct projectile *projectile = *iterator;
            int shooter_index = 0;
            TCOD_LIST_FOREACH(map->actors)
            {
                struct actor *actor = *iterator;
                if (actor == projectile->shooter)
                {
                    break;
                }
                shooter_index++;
            }
            TCOD_zip_put_int(zip, shooter_index);
        }

        TCOD_zip_put_int(zip, TCOD_list_size(map->explosions));
        TCOD_LIST_FOREACH(map->explosions)
        {
            struct explosion *explosion = *iterator;
            TCOD_zip_put_int(zip, explosion->x);
            TCOD_zip_put_int(zip, explosion->y);
            TCOD_zip_put_int(zip, explosion->radius);
            TCOD_zip_put_color(zip, explosion->color);
            TCOD_zip_put_float(zip, explosion->lifetime);
        }

        TCOD_zip_put_int(zip, map->current_actor_index);
    }

    TCOD_zip_put_int(zip, player_floor);
    TCOD_zip_put_int(zip, player_index);
    TCOD_zip_put_int(zip, hero_floor);
    TCOD_zip_put_int(zip, hero_index);

    TCOD_zip_put_int(zip, TCOD_list_size(world->messages));
    TCOD_LIST_FOREACH(world->messages)
    {
        struct message *message = *iterator;
        TCOD_zip_put_string(zip, message->text);
        TCOD_zip_put_color(zip, message->color);
    }

    TCOD_zip_save_to_file(zip, filename);

    TCOD_zip_delete(zip);

    printf("World saved.\n");
}

void world_load(const char *filename)
{
    TCOD_zip_t zip = TCOD_zip_new();
    TCOD_zip_load_from_file(zip, filename);

    world->random = TCOD_zip_get_random(zip);
    TCOD_namegen_parse("data/namegen.cfg", world->random);
    world->time = TCOD_zip_get_int(zip);

    for (uint8_t floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *map = &world->maps[floor];

        map->stair_down_x = (uint8_t)TCOD_zip_get_int(zip);
        map->stair_down_y = (uint8_t)TCOD_zip_get_int(zip);
        map->stair_up_x = (uint8_t)TCOD_zip_get_int(zip);
        map->stair_up_y = (uint8_t)TCOD_zip_get_int(zip);

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
            uint8_t x = (uint8_t)TCOD_zip_get_int(zip);
            uint8_t y = (uint8_t)TCOD_zip_get_int(zip);
            uint8_t w = (uint8_t)TCOD_zip_get_int(zip);
            uint8_t h = (uint8_t)TCOD_zip_get_int(zip);

            struct room *room = room_new(x, y, w, h);

            TCOD_list_push(map->rooms, room);
        }

        int num_objects = TCOD_zip_get_int(zip);
        for (int i = 0; i < num_objects; i++)
        {
            enum object_type type = TCOD_zip_get_int(zip);
            uint8_t x = (uint8_t)TCOD_zip_get_int(zip);
            uint8_t y = (uint8_t)TCOD_zip_get_int(zip);
            TCOD_color_t color = TCOD_zip_get_color(zip);
            int light_radius = TCOD_zip_get_int(zip);
            TCOD_color_t light_color = TCOD_zip_get_color(zip);
            float light_intensity = TCOD_zip_get_float(zip);
            bool light_flicker = TCOD_zip_get_int(zip);

            struct object *object = object_new(type, floor, x, y, color, light_radius, light_color, light_intensity, light_flicker);

            map->tiles[x][y].object = object;
            TCOD_list_push(map->objects, object);
        }

        int num_actors = TCOD_zip_get_int(zip);
        for (int i = 0; i < num_actors; i++)
        {
            const char *name = TCOD_zip_get_string(zip);
            enum race race = TCOD_zip_get_int(zip);
            enum class class = TCOD_zip_get_int(zip);
            enum faction faction = TCOD_zip_get_int(zip);
            uint8_t level = (uint8_t)TCOD_zip_get_int(zip);
            int experience = TCOD_zip_get_int(zip);
            int ability_points = TCOD_zip_get_int(zip);
            int strength = TCOD_zip_get_int(zip);
            int dexterity = TCOD_zip_get_int(zip);
            int constitution = TCOD_zip_get_int(zip);
            int intelligence = TCOD_zip_get_int(zip);
            int base_hit_points = TCOD_zip_get_int(zip);
            int base_mana_points = TCOD_zip_get_int(zip);
            int hit_points = TCOD_zip_get_int(zip);
            int mana_points = TCOD_zip_get_int(zip);
            int gold = TCOD_zip_get_int(zip);
            struct item *equipment[NUM_EQUIP_SLOTS];
            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                bool exists = TCOD_zip_get_int(zip);
                if (exists)
                {
                    enum item_type type = TCOD_zip_get_int(zip);
                    uint8_t x = (uint8_t)TCOD_zip_get_int(zip);
                    uint8_t y = (uint8_t)TCOD_zip_get_int(zip);
                    int current_durability = TCOD_zip_get_int(zip);
                    int current_stack = TCOD_zip_get_int(zip);
                    struct item *item = item_new(type, floor, x, y, current_stack);
                    item->durability = current_durability;
                    equipment[equip_slot] = item;
                }
                else
                {
                    equipment[equip_slot] = NULL;
                }
            }
            TCOD_list_t items = TCOD_list_new();
            int num_items = TCOD_zip_get_int(zip);
            for (int j = 0; j < num_items; j++)
            {
                enum item_type type = TCOD_zip_get_int(zip);
                uint8_t x = (uint8_t)TCOD_zip_get_int(zip);
                uint8_t y = (uint8_t)TCOD_zip_get_int(zip);
                int current_durability = TCOD_zip_get_int(zip);
                int current_stack = TCOD_zip_get_int(zip);

                struct item *item = item_new(type, floor, x, y, current_stack);
                item->durability = current_durability;

                TCOD_list_push(items, item);
            }
            TCOD_list_t known_spell_types = TCOD_list_new();
            int num_known_spell_types = TCOD_zip_get_int(zip);
            for (int j = 0; j < num_known_spell_types; j++)
            {
                enum spell_type spell_type = TCOD_zip_get_int(zip);

                TCOD_list_push(known_spell_types, (void *)(size_t)spell_type);
            }
            enum spell_type readied_spell_type = TCOD_zip_get_int(zip);
            uint8_t x = (uint8_t)TCOD_zip_get_int(zip);
            uint8_t y = (uint8_t)TCOD_zip_get_int(zip);
            bool took_turn = TCOD_zip_get_int(zip);
            float energy = TCOD_zip_get_float(zip);
            int last_seen_x = TCOD_zip_get_int(zip);
            int last_seen_y = TCOD_zip_get_int(zip);
            int turns_chased = TCOD_zip_get_int(zip);
            enum light_type light_type = TCOD_zip_get_int(zip);
            TCOD_color_t flash_color = TCOD_zip_get_color(zip);
            float flash_fade_coef = TCOD_zip_get_float(zip);
            bool controllable = TCOD_zip_get_int(zip);

            struct actor *actor = actor_new(name, race, class, faction, level, floor, x, y, light_type);
            actor->experience = experience;
            actor->ability_points = ability_points;
            actor->ability_scores[ABILITY_STRENGTH] = strength;
            actor->ability_scores[ABILITY_DEXTERITY] = dexterity;
            actor->ability_scores[ABILITY_CONSTITUTION] = constitution;
            actor->ability_scores[ABILITY_INTELLIGENCE] = intelligence;
            actor->base_hit_points = base_hit_points;
            actor->base_mana_points = base_mana_points;
            actor->hit_points = hit_points;
            actor->mana_points = mana_points;
            actor->gold = gold;
            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                actor->equipment[equip_slot] = equipment[equip_slot];
            }
            TCOD_list_delete(actor->items);
            actor->items = items;
            TCOD_list_delete(actor->known_spell_types);
            actor->known_spell_types = known_spell_types;
            actor->readied_spell_type = readied_spell_type;
            actor->took_turn = took_turn;
            actor->energy = energy;
            actor->last_seen_x = last_seen_x;
            actor->last_seen_y = last_seen_y;
            actor->turns_chased = turns_chased;
            actor->flash_color = flash_color;
            actor->flash_fade_coef = flash_fade_coef;
            actor->controllable = controllable;

            map->tiles[x][y].actor = actor;
            TCOD_list_push(map->actors, actor);
        }

        TCOD_LIST_FOREACH(map->actors)
        {
            struct actor *actor = *iterator;

            int leader_index = TCOD_zip_get_int(zip);
            if (leader_index > -1)
            {
                actor->leader = TCOD_list_get(map->actors, leader_index);
            }
        }

        int num_corpses = TCOD_zip_get_int(zip);
        for (int i = 0; i < num_corpses; i++)
        {
            const char *name = TCOD_zip_get_string(zip);
            uint8_t level = (uint8_t)TCOD_zip_get_int(zip);
            uint8_t x = (uint8_t)TCOD_zip_get_int(zip);
            uint8_t y = (uint8_t)TCOD_zip_get_int(zip);

            struct corpse *corpse = corpse_new(name, level, floor, x, y);

            TCOD_list_push(map->tiles[x][y].corpses, corpse);
            TCOD_list_push(map->corpses, corpse);
        }

        int num_items = TCOD_zip_get_int(zip);
        for (int i = 0; i < num_items; i++)
        {
            enum item_type type = TCOD_zip_get_int(zip);
            uint8_t x = (uint8_t)TCOD_zip_get_int(zip);
            uint8_t y = (uint8_t)TCOD_zip_get_int(zip);
            int current_durability = TCOD_zip_get_int(zip);
            int current_stack = TCOD_zip_get_int(zip);

            struct item *item = item_new(type, floor, x, y, current_stack);
            item->durability = current_durability;

            TCOD_list_push(map->tiles[x][y].items, item);
            TCOD_list_push(map->items, item);
        }

        int num_projectiles = TCOD_zip_get_int(zip);
        for (int i = 0; i < num_projectiles; i++)
        {
            enum projectile_type type = TCOD_zip_get_int(zip);
            int origin_x = TCOD_zip_get_int(zip);
            int origin_y = TCOD_zip_get_int(zip);
            int target_x = TCOD_zip_get_int(zip);
            int target_y = TCOD_zip_get_int(zip);
            float x = TCOD_zip_get_float(zip);
            float y = TCOD_zip_get_float(zip);
            bool ammunition_exists = TCOD_zip_get_int(zip);
            struct item *ammunition = NULL;
            if (ammunition_exists)
            {
                enum item_type ammunition_type = TCOD_zip_get_int(zip);
                uint8_t ammunition_x = (uint8_t)TCOD_zip_get_int(zip);
                uint8_t ammunition_y = (uint8_t)TCOD_zip_get_int(zip);
                int ammunition_current_durability = TCOD_zip_get_int(zip);
                int ammunition_current_stack = TCOD_zip_get_int(zip);

                ammunition = item_new(ammunition_type, floor, ammunition_x, ammunition_y, ammunition_current_stack);
                ammunition->durability = ammunition_current_durability;
            }

            struct projectile *projectile = projectile_new(type, floor, origin_x, origin_y, target_x, target_y, NULL, ammunition);
            projectile->x = x;
            projectile->y = y;

            TCOD_list_push(map->projectiles, projectile);
        }

        TCOD_LIST_FOREACH(map->projectiles)
        {
            struct projectile *projectile = *iterator;

            int shooter_index = TCOD_zip_get_int(zip);
            projectile->shooter = TCOD_list_get(map->actors, shooter_index);
            if (projectile->type == PROJECTILE_TYPE_ARROW)
            {
                projectile->ammunition = projectile->shooter->equipment[EQUIP_SLOT_AMMUNITION];
            }
        }

        int num_explosions = TCOD_zip_get_int(zip);
        for (int i = 0; i < num_explosions; i++)
        {
            uint8_t x = (uint8_t)TCOD_zip_get_int(zip);
            uint8_t y = (uint8_t)TCOD_zip_get_int(zip);
            int radius = TCOD_zip_get_int(zip);
            TCOD_color_t color = TCOD_zip_get_color(zip);
            float lifetime = TCOD_zip_get_float(zip);

            struct explosion *explosion = explosion_new(floor, x, y, radius, color, NULL);
            explosion->lifetime = lifetime;

            TCOD_list_push(map->explosions, explosion);
        }

        map->current_actor_index = TCOD_zip_get_int(zip);
    }

    int player_floor = TCOD_zip_get_int(zip);
    int player_index = TCOD_zip_get_int(zip);
    world->player = TCOD_list_get(world->maps[player_floor].actors, player_index);

    int hero_floor = TCOD_zip_get_int(zip);
    int hero_index = TCOD_zip_get_int(zip);
    world->hero = TCOD_list_get(world->maps[hero_floor].actors, hero_index);

    int num_messages = TCOD_zip_get_int(zip);
    for (int i = 0; i < num_messages; i++)
    {
        const char *text = TCOD_zip_get_string(zip);
        TCOD_color_t color = TCOD_zip_get_color(zip);

        struct message *message = message_new(text, color);

        TCOD_list_push(world->messages, message);
    }

    TCOD_zip_delete(zip);

    world_log(
        -1,
        -1,
        -1,
        TCOD_white,
        "Welcome back, %s!",
        world->hero->name);

    printf("World loaded.\n");
}

void world_update(float delta_time)
{
    struct map *map = &world->maps[world->hero->floor];

    // update things that should update every frame, regardless of whether a turn has passed
    TCOD_LIST_FOREACH(map->objects)
    {
        struct object *object = *iterator;
        object_calc_light(object);
    }
    TCOD_LIST_FOREACH(map->actors)
    {
        struct actor *actor = *iterator;
        actor_calc_light(actor);
        actor_calc_fade(actor, delta_time);
    }
    TCOD_LIST_FOREACH(map->projectiles)
    {
        struct projectile *projectile = *iterator;
        if (projectile_move(projectile, delta_time))
        {
            projectile_calc_light(projectile);
        }
        else
        {
            iterator = TCOD_list_remove_iterator_fast(map->projectiles, iterator);
            projectile_delete(projectile);

            if (!iterator)
            {
                break;
            }
        }
    }
    TCOD_LIST_FOREACH(map->explosions)
    {
        struct explosion *explosion = *iterator;
        if (!explosion_update(explosion, delta_time))
        {
            iterator = TCOD_list_remove_iterator_fast(map->explosions, iterator);
            explosion_delete(explosion);

            if (!iterator)
            {
                break;
            }
        }
    }

    // calculate player fov every frame because of moving lights (like projectiles) that can reveal things in between turns
    actor_calc_fov(world->player);

    // process actor turns as long no animations are playing
    while (TCOD_list_size(map->projectiles) == 0 &&
           TCOD_list_size(map->explosions) == 0)
    {
        // update things that should be updated per-turn
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

        // check if the last actor in the map has been reached
        if (map->current_actor_index >= TCOD_list_size(map->actors))
        {
            // move the pointer to the first actor
            map->current_actor_index = 0;

            // update world state
            world->time++;

            // reset all actor turns
            // if there are no controllable actors, return control back to the UI so the current state will be rendered
            bool controllable_exists = false;
            TCOD_LIST_FOREACH(map->actors)
            {
                struct actor *const actor = *iterator;

                actor_restore_mana_points(actor, 1);
                actor->took_turn = false;
                actor->energy += actor_calc_speed(actor);

                if (actor->controllable)
                {
                    controllable_exists = true;
                }

                if (actor->dead)
                {
                    // remove from map
                    struct map *const map = &world->maps[actor->floor];
                    iterator = TCOD_list_remove_iterator(map->actors, iterator);

                    // remove from tile
                    struct tile *const tile = &map->tiles[actor->x][actor->y];
                    tile->actor = NULL;

                    // create a corpse
                    struct corpse *const corpse = corpse_new(
                        actor->name,
                        actor->level,
                        actor->floor,
                        actor->x,
                        actor->y);
                    TCOD_list_push(map->corpses, corpse);
                    TCOD_list_push(tile->corpses, corpse);

                    // drop items
                    if (actor != world->hero)
                    {
                        // move equipment to ground
                        for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
                        {
                            struct item *const equipment = actor->equipment[i];
                            if (equipment)
                            {
                                equipment->floor = actor->floor;
                                equipment->x = actor->x;
                                equipment->y = actor->y;

                                TCOD_list_push(tile->items, equipment);
                                TCOD_list_push(map->items, equipment);

                                actor->equipment[i] = NULL;
                            }
                        }

                        // move inventory to ground
                        TCOD_LIST_FOREACH(actor->items)
                        {
                            struct item *const item = *iterator;
                            item->floor = actor->floor;
                            item->x = actor->x;
                            item->y = actor->y;

                            TCOD_list_push(tile->items, item);
                            TCOD_list_push(map->items, item);

                            iterator = TCOD_list_remove_iterator_fast(actor->items, iterator);

                            if (!iterator)
                            {
                                break;
                            }
                        }
                    }

                    if (actor == world->hero)
                    {
                        world->hero_dead = true;

                        world_log(
                            actor->floor,
                            actor->x,
                            actor->y,
                            TCOD_green,
                            "Game over! Press 'ESC' to return to the menu.");
                    }
                    else
                    {
                        actor_delete(actor);
                    }
                }

                if (!iterator)
                {
                    break;
                }
            }
            if (!controllable_exists)
            {
                break;
            }
        }

        // get the current actor and figure out their turn(s), as long as they have energy
        struct actor *const actor = TCOD_list_get(map->actors, map->current_actor_index);
        if (actor_can_take_turn(actor))
        {
            // have we reached an actor that requires player input?
            if (actor->controllable)
            {
                // if so, this is the actor that the UI should affect
                world->player = actor;
            }
            else
            {
                // if not, then run the actor's AI

                // slow down the AI if the hero is dead
                if (world->hero_dead)
                {
                    static float timer = 0.0f;
                    timer += delta_time;
                    if (timer < 0.1f)
                    {
                        break;
                    }
                    timer = 0.0f;
                }

                actor->took_turn = actor_ai(actor);
            }

            // for a controllable actor, the UI is responsible for setting took_turn to true
            // for non-controllable actors, the AI function will do it
            if (actor->took_turn)
            {
                // decrease energy
                actor->energy -= 1.0f;

                // check if the actor can still take a turn
                if (actor_can_take_turn(actor))
                {
                    // reset took_turn status
                    actor->took_turn = false;

                    // return control back to the UI to render the current state
                    // but do not move current_actor_index to the next actor, since it is still this actor's turn
                    break;
                }
            }
            else
            {
                // return control to the UI so the frame can be rendered
                // controllable actors will reach this part if they did not make a move this frame
                // non-controllable actors will reach here if for some reason their AI has been deferred to another frame
                break;
            }
        }

        // move the pointer to the next actor, now that the current one has been resolved
        map->current_actor_index++;
    }
}

bool world_player_can_take_turn(void)
{
    if (world->hero_dead)
    {
        return false;
    }

    const struct map *const map = &world->maps[world->player->floor];
    if (world->player != TCOD_list_get(map->actors, map->current_actor_index))
    {
        return false;
    }

    if (TCOD_list_size(map->projectiles) > 0)
    {
        return false;
    }

    if (TCOD_list_size(map->explosions) > 0)
    {
        return false;
    }

    return true;
}

void world_log(int floor, int x, int y, TCOD_color_t color, char *fmt, ...)
{
    if (floor != -1 &&
        (!world->player ||
         floor != world->player->floor ||
         !world->player->fov ||
         !TCOD_map_is_in_fov(world->player->fov, x, y)))
    {
        return;
    }

    va_list args;
    va_start(args, fmt);

    size_t size = vsnprintf(NULL, 0, fmt, args);
    char *string = malloc(size + 1);
    assert(string);
    vsprintf_s(string, size + 1, fmt, args);

    va_end(args);

    char *line_begin = string;
    char *line_end;
    do
    {
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

    free(string);
}
