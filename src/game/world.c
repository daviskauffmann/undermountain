#include "world.h"

#include "actor.h"
#include "assets.h"
#include "color.h"
#include "corpse.h"
#include "explosion.h"
#include "message.h"
#include "object.h"
#include "projectile.h"
#include "room.h"
#include "spell.h"
#include "util.h"
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

    world->random = NULL;

    world->time = 0;

    world->spawned_unique_item_types = list_new();

    for (uint8_t floor = 0; floor < NUM_MAPS; floor++)
    {
        map_init(&world->maps[floor], floor);
    }

    world->player = NULL;

    world->hero = NULL;
    world->doomed = false;

    world->messages = list_new();
}

void world_uninit(void)
{
    for (size_t message_index = 0; message_index < world->messages->size; message_index++)
    {
        message_delete(list_get(world->messages, message_index));
    }
    list_delete(world->messages);

    for (uint8_t floor = 0; floor < NUM_MAPS; floor++)
    {
        map_uninit(&world->maps[floor]);
    }

    list_delete(world->spawned_unique_item_types);

    if (world->random)
    {
        TCOD_random_delete(world->random);
        TCOD_namegen_destroy();
    }

    if (world->doomed)
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
                (int[]){
                    [ABILITY_STRENGTH] = 10,
                    [ABILITY_DEXTERITY] = 10,
                    [ABILITY_CONSTITUTION] = 10,
                    [ABILITY_INTELLIGENCE] = 10,
                },
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
        color_white,
        "Hail, %s!",
        world->hero->name);

    printf("World created.\n");
}

// TODO: maybe this function should just take in a stream instead of loading the file
// call it world_serialize? and world_deserialize?
void world_save(const char *filename)
{
    FILE *const file = fopen(filename, "wb");

    fwrite(world->random, sizeof(*world->random), 1, file);

    fwrite(&world->spawned_unique_item_types->size, sizeof(world->spawned_unique_item_types->size), 1, file);
    for (size_t spawned_unique_item_type_index = 0; spawned_unique_item_type_index < world->spawned_unique_item_types->size; spawned_unique_item_type_index++)
    {
        const enum item_type type = (size_t)list_get(world->spawned_unique_item_types, spawned_unique_item_type_index);

        fwrite(&type, sizeof(type), 1, file);
    }

    fwrite(&world->time, sizeof(world->time), 1, file);

    uint8_t player_floor = 0;
    size_t player_index = 0;
    uint8_t hero_floor = 0;
    size_t hero_index = 0;

    for (uint8_t floor = 0; floor < NUM_MAPS; floor++)
    {
        const struct map *const map = &world->maps[floor];

        fwrite(&map->stair_down_x, sizeof(map->stair_down_x), 1, file);
        fwrite(&map->stair_down_y, sizeof(map->stair_down_y), 1, file);
        fwrite(&map->stair_up_x, sizeof(map->stair_up_x), 1, file);
        fwrite(&map->stair_up_y, sizeof(map->stair_up_y), 1, file);

        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                const struct tile *const tile = &map->tiles[x][y];

                fwrite(&tile->type, sizeof(tile->type), 1, file);
                fwrite(&tile->explored, sizeof(tile->explored), 1, file);
            }
        }

        fwrite(&map->rooms->size, sizeof(map->rooms->size), 1, file);
        for (size_t room_index = 0; room_index < map->rooms->size; room_index++)
        {
            const struct room *const room = list_get(map->rooms, room_index);

            fwrite(&room->x, sizeof(room->x), 1, file);
            fwrite(&room->y, sizeof(room->y), 1, file);
            fwrite(&room->w, sizeof(room->w), 1, file);
            fwrite(&room->h, sizeof(room->h), 1, file);
        }

        fwrite(&map->objects->size, sizeof(map->objects->size), 1, file);
        for (size_t object_index = 0; object_index < map->objects->size; object_index++)
        {
            const struct object *const object = list_get(map->objects, object_index);

            fwrite(&object->type, sizeof(object->type), 1, file);

            fwrite(&object->x, sizeof(object->x), 1, file);
            fwrite(&object->y, sizeof(object->y), 1, file);
        }

        fwrite(&map->actors->size, sizeof(map->actors->size), 1, file);
        for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
        {
            const struct actor *const actor = list_get(map->actors, actor_index);

            const size_t actor_name_length = strlen(actor->name);
            fwrite(&actor_name_length, sizeof(actor_name_length), 1, file);
            fwrite(actor->name, actor_name_length, 1, file);

            fwrite(&actor->race, sizeof(actor->race), 1, file);
            fwrite(&actor->class, sizeof(actor->class), 1, file);
            fwrite(&actor->faction, sizeof(actor->faction), 1, file);

            fwrite(&actor->level, sizeof(actor->level), 1, file);
            fwrite(&actor->experience, sizeof(actor->experience), 1, file);
            fwrite(&actor->ability_points, sizeof(actor->ability_points), 1, file);

            fwrite(&actor->ability_scores, sizeof(actor->ability_scores), 1, file);

            fwrite(&actor->base_hit_points, sizeof(actor->base_hit_points), 1, file);
            fwrite(&actor->base_mana_points, sizeof(actor->base_mana_points), 1, file);

            fwrite(&actor->hit_points, sizeof(actor->hit_points), 1, file);
            fwrite(&actor->mana_points, sizeof(actor->mana_points), 1, file);

            fwrite(&actor->gold, sizeof(actor->gold), 1, file);

            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                const struct item *const item = actor->equipment[equip_slot];

                if (item)
                {
                    const bool exists = true;
                    fwrite(&exists, sizeof(exists), 1, file);

                    fwrite(&item->type, sizeof(item->type), 1, file);

                    fwrite(&item->stack, sizeof(item->stack), 1, file);

                    fwrite(&item->durability, sizeof(item->durability), 1, file);
                }
                else
                {
                    const bool exists = false;
                    fwrite(&exists, sizeof(exists), 1, file);
                }
            }

            fwrite(&actor->items->size, sizeof(actor->items->size), 1, file);
            for (size_t item_index = 0; item_index < actor->items->size; item_index++)
            {
                const struct item *const item = list_get(actor->items, item_index);

                fwrite(&item->type, sizeof(item->type), 1, file);

                fwrite(&item->stack, sizeof(item->stack), 1, file);

                fwrite(&item->durability, sizeof(item->durability), 1, file);
            }

            fwrite(&actor->known_spell_types->size, sizeof(actor->known_spell_types->size), 1, file);
            for (size_t known_spell_type_index = 0; known_spell_type_index < actor->known_spell_types->size; known_spell_type_index++)
            {
                const enum spell_type spell_type = (size_t)(list_get(actor->known_spell_types, known_spell_type_index));

                fwrite(&spell_type, sizeof(spell_type), 1, file);
            }

            fwrite(&actor->readied_spell_type, sizeof(actor->readied_spell_type), 1, file);

            fwrite(&actor->x, sizeof(actor->x), 1, file);
            fwrite(&actor->y, sizeof(actor->y), 1, file);

            fwrite(&actor->took_turn, sizeof(actor->took_turn), 1, file);
            fwrite(&actor->energy, sizeof(actor->energy), 1, file);

            fwrite(&actor->last_seen_x, sizeof(actor->last_seen_x), 1, file);
            fwrite(&actor->last_seen_y, sizeof(actor->last_seen_y), 1, file);
            fwrite(&actor->turns_chased, sizeof(actor->turns_chased), 1, file);

            fwrite(&actor->light_type, sizeof(actor->light_type), 1, file);

            fwrite(&actor->flash_color, sizeof(actor->flash_color), 1, file);
            fwrite(&actor->flash_fade_coef, sizeof(actor->flash_fade_coef), 1, file);

            fwrite(&actor->controllable, sizeof(actor->controllable), 1, file);

            fwrite(&actor->dead, sizeof(actor->dead), 1, file);

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

            const size_t leader_index = list_index_of(map->actors, actor->leader);
            fwrite(&leader_index, sizeof(leader_index), 1, file);
        }

        fwrite(&map->corpses->size, sizeof(map->corpses->size), 1, file);
        for (size_t corpse_index = 0; corpse_index < map->corpses->size; corpse_index++)
        {
            const struct corpse *const corpse = list_get(map->corpses, corpse_index);

            const size_t corpse_name_length = strlen(corpse->name);
            fwrite(&corpse_name_length, sizeof(corpse_name_length), 1, file);
            fwrite(corpse->name, corpse_name_length, 1, file);

            fwrite(&corpse->level, sizeof(corpse->level), 1, file);

            fwrite(&corpse->x, sizeof(corpse->x), 1, file);
            fwrite(&corpse->y, sizeof(corpse->y), 1, file);
        }

        fwrite(&map->items->size, sizeof(map->items->size), 1, file);
        for (size_t item_index = 0; item_index < map->items->size; item_index++)
        {
            const struct item *const item = list_get(map->items, item_index);

            fwrite(&item->type, sizeof(item->type), 1, file);

            fwrite(&item->x, sizeof(item->x), 1, file);
            fwrite(&item->y, sizeof(item->y), 1, file);

            fwrite(&item->stack, sizeof(item->stack), 1, file);

            fwrite(&item->durability, sizeof(item->durability), 1, file);
        }

        fwrite(&map->projectiles->size, sizeof(map->projectiles->size), 1, file);
        for (size_t projectile_index = 0; projectile_index < map->projectiles->size; projectile_index++)
        {
            const struct projectile *const projectile = list_get(map->projectiles, projectile_index);

            fwrite(&projectile->type, sizeof(projectile->type), 1, file);

            fwrite(&projectile->origin_x, sizeof(projectile->origin_x), 1, file);
            fwrite(&projectile->origin_y, sizeof(projectile->origin_y), 1, file);
            fwrite(&projectile->target_x, sizeof(projectile->target_x), 1, file);
            fwrite(&projectile->target_x, sizeof(projectile->target_x), 1, file);
            fwrite(&projectile->x, sizeof(projectile->x), 1, file);
            fwrite(&projectile->y, sizeof(projectile->y), 1, file);

            const size_t shooter_index = list_index_of(map->actors, projectile->shooter);
            fwrite(&shooter_index, sizeof(shooter_index), 1, file);

            if (projectile->ammunition)
            {
                const bool has_ammunition = true;
                fwrite(&has_ammunition, sizeof(has_ammunition), 1, file);

                const struct item *const item = projectile->ammunition;

                fwrite(&item->type, sizeof(item->type), 1, file);

                fwrite(&item->stack, sizeof(item->stack), 1, file);

                fwrite(&item->durability, sizeof(item->durability), 1, file);
            }
            else
            {
                const bool has_ammunition = false;
                fwrite(&has_ammunition, sizeof(has_ammunition), 1, file);
            }
        }

        fwrite(&map->explosions->size, sizeof(map->explosions->size), 1, file);
        for (size_t explosion_index = 0; explosion_index < map->explosions->size; explosion_index++)
        {
            const struct explosion *const explosion = list_get(map->explosions, explosion_index);

            fwrite(&explosion->x, sizeof(explosion->x), 1, file);
            fwrite(&explosion->y, sizeof(explosion->y), 1, file);

            fwrite(&explosion->radius, sizeof(explosion->radius), 1, file);
            fwrite(&explosion->color, sizeof(explosion->color), 1, file);

            fwrite(&explosion->time, sizeof(explosion->time), 1, file);
        }

        fwrite(&map->current_actor_index, sizeof(map->current_actor_index), 1, file);
    }

    fwrite(&player_floor, sizeof(player_floor), 1, file);
    fwrite(&player_index, sizeof(player_index), 1, file);
    fwrite(&hero_floor, sizeof(hero_floor), 1, file);
    fwrite(&hero_index, sizeof(hero_index), 1, file);

    fwrite(&world->messages->size, sizeof(world->messages->size), 1, file);
    for (size_t message_index = 0; message_index < world->messages->size; message_index++)
    {
        const struct message *const message = list_get(world->messages, message_index);

        const size_t message_length = strlen(message->text);
        fwrite(&message_length, sizeof(message_length), 1, file);
        fwrite(message->text, message_length, 1, file);

        fwrite(&message->color, sizeof(message->color), 1, file);
    }

    fclose(file);

    printf("World saved.\n");
}

void world_load(const char *filename)
{
    FILE *const file = fopen(filename, "rb");

    world->random = TCOD_random_new(TCOD_RNG_MT);
    fread(world->random, sizeof(*world->random), 1, file);

    TCOD_namegen_parse("data/namegen.cfg", world->random);

    size_t spawned_unique_item_types;
    fread(&spawned_unique_item_types, sizeof(spawned_unique_item_types), 1, file);
    for (size_t spawned_unique_item_type_index = 0; spawned_unique_item_type_index < spawned_unique_item_types; spawned_unique_item_type_index++)
    {
        enum item_type type;
        fread(&type, sizeof(type), 1, file);

        list_add(world->spawned_unique_item_types, (void *)(size_t)type);
    }

    fread(&world->time, sizeof(world->time), 1, file);

    for (uint8_t floor = 0; floor < NUM_MAPS; floor++)
    {
        struct map *const map = &world->maps[floor];

        fread(&map->stair_down_x, sizeof(map->stair_down_x), 1, file);
        fread(&map->stair_down_y, sizeof(map->stair_down_y), 1, file);
        fread(&map->stair_up_x, sizeof(map->stair_up_x), 1, file);
        fread(&map->stair_up_y, sizeof(map->stair_up_y), 1, file);

        for (size_t x = 0; x < MAP_WIDTH; x++)
        {
            for (size_t y = 0; y < MAP_HEIGHT; y++)
            {
                struct tile *const tile = &map->tiles[x][y];

                fread(&tile->type, sizeof(tile->type), 1, file);
                fread(&tile->explored, sizeof(tile->explored), 1, file);
            }
        }

        size_t num_rooms;
        fread(&num_rooms, sizeof(num_rooms), 1, file);
        for (size_t room_index = 0; room_index < num_rooms; room_index++)
        {
            struct room *const room = malloc(sizeof(*room));

            fread(&room->x, sizeof(room->x), 1, file);
            fread(&room->y, sizeof(room->y), 1, file);
            fread(&room->w, sizeof(room->w), 1, file);
            fread(&room->h, sizeof(room->h), 1, file);

            list_add(map->rooms, room);
        }

        size_t num_objects;
        fread(&num_objects, sizeof(num_objects), 1, file);
        for (size_t i = 0; i < num_objects; i++)
        {
            struct object *const object = malloc(sizeof(*object));

            fread(&object->type, sizeof(object->type), 1, file);

            object->floor = floor;
            fread(&object->x, sizeof(object->x), 1, file);
            fread(&object->y, sizeof(object->y), 1, file);

            object->light_fov = NULL;

            list_add(map->objects, object);
            map->tiles[object->x][object->y].object = object;
        }

        size_t num_actors;
        fread(&num_actors, sizeof(num_actors), 1, file);
        for (size_t actor_index = 0; actor_index < num_actors; actor_index++)
        {
            struct actor *const actor = malloc(sizeof(*actor));

            size_t actor_name_length;
            fread(&actor_name_length, sizeof(actor_name_length), 1, file);
            actor->name = malloc(actor_name_length + 1);
            fread(actor->name, actor_name_length, 1, file);
            actor->name[actor_name_length] = '\0';

            fread(&actor->race, sizeof(actor->race), 1, file);
            fread(&actor->class, sizeof(actor->class), 1, file);
            fread(&actor->faction, sizeof(actor->faction), 1, file);

            fread(&actor->level, sizeof(actor->level), 1, file);
            fread(&actor->experience, sizeof(actor->experience), 1, file);
            fread(&actor->ability_points, sizeof(actor->ability_points), 1, file);

            fread(&actor->ability_scores, sizeof(actor->ability_scores), 1, file);

            fread(&actor->base_hit_points, sizeof(actor->base_hit_points), 1, file);
            fread(&actor->base_mana_points, sizeof(actor->base_mana_points), 1, file);

            fread(&actor->hit_points, sizeof(actor->hit_points), 1, file);
            fread(&actor->mana_points, sizeof(actor->mana_points), 1, file);

            fread(&actor->gold, sizeof(actor->gold), 1, file);

            for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                bool exists;
                fread(&exists, sizeof(exists), 1, file);

                if (exists)
                {
                    struct item *const item = malloc(sizeof(*item));

                    fread(&item->type, sizeof(item->type), 1, file);

                    fread(&item->stack, sizeof(item->stack), 1, file);

                    fread(&item->durability, sizeof(item->durability), 1, file);

                    actor->equipment[equip_slot] = item;
                }
                else
                {
                    actor->equipment[equip_slot] = NULL;
                }
            }

            actor->items = list_new();
            size_t num_items;
            fread(&num_items, sizeof(num_items), 1, file);
            for (size_t item_index = 0; item_index < num_items; item_index++)
            {
                struct item *const item = malloc(sizeof(*item));

                fread(&item->type, sizeof(item->type), 1, file);

                fread(&item->stack, sizeof(item->stack), 1, file);

                fread(&item->durability, sizeof(item->durability), 1, file);

                list_add(actor->items, item);
            }

            actor->known_spell_types = list_new();
            size_t num_known_spell_types;
            fread(&num_known_spell_types, sizeof(num_known_spell_types), 1, file);
            for (size_t known_spell_type_index = 0; known_spell_type_index < num_known_spell_types; known_spell_type_index++)
            {
                enum spell_type spell_type;
                fread(&spell_type, sizeof(spell_type), 1, file);

                list_add(actor->known_spell_types, (void *)(size_t)spell_type);
            }

            fread(&actor->readied_spell_type, sizeof(actor->readied_spell_type), 1, file);

            actor->floor = floor;
            fread(&actor->x, sizeof(actor->x), 1, file);
            fread(&actor->y, sizeof(actor->y), 1, file);

            actor->fov = NULL;

            fread(&actor->took_turn, sizeof(actor->took_turn), 1, file);
            fread(&actor->energy, sizeof(actor->energy), 1, file);

            fread(&actor->last_seen_x, sizeof(actor->last_seen_x), 1, file);
            fread(&actor->last_seen_y, sizeof(actor->last_seen_y), 1, file);
            fread(&actor->turns_chased, sizeof(actor->turns_chased), 1, file);

            actor->leader = NULL;

            fread(&actor->light_type, sizeof(actor->light_type), 1, file);
            actor->light_fov = NULL;

            fread(&actor->flash_color, sizeof(actor->flash_color), 1, file);
            fread(&actor->flash_fade_coef, sizeof(actor->flash_fade_coef), 1, file);

            fread(&actor->controllable, sizeof(actor->controllable), 1, file);

            fread(&actor->dead, sizeof(actor->dead), 1, file);

            list_add(map->actors, actor);
            map->tiles[actor->x][actor->y].actor = actor;
        }

        for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
        {
            struct actor *const actor = list_get(map->actors, actor_index);

            size_t leader_index;
            fread(&leader_index, sizeof(leader_index), 1, file);
            actor->leader = list_get(map->actors, leader_index);
        }

        size_t num_corpses;
        fread(&num_corpses, sizeof(num_corpses), 1, file);
        for (size_t corpse_index = 0; corpse_index < num_corpses; corpse_index++)
        {
            struct corpse *corpse = malloc(sizeof(*corpse));

            size_t corpse_name_length;
            fread(&corpse_name_length, sizeof(corpse_name_length), 1, file);
            corpse->name = malloc(corpse_name_length + 1);
            fread(corpse->name, corpse_name_length, 1, file);
            corpse->name[corpse_name_length] = '\0';

            fread(&corpse->level, sizeof(corpse->level), 1, file);

            corpse->floor = floor;
            fread(&corpse->x, sizeof(corpse->x), 1, file);
            fread(&corpse->y, sizeof(corpse->y), 1, file);

            list_add(map->corpses, corpse);
            list_add(map->tiles[corpse->x][corpse->y].corpses, corpse);
        }

        size_t num_items;
        fread(&num_items, sizeof(num_items), 1, file);
        for (size_t i = 0; i < num_items; i++)
        {
            struct item *const item = malloc(sizeof(*item));

            fread(&item->type, sizeof(item->type), 1, file);

            item->floor = floor;
            fread(&item->x, sizeof(item->x), 1, file);
            fread(&item->y, sizeof(item->y), 1, file);

            fread(&item->stack, sizeof(item->stack), 1, file);

            fread(&item->durability, sizeof(item->durability), 1, file);

            list_add(map->items, item);
            list_add(map->tiles[item->x][item->y].items, item);
        }

        size_t num_projectiles;
        fread(&num_projectiles, sizeof(num_projectiles), 1, file);
        for (size_t projectile_index = 0; projectile_index < num_projectiles; projectile_index++)
        {
            struct projectile *const projectile = malloc(sizeof(*projectile));

            fread(&projectile->type, sizeof(projectile->type), 1, file);

            projectile->floor = floor;
            fread(&projectile->origin_x, sizeof(projectile->origin_x), 1, file);
            fread(&projectile->origin_y, sizeof(projectile->origin_y), 1, file);
            fread(&projectile->target_x, sizeof(projectile->target_x), 1, file);
            fread(&projectile->target_y, sizeof(projectile->target_y), 1, file);
            fread(&projectile->x, sizeof(projectile->x), 1, file);
            fread(&projectile->y, sizeof(projectile->y), 1, file);

            size_t shooter_index;
            fread(&shooter_index, sizeof(shooter_index), 1, file);
            projectile->shooter = list_get(map->actors, shooter_index);

            bool ammunition_exists;
            fread(&ammunition_exists, sizeof(ammunition_exists), 1, file);

            if (ammunition_exists)
            {
                struct item *const item = malloc(sizeof(*item));

                fread(&item->type, sizeof(item->type), 1, file);

                fread(&item->stack, sizeof(item->stack), 1, file);

                fread(&item->durability, sizeof(item->durability), 1, file);

                projectile->ammunition = item;
            }
            else
            {
                projectile->ammunition = NULL;
            }

            projectile->light_fov = NULL;

            list_add(map->projectiles, projectile);
        }

        size_t num_explosions;
        fread(&num_explosions, sizeof(num_explosions), 1, file);
        for (size_t explosion_index = 0; explosion_index < num_explosions; explosion_index++)
        {
            struct explosion *const explosion = malloc(sizeof(*explosion));

            explosion->floor = floor;
            fread(&explosion->x, sizeof(explosion->x), 1, file);
            fread(&explosion->y, sizeof(explosion->y), 1, file);

            fread(&explosion->radius, sizeof(explosion->radius), 1, file);
            fread(&explosion->color, sizeof(explosion->color), 1, file);

            fread(&explosion->time, sizeof(explosion->time), 1, file);

            explosion->fov = map_to_fov_map(
                map,
                explosion->x,
                explosion->y,
                explosion->radius);

            list_add(map->explosions, explosion);
        }

        fread(&map->current_actor_index, sizeof(map->current_actor_index), 1, file);
    }

    uint8_t player_floor;
    fread(&player_floor, sizeof(player_floor), 1, file);
    size_t player_index;
    fread(&player_index, sizeof(player_index), 1, file);
    world->player = list_get(world->maps[player_floor].actors, player_index);

    uint8_t hero_floor;
    fread(&hero_floor, sizeof(hero_floor), 1, file);
    size_t hero_index;
    fread(&hero_index, sizeof(hero_index), 1, file);
    world->hero = list_get(world->maps[hero_floor].actors, hero_index);

    size_t num_messages;
    fread(&num_messages, sizeof(num_messages), 1, file);
    for (int message_index = 0; message_index < num_messages; message_index++)
    {
        struct message *const message = malloc(sizeof(*message));

        size_t message_text_length;
        fread(&message_text_length, sizeof(message_text_length), 1, file);
        message->text = malloc(message_text_length + 1);
        fread(message->text, message_text_length, 1, file);
        message->text[message_text_length] = '\0';

        fread(&message->color, sizeof(message->color), 1, file);

        list_add(world->messages, message);
    }

    fclose(file);

    world_log(
        -1,
        -1,
        -1,
        color_white,
        "Welcome back, %s!",
        world->hero->name);

    printf("World loaded.\n");
}

void world_update(float delta_time)
{
    struct map *const map = &world->maps[world->hero->floor];

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

    for (size_t actor_index = 0; actor_index < map->actors->size; actor_index++)
    {
        struct actor *const actor = list_get(map->actors, actor_index);

        actor_calc_fov(actor);
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

    // process actor turns as long no animations are playing
    while (!map_is_animation_playing(map))
    {
        // check if the last actor in the map has been reached
        // if so, process once-per-turn things
        if (map->current_actor_index >= map->actors->size)
        {
            // move the pointer to the first actor
            map->current_actor_index = 0;

            // update world state
            world->time++;

            // update surfaces
            for (size_t surface_index = 0; surface_index < map->surfaces->size; surface_index++)
            {
                struct surface *const surface = list_get(map->surfaces, surface_index);
                const struct surface_data *const surface_data = &surface_database[surface->type];

                surface->time++;

                if (surface->time >= surface_data->duration)
                {
                    list_remove_at(map->surfaces, surface_index--);

                    struct tile *const tile = &map->tiles[surface->x][surface->y];
                    tile->surface = NULL;

                    surface_delete(surface);

                    continue;
                }
            }

            bool controllable_exists = false;

            // reset all actor turns
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
                        for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
                        {
                            struct item *const equipment = actor->equipment[equip_slot];

                            if (equipment)
                            {
                                equipment->floor = actor->floor;
                                equipment->x = actor->x;
                                equipment->y = actor->y;

                                list_add(tile->items, equipment);
                                list_add(map->items, equipment);

                                actor->equipment[equip_slot] = NULL;
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
                        world->doomed = true;

                        world_log(
                            actor->floor,
                            actor->x,
                            actor->y,
                            color_green,
                            "Game over! Press 'ESC' to return to the menu.");
                    }
                    else
                    {
                        actor_delete(actor);
                    }
                }
            }

            // if there are no controllable actors, return control back to the UI so the current state will be rendered
            // the next frame will go on to process actor turns ahead
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
                if (world->doomed)
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
            // for non-controllable actors, it was already done earlier
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

bool world_can_player_take_turn(void)
{
    if (world->doomed)
    {
        return false;
    }

    const struct map *const map = &world->maps[world->player->floor];

    if (world->player != list_get(map->actors, map->current_actor_index))
    {
        return false;
    }

    if (map_is_animation_playing(map))
    {
        return false;
    }

    return true;
}

void world_log(int floor, int x, int y, TCOD_ColorRGB color, const char *fmt, ...)
{
    if (floor != -1)
    {
        if (!world->player)
        {
            return;
        }

        if (floor != world->player->floor)
        {
            return;
        }

        if (!world->player->fov)
        {
            return;
        }

        if (!TCOD_map_is_in_fov(world->player->fov, x, y))
        {
            return;
        }
    }

    va_list args;
    va_start(args, fmt);

    const size_t size = vsnprintf(NULL, 0, fmt, args);
    char *string = malloc(size + 1);
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
