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
        item_database[item_type].spawned = false;
    }
    for (uint8_t floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *map = &world->maps[floor];
        map_init(map, floor);
    }

    world->player = NULL;

    world->hero = NULL;
    world->hero_dead = false;

    world->messages = list_new();
}

void world_uninit(void)
{
    for (size_t i = 0; i < world->messages->size; i++)
    {
        message_delete(list_get(world->messages, i));
    }
    list_delete(world->messages);

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

            list_add(map->actors, hero);

            map->tiles[hero->x][hero->y].actor = hero;
        }

        // create pet
        {
            struct actor *pet = actor_new(
                "Spot",
                RACE_DOG,
                CLASS_DOG,
                hero->faction,
                floor,
                map->stair_up_x + 1,
                map->stair_up_y + 1);
            pet->leader = world->hero;

            list_add(map->actors, pet);

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

    uint8_t player_floor = 0;
    size_t player_index = 0;
    uint8_t hero_floor = 0;
    size_t hero_index = 0;

    for (uint8_t floor = 0; floor < NUM_MAPS; floor++)
    {
        const struct map *const map = &world->maps[floor];

        TCOD_zip_put_int(zip, map->stair_down_x);
        TCOD_zip_put_int(zip, map->stair_down_y);
        TCOD_zip_put_int(zip, map->stair_up_x);
        TCOD_zip_put_int(zip, map->stair_up_y);

        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                const struct tile *const tile = &map->tiles[x][y];

                TCOD_zip_put_int(zip, tile->type);
                TCOD_zip_put_int(zip, tile->explored);
            }
        }

        TCOD_zip_put_int(zip, (int)map->rooms->size);
        for (size_t room_index = 0; room_index < map->rooms->size; room_index++)
        {
            const struct room *const room = list_get(map->rooms, room_index);

            TCOD_zip_put_int(zip, room->x);
            TCOD_zip_put_int(zip, room->y);
            TCOD_zip_put_int(zip, room->w);
            TCOD_zip_put_int(zip, room->h);
        }

        TCOD_zip_put_int(zip, (int)map->objects->size);
        for (size_t object_index = 0; object_index < map->objects->size; object_index++)
        {
            const struct object *const object = list_get(map->objects, object_index);

            TCOD_zip_put_int(zip, object->type);

            TCOD_zip_put_int(zip, object->x);
            TCOD_zip_put_int(zip, object->y);
        }

        TCOD_zip_put_int(zip, (int)map->actors->size);
        for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
        {
            const struct actor *const actor = list_get(map->actors, actor_index);

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
                const struct item *const item = actor->equipment[equip_slot];

                if (item)
                {
                    TCOD_zip_put_int(zip, 1);

                    TCOD_zip_put_int(zip, item->type);

                    TCOD_zip_put_int(zip, item->x);
                    TCOD_zip_put_int(zip, item->y);

                    TCOD_zip_put_int(zip, item->stack);

                    TCOD_zip_put_int(zip, item->durability);
                }
                else
                {
                    TCOD_zip_put_int(zip, 0);
                }
            }

            TCOD_zip_put_int(zip, (int)actor->items->size);
            for (size_t item_index = 0; item_index < actor->items->size; item_index++)
            {
                const struct item *const item = list_get(map->items, item_index);

                TCOD_zip_put_int(zip, item->type);

                TCOD_zip_put_int(zip, item->x);
                TCOD_zip_put_int(zip, item->y);

                TCOD_zip_put_int(zip, item->stack);

                TCOD_zip_put_int(zip, item->durability);
            }

            TCOD_zip_put_int(zip, (int)actor->known_spell_types->size);
            for (size_t known_spell_type_index = 0; known_spell_type_index < actor->known_spell_types->size; known_spell_type_index++)
            {
                const enum spell_type spell_type = (size_t)(list_get(actor->known_spell_types, known_spell_type_index));

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
                player_index = actor_index;
            }

            if (actor == world->hero)
            {
                hero_floor = floor;
                hero_index = actor_index;
            }
        }

        for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
        {
            const struct actor *const actor = list_get(map->actors, actor_index);

            if (actor->leader)
            {
                size_t leader_index = map->actors->size;

                for (size_t other_actor_index = 0; other_actor_index < map->actors->size; other_actor_index++)
                {
                    const struct actor *const other = list_get(map->actors, other_actor_index);

                    if (other == actor->leader)
                    {
                        leader_index = other_actor_index;

                        break;
                    }
                }

                TCOD_zip_put_int(zip, (int)leader_index);
            }
            else
            {
                TCOD_zip_put_int(zip, -1);
            }
        }

        TCOD_zip_put_int(zip, (int)map->corpses->size);
        for (size_t corpse_index = 0; corpse_index < map->corpses->size; corpse_index++)
        {
            const struct corpse *const corpse = list_get(map->corpses, corpse_index);

            TCOD_zip_put_string(zip, corpse->name);
            TCOD_zip_put_int(zip, corpse->level);
            TCOD_zip_put_int(zip, corpse->x);
            TCOD_zip_put_int(zip, corpse->y);
        }

        TCOD_zip_put_int(zip, (int)map->items->size);
        for (size_t item_index = 0; item_index < map->items->size; item_index++)
        {
            const struct item *const item = list_get(map->items, item_index);

            TCOD_zip_put_int(zip, item->type);
            TCOD_zip_put_int(zip, item->x);
            TCOD_zip_put_int(zip, item->y);
            TCOD_zip_put_int(zip, item->stack);
            TCOD_zip_put_int(zip, item->durability);
        }

        TCOD_zip_put_int(zip, (int)map->projectiles->size);
        for (size_t projectile_index = 0; projectile_index < map->projectiles->size; projectile_index++)
        {
            const struct projectile *const projectile = list_get(map->projectiles, projectile_index);

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

        for (size_t projectile_index = 0; projectile_index < map->projectiles->size; projectile_index++)
        {
            const struct projectile *const projectile = list_get(map->projectiles, projectile_index);

            size_t shooter_index = map->actors->size;

            for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
            {
                const struct actor *const actor = list_get(map->actors, actor_index);

                if (actor == projectile->shooter)
                {
                    shooter_index = actor_index;

                    break;
                }
            }

            TCOD_zip_put_int(zip, (int)shooter_index);
        }

        TCOD_zip_put_int(zip, (int)map->explosions->size);
        for (size_t explosion_index = 0; explosion_index < map->explosions->size; explosion_index++)
        {
            const struct explosion *const explosion = list_get(map->explosions, explosion_index);

            TCOD_zip_put_int(zip, explosion->x);
            TCOD_zip_put_int(zip, explosion->y);
            TCOD_zip_put_int(zip, explosion->radius);
            TCOD_zip_put_color(zip, explosion->color);
            TCOD_zip_put_float(zip, explosion->lifetime);
        }

        TCOD_zip_put_int(zip, map->current_actor_index);
    }

    TCOD_zip_put_int(zip, player_floor);
    TCOD_zip_put_int(zip, (int)player_index);
    TCOD_zip_put_int(zip, hero_floor);
    TCOD_zip_put_int(zip, (int)hero_index);

    TCOD_zip_put_int(zip, (int)world->messages->size);
    for (size_t message_index = 0; message_index < world->messages->size; message_index++)
    {
        const struct message *const message = list_get(world->messages, message_index);

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
        struct map *const map = &world->maps[floor];

        map->stair_down_x = (uint8_t)TCOD_zip_get_int(zip);
        map->stair_down_y = (uint8_t)TCOD_zip_get_int(zip);
        map->stair_up_x = (uint8_t)TCOD_zip_get_int(zip);
        map->stair_up_y = (uint8_t)TCOD_zip_get_int(zip);

        for (size_t x = 0; x < MAP_WIDTH; x++)
        {
            for (size_t y = 0; y < MAP_HEIGHT; y++)
            {
                struct tile *const tile = &map->tiles[x][y];

                tile->type = TCOD_zip_get_int(zip);
                tile->explored = TCOD_zip_get_int(zip);
            }
        }

        const size_t num_rooms = TCOD_zip_get_int(zip);
        for (size_t room_index = 0; room_index < num_rooms; room_index++)
        {
            struct room *const room = malloc(sizeof(struct room));
            assert(room);

            room->x = (uint8_t)TCOD_zip_get_int(zip);
            room->y = (uint8_t)TCOD_zip_get_int(zip);
            room->w = (uint8_t)TCOD_zip_get_int(zip);
            room->h = (uint8_t)TCOD_zip_get_int(zip);

            list_add(map->rooms, room);
        }

        const size_t num_objects = TCOD_zip_get_int(zip);
        for (size_t i = 0; i < num_objects; i++)
        {
            struct object *const object = malloc(sizeof(struct object));
            assert(object);

            object->type = TCOD_zip_get_int(zip);

            object->floor = floor;
            object->x = (uint8_t)TCOD_zip_get_int(zip);
            object->y = (uint8_t)TCOD_zip_get_int(zip);

            object->light_fov = NULL;

            list_add(map->objects, object);
            map->tiles[object->x][object->y].object = object;
        }

        const size_t num_actors = TCOD_zip_get_int(zip);
        for (size_t actor_index = 0; actor_index < num_actors; actor_index++)
        {
            struct actor *const actor = malloc(sizeof(struct actor));
            assert(actor);

            actor->name = TCOD_strdup(TCOD_zip_get_string(zip));
            actor->race = TCOD_zip_get_int(zip);
            actor->class = TCOD_zip_get_int(zip);
            actor->faction = TCOD_zip_get_int(zip);

            actor->level = (uint8_t)TCOD_zip_get_int(zip);
            actor->experience = TCOD_zip_get_int(zip);
            actor->ability_points = TCOD_zip_get_int(zip);

            for (enum ability ability_score = 0; ability_score < NUM_ABILITIES; ability_score++)
            {
                actor->ability_scores[ability_score] = TCOD_zip_get_int(zip);
            }

            actor->base_hit_points = TCOD_zip_get_int(zip);
            actor->base_mana_points = TCOD_zip_get_int(zip);

            actor->hit_points = TCOD_zip_get_int(zip);
            actor->mana_points = TCOD_zip_get_int(zip);

            actor->gold = TCOD_zip_get_int(zip);

            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                const bool exists = TCOD_zip_get_int(zip);

                if (exists)
                {
                    struct item *const item = malloc(sizeof(*item));
                    assert(item);

                    item->type = TCOD_zip_get_int(zip);

                    item->floor = floor;
                    item->x = (uint8_t)TCOD_zip_get_int(zip);
                    item->y = (uint8_t)TCOD_zip_get_int(zip);

                    item->stack = TCOD_zip_get_int(zip);

                    item->durability = TCOD_zip_get_int(zip);

                    actor->equipment[equip_slot] = item;
                }
                else
                {
                    actor->equipment[equip_slot] = NULL;
                }
            }

            actor->items = list_new();
            const size_t num_items = TCOD_zip_get_int(zip);
            for (size_t item_index = 0; item_index < num_items; item_index++)
            {
                struct item *const item = malloc(sizeof(struct item));
                assert(item);

                item->type = TCOD_zip_get_int(zip);

                item->x = (uint8_t)TCOD_zip_get_int(zip);
                item->y = (uint8_t)TCOD_zip_get_int(zip);

                item->stack = TCOD_zip_get_int(zip);

                item->durability = TCOD_zip_get_int(zip);

                list_add(actor->items, item);
            }

            actor->known_spell_types = list_new();
            const size_t num_known_spell_types = TCOD_zip_get_int(zip);
            for (size_t known_spell_type_index = 0; known_spell_type_index < num_known_spell_types; known_spell_type_index++)
            {
                const enum spell_type spell_type = TCOD_zip_get_int(zip);

                list_add(actor->known_spell_types, (void *)(size_t)spell_type);
            }

            actor->readied_spell_type = TCOD_zip_get_int(zip);

            actor->floor = floor;
            actor->x = (uint8_t)TCOD_zip_get_int(zip);
            actor->y = (uint8_t)TCOD_zip_get_int(zip);

            actor->fov = NULL;

            actor->took_turn = TCOD_zip_get_int(zip);
            actor->energy = TCOD_zip_get_float(zip);

            actor->last_seen_x = TCOD_zip_get_int(zip);
            actor->last_seen_y = TCOD_zip_get_int(zip);
            actor->turns_chased = TCOD_zip_get_int(zip);

            actor->leader = NULL;

            actor->light_type = TCOD_zip_get_int(zip);
            actor->light_fov = NULL;

            actor->flash_color = TCOD_zip_get_color(zip);
            actor->flash_fade_coef = TCOD_zip_get_float(zip);

            actor->controllable = TCOD_zip_get_int(zip);

            actor->dead = false;

            list_add(map->actors, actor);
            map->tiles[actor->x][actor->y].actor = actor;
        }

        for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
        {
            struct actor *const actor = list_get(map->actors, actor_index);

            const size_t leader_index = TCOD_zip_get_int(zip);

            if (leader_index < map->actors->size)
            {
                actor->leader = list_get(map->actors, leader_index);
            }
        }

        const size_t num_corpses = TCOD_zip_get_int(zip);
        for (size_t corpse_index = 0; corpse_index < num_corpses; corpse_index++)
        {
            struct corpse *corpse = malloc(sizeof(*corpse));
            assert(corpse);

            corpse->name = TCOD_strdup(TCOD_zip_get_string(zip));
            corpse->level = (uint8_t)TCOD_zip_get_int(zip);

            corpse->floor = floor;
            corpse->x = (uint8_t)TCOD_zip_get_int(zip);
            corpse->y = (uint8_t)TCOD_zip_get_int(zip);

            list_add(map->corpses, corpse);
            list_add(map->tiles[corpse->x][corpse->y].corpses, corpse);
        }

        const size_t num_items = TCOD_zip_get_int(zip);
        for (size_t i = 0; i < num_items; i++)
        {
            struct item *const item = malloc(sizeof(struct item));
            assert(item);

            item->type = TCOD_zip_get_int(zip);

            item->x = (uint8_t)TCOD_zip_get_int(zip);
            item->y = (uint8_t)TCOD_zip_get_int(zip);

            item->stack = TCOD_zip_get_int(zip);

            item->durability = TCOD_zip_get_int(zip);

            list_add(map->items, item);
            list_add(map->tiles[item->x][item->y].items, item);
        }

        const size_t num_projectiles = TCOD_zip_get_int(zip);
        for (size_t projectile_index = 0; projectile_index < num_projectiles; projectile_index++)
        {
            struct projectile *const projectile = malloc(sizeof(struct projectile));
            assert(projectile);

            projectile->type = TCOD_zip_get_int(zip);

            projectile->floor = floor;
            projectile->origin_x = TCOD_zip_get_int(zip);
            projectile->origin_y = TCOD_zip_get_int(zip);
            projectile->target_x = TCOD_zip_get_int(zip);
            projectile->target_y = TCOD_zip_get_int(zip);
            projectile->x = TCOD_zip_get_float(zip);
            projectile->y = TCOD_zip_get_float(zip);

            projectile->shooter = NULL;

            bool ammunition_exists = TCOD_zip_get_int(zip);
            if (ammunition_exists)
            {
                struct item *const item = malloc(sizeof(struct item));
                assert(item);

                item->type = TCOD_zip_get_int(zip);

                item->x = (uint8_t)TCOD_zip_get_int(zip);
                item->y = (uint8_t)TCOD_zip_get_int(zip);

                item->stack = TCOD_zip_get_int(zip);

                item->durability = TCOD_zip_get_int(zip);

                projectile->ammunition = item;
            }
            else
            {
                projectile->ammunition = NULL;
            }

            projectile->light_fov = NULL;

            list_add(map->projectiles, projectile);
        }

        for (size_t projectile_index = 0; projectile_index < map->projectiles->size; projectile_index++)
        {
            struct projectile *const projectile = list_get(map->projectiles, projectile_index);

            const size_t shooter_index = TCOD_zip_get_int(zip);

            projectile->shooter = list_get(map->actors, shooter_index);

            if (projectile->type == PROJECTILE_TYPE_ARROW)
            {
                projectile->ammunition = projectile->shooter->equipment[EQUIP_SLOT_AMMUNITION];
            }
        }

        const size_t num_explosions = TCOD_zip_get_int(zip);
        for (size_t explosion_index = 0; explosion_index < num_explosions; explosion_index++)
        {
            struct explosion *const explosion = malloc(sizeof(struct explosion));
            assert(explosion);

            explosion->floor = floor;
            explosion->x = (uint8_t)TCOD_zip_get_int(zip);
            explosion->y = (uint8_t)TCOD_zip_get_int(zip);

            explosion->radius = TCOD_zip_get_int(zip);
            explosion->color = TCOD_zip_get_color(zip);
            explosion->lifetime = TCOD_zip_get_float(zip);

            list_add(map->explosions, explosion);
        }

        map->current_actor_index = TCOD_zip_get_int(zip);
    }

    const size_t player_floor = TCOD_zip_get_int(zip);
    const size_t player_index = TCOD_zip_get_int(zip);
    world->player = list_get(world->maps[player_floor].actors, player_index);

    const size_t hero_floor = TCOD_zip_get_int(zip);
    const size_t hero_index = TCOD_zip_get_int(zip);
    world->hero = list_get(world->maps[hero_floor].actors, hero_index);

    const size_t num_messages = TCOD_zip_get_int(zip);
    for (int message_index = 0; message_index < num_messages; message_index++)
    {
        struct message *message = malloc(sizeof(struct message));
        assert(message);

        message->text = TCOD_strdup(TCOD_zip_get_string(zip));
        message->color = TCOD_zip_get_color(zip);

        list_add(world->messages, message);
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
    for (size_t object_index = 0; object_index < map->objects->size; object_index++)
    {
        struct object *const object = list_get(map->objects, object_index);

        object_calc_light(object);
    }

    for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
    {
        struct actor *const actor = list_get(map->actors, actor_index);

        actor_calc_light(actor);
        actor_calc_fade(actor, delta_time);
    }

    for (size_t projectile_index = 0; projectile_index < map->projectiles->size; projectile_index++)
    {
        struct projectile *const projectile = list_get(map->projectiles, projectile_index);

        if (projectile_move(projectile, delta_time))
        {
            projectile_calc_light(projectile);
        }
        else
        {
            list_remove_at(map->projectiles, projectile_index--);

            projectile_delete(projectile);
        }
    }

    for (size_t explosion_index = 0; explosion_index < map->explosions->size; explosion_index++)
    {
        struct explosion *const explosion = list_get(map->explosions, explosion_index);

        if (!explosion_update(explosion, delta_time))
        {
            list_remove_at(map->explosions, explosion_index--);

            explosion_delete(explosion);
        }
    }

    // calculate player fov every frame because of moving lights (like projectiles) that can reveal things in between turns
    actor_calc_fov(world->player);

    // process actor turns as long no animations are playing
    while (map->projectiles->size == 0 &&
           map->explosions->size == 0)
    {
        // update things that should be updated per-turn
        for (size_t object_index = 0; object_index < map->objects->size; object_index++)
        {
            struct object *const object = list_get(map->objects, object_index);

            object_calc_light(object);
        }

        for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
        {
            struct actor *const actor = list_get(map->actors, actor_index);

            actor_calc_light(actor);
        }

        for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
        {
            struct actor *const actor = list_get(map->actors, actor_index);

            actor_calc_fov(actor);
        }

        // check if the last actor in the map has been reached
        if (map->current_actor_index >= map->actors->size)
        {
            // move the pointer to the first actor
            map->current_actor_index = 0;

            // update world state
            world->time++;

            // reset all actor turns
            // if there are no controllable actors, return control back to the UI so the current state will be rendered
            bool controllable_exists = false;
            for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
            {
                struct actor *const actor = list_get(map->actors, actor_index);

                actor->took_turn = false;
                actor->energy += actor_calc_speed(actor);

                if (actor->controllable)
                {
                    controllable_exists = true;
                }

                if (actor->dead)
                {
                    // remove from map
                    list_remove_at(map->actors, actor_index--);

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
                    list_add(map->corpses, corpse);
                    list_add(tile->corpses, corpse);

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

                                list_add(tile->items, equipment);
                                list_add(map->items, equipment);

                                actor->equipment[i] = NULL;
                            }
                        }

                        // move inventory to ground
                        for (size_t item_index = 0; item_index < actor->items->size; item_index++)
                        {
                            struct item *const item = list_get(actor->items, item_index);

                            item->floor = actor->floor;
                            item->x = actor->x;
                            item->y = actor->y;

                            list_add(tile->items, item);
                            list_add(map->items, item);

                            list_remove_at(actor->items, item_index--);
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
            }
            if (!controllable_exists)
            {
                break;
            }
        }

        // get the current actor and figure out their turn(s), as long as they have energy
        struct actor *const actor = list_get(map->actors, map->current_actor_index);
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
                    static float timer = 0;
                    timer += delta_time;
                    if (timer < 1)
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
    if (world->player != list_get(map->actors, map->current_actor_index))
    {
        return false;
    }

    if (map->projectiles->size > 0)
    {
        return false;
    }

    if (map->explosions->size > 0)
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
        list_add(world->messages, message);
        line_begin = line_end + 1;
    } while (line_end);

    free(string);
}
