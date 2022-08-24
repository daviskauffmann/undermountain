#include "actor.h"

#include "assets.h"
#include "explosion.h"
#include "item.h"
#include "projectile.h"
#include "room.h"
#include "util.h"
#include "world.h"
#include <assert.h>
#include <float.h>
#include <malloc.h>
#include <math.h>

struct actor *actor_new(
    const char *const name,
    const enum race race,
    const enum class class,
    const enum faction faction,
    const uint8_t level,
    const uint8_t floor,
    const uint8_t x,
    const uint8_t y,
    const bool torch)
{
    struct actor *const actor = malloc(sizeof(*actor));
    assert(actor);

    actor->name = TCOD_strdup(name);
    actor->race = race;
    actor->class = class;
    actor->faction = faction;

    actor->level = level;
    actor->experience = actor_calc_experience_to_level(actor->level);

    actor->max_hp = 10 + (10 * level);
    actor->current_hp = actor->max_hp;

    actor->gold = 0;
    for (enum equip_slot equip_slot = 0; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
    {
        actor->equipment[equip_slot] = NULL;
    }
    actor->items = TCOD_list_new();

    actor->readied_spell = SPELL_TYPE_HEAL;

    actor->floor = floor;
    actor->x = x;
    actor->y = y;

    actor->fov = NULL;

    actor->took_turn = false;
    actor->energy = 1.0f;
    actor->energy_per_turn = race_data[race].speed;

    actor->last_seen_x = -1;
    actor->last_seen_y = -1;
    actor->turns_chased = 0;

    actor->leader = NULL;

    actor->light_radius = -1;
    actor->light_color = TCOD_white;
    actor->light_intensity = 0;
    actor->light_flicker = false;
    actor->light_fov = NULL;
    if (torch)
    {
        actor->light_radius = actor_common.torch_radius;
        actor->light_color = actor_common.torch_color;
        actor->light_intensity = actor_common.torch_intensity;
        actor->light_flicker = true;
    }

    actor->flash_color = TCOD_white;
    actor->flash_fade_coef = 0.0f;

    actor->controllable = false;

    return actor;
}

void actor_delete(struct actor *const actor)
{
    if (actor->light_fov != NULL)
    {
        TCOD_map_delete(actor->light_fov);
    }

    if (actor->fov != NULL)
    {
        TCOD_map_delete(actor->fov);
    }

    TCOD_LIST_FOREACH(actor->items)
    {
        struct item *item = *iterator;
        item_delete(item);
    }
    TCOD_list_delete(actor->items);

    free(actor->name);

    free(actor);
}

int actor_calc_experience_to_level(const int level)
{
    return level * (level - 1) / 2 * 1000;
}

void actor_update(struct actor *const actor, const float delta_time)
{
    if (actor->flash_fade_coef > 0)
    {
        // TODO: slower/faster fade depending on circumstances
        // TODO: different fade functions such as sin()
        actor->flash_fade_coef -= 4.0f * delta_time;
    }
    else
    {
        actor->flash_fade_coef = 0.0f;
    }
}

void actor_calc_light(struct actor *const actor)
{
    if (actor->light_fov)
    {
        TCOD_map_delete(actor->light_fov);
        actor->light_fov = NULL;
    }

    if (actor->light_radius >= 0)
    {
        actor->light_fov = map_to_fov_map(
            &world->maps[actor->floor],
            actor->x,
            actor->y,
            actor->light_radius);
    }
}

void actor_calc_fov(struct actor *const actor)
{
    if (actor->fov)
    {
        TCOD_map_delete(actor->fov);
    }

    struct map *map = &world->maps[actor->floor];
    actor->fov = map_to_fov_map(map, actor->x, actor->y, 1);

    TCOD_map_t los_map = map_to_fov_map(map, actor->x, actor->y, 0);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            if (!TCOD_map_is_in_fov(actor->fov, x, y) &&
                TCOD_map_is_in_fov(los_map, x, y))
            {
                TCOD_LIST_FOREACH(map->objects)
                {
                    const struct object *const object = *iterator;
                    if (object->light_fov &&
                        TCOD_map_is_in_fov(object->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                TCOD_LIST_FOREACH(map->actors)
                {
                    const struct actor *const other = *iterator;
                    if (other->light_fov &&
                        TCOD_map_is_in_fov(other->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                TCOD_LIST_FOREACH(map->projectiles)
                {
                    const struct projectile *const projectile = *iterator;
                    if (projectile->light_fov &&
                        TCOD_map_is_in_fov(projectile->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                TCOD_LIST_FOREACH(map->explosions)
                {
                    const struct explosion *const explosion = *iterator;
                    if (explosion->fov &&
                        TCOD_map_is_in_fov(explosion->fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }
            }
        }
    }

    TCOD_map_delete(los_map);
}

void actor_ai(struct actor *const actor)
{
    const struct map *const map = &world->maps[actor->floor];
    const struct tile *const tile = &map->tiles[actor->x][actor->y];

    // look for fountains to heal if low health
    if (actor->current_hp < actor->max_hp / 2)
    {
        const struct object *target = NULL;

        float min_distance = FLT_MAX;
        TCOD_LIST_FOREACH(map->objects)
        {
            const struct object *const object = *iterator;
            if (TCOD_map_is_in_fov(actor->fov, object->x, object->y) &&
                object->type == OBJECT_TYPE_FOUNTAIN)
            {
                const float distance = distance_between_sq(
                    actor->x, actor->y,
                    object->x, object->y);
                if (distance < min_distance)
                {
                    target = object;
                    min_distance = distance;
                }
            }
        }

        if (target)
        {
            if (distance_between(
                    actor->x, actor->y,
                    target->x, target->y) < 2.0f)
            {
                if (actor_drink(actor, target->x, target->y))
                {
                    goto done;
                }
            }
            else
            {
                if (actor_path_towards(actor, target->x, target->y))
                {
                    goto done;
                }
            }
        }
    }

    // look for hostile targets
    {
        struct actor *target = NULL;

        float min_distance = FLT_MAX;
        TCOD_LIST_FOREACH(map->actors)
        {
            struct actor *const other = *iterator;
            if (TCOD_map_is_in_fov(actor->fov, other->x, other->y) &&
                other->faction != actor->faction)
            {
                const float distance = distance_between_sq(
                    actor->x, actor->y,
                    other->x, other->y);
                if (distance < min_distance)
                {
                    target = other;
                    min_distance = distance;
                }
            }
        }

        if (target)
        {
            // target spotted, so remember the location in case the actor loses them
            actor->last_seen_x = target->x;
            actor->last_seen_y = target->y;
            actor->turns_chased = 0;

            bool ranged = false;

            const struct item *const weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];
            if (weapon)
            {
                const struct item_datum *const weapon_datum = &item_data[weapon->type];
                if (weapon_datum->ranged)
                {
                    // does the actor have ammo?
                    struct item *ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];
                    if (ammunition)
                    {
                        // is it the correct ammo?
                        const struct item_datum *const ammunition_datum = &item_data[ammunition->type];
                        if (weapon_datum->ammunition_type == ammunition_datum->ammunition_type)
                        {
                            ranged = true;
                        }
                    }
                    else
                    {
                        // out of ammo or using the wrong ammo, so unequip the weapon
                        // TODO: look in inventory for suitable ammo and equip
                        if (actor_unequip(actor, EQUIP_SLOT_MAIN_HAND))
                        {
                            goto done;
                        }
                    }
                }
            }

            if (ranged)
            {
                if (actor_shoot(actor, target->x, target->y))
                {
                    // do not set took_turn to true
                    // the turn is not complete until the projectile is done moving
                    return;
                }
            }
            else
            {
                if (distance_between(
                        actor->x, actor->y,
                        target->x, target->y) < 2.0f)
                {
                    if (actor_attack(actor, target, NULL))
                    {
                        goto done;
                    }
                }
                else
                {
                    if (actor_path_towards(actor, target->x, target->y))
                    {
                        goto done;
                    }
                }
            }
        }
    }

    // go to where a hostile target was recently seen
    if (actor->last_seen_x != -1 && actor->last_seen_y != -1)
    {
        if ((actor->x == actor->last_seen_x && actor->y == actor->last_seen_y) ||
            actor->turns_chased > actor_common.turns_to_chase)
        {
            actor->last_seen_x = -1;
            actor->last_seen_y = -1;
        }
        else if (actor_path_towards(actor, actor->last_seen_x, actor->last_seen_y))
        {
            actor->turns_chased++;

            goto done;
        }
    }

    // stay in visiblity/proximity to leader
    if (actor->leader)
    {
        if (!TCOD_map_is_in_fov(actor->fov, actor->leader->x, actor->leader->y) ||
            distance_between(actor->x, actor->y, actor->leader->x, actor->leader->y) > 5.0f)
        {
            if (actor_path_towards(actor, actor->leader->x, actor->leader->y))
            {
                goto done;
            }
        }
    }

    // TODO: move between floors (deferred until processing of inactive maps is figured out)
    // if (tile->object)
    // {
    //     switch (tile->object->type)
    //     {
    //     case OBJECT_TYPE_STAIR_DOWN:
    //     {
    //         if (actor_descend(actor))
    //         {
    //             goto done;
    //         }
    //     }
    //     break;
    //     case OBJECT_TYPE_STAIR_UP:
    //     {
    //         if (actor_ascend(actor))
    //         {
    //             goto done;
    //         }
    //     }
    //     break;
    //     }
    // }

    // TODO: look for objects to interact with if needed (fountains already done, but consider other types)

    // TODO: look for items to pick up

    // pick up items on ground
    if (TCOD_list_size(tile->items) > 0)
    {
        if (actor_grab(actor, actor->x, actor->y))
        {
            goto done;
        }
    }

    // move randomly
    if (TCOD_random_get_int(world->random, 0, 1) == 0)
    {
        const int x = actor->x + TCOD_random_get_int(world->random, -1, 1);
        const int y = actor->y + TCOD_random_get_int(world->random, -1, 1);
        actor_move(actor, x, y);

        goto done;
    }
done:;
    actor->took_turn = true;
}

void actor_give_experience(struct actor *const actor, const int experience)
{
    actor->experience += experience;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_azure,
        "%s gains %d experience.",
        actor->name,
        experience);

    while (actor->experience >= actor_calc_experience_to_level(actor->level + 1))
    {
        // TODO: do not automatically level up if it's the player character
        // notify the player instead and they can level up in a level up screen
        actor_level_up(actor);
    }
}

void actor_level_up(struct actor *const actor)
{
    // TODO: the player should not generally use this to level up
    // this is an automatic level up that all non-player actors will use when their experience is high enough
    // however, in the level up screen (to be implemented), the player can elect to auto-level their character
    // in that case, use this function

    actor->level++;
    actor->max_hp += 10;
    actor->current_hp = actor->max_hp;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_yellow,
        "%s has gained a level!",
        actor->name);
}

bool actor_path_towards(
    struct actor *const actor,
    const int target_x, const int target_y)
{
    TCOD_map_t TCOD_map = map_to_TCOD_map(&world->maps[actor->floor]);
    TCOD_map_set_properties(
        TCOD_map,
        target_x, target_y,
        TCOD_map_is_transparent(TCOD_map, target_x, target_y),
        true);

    TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1.0f);
    TCOD_path_compute(
        path,
        actor->x, actor->y,
        target_x, target_y);

    bool success = false;

    int next_x, next_y;
    if (!TCOD_path_is_empty(path) &&
        TCOD_path_walk(path, &next_x, &next_y, false))
    {
        success = actor_move(actor, next_x, next_y);
    }
    else
    {
        success = actor_move_towards(actor, target_x, target_y);
    }

    TCOD_path_delete(path);
    TCOD_map_delete(TCOD_map);

    return success;
}

bool actor_move_towards(
    struct actor *const actor,
    const int target_x, const int target_y)
{
    int dx = target_x - actor->x;
    int dy = target_y - actor->y;
    const float distance = distance_between(
        actor->x, actor->y,
        target_x, target_y);
    if (distance > 0.0f)
    {
        dx = (int)round(dx / distance);
        dy = (int)round(dy / distance);

        return actor_move(
            actor,
            actor->x + dx, actor->y + dy);
    }

    return false;
}

bool actor_move(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];

    if (!tile_data[tile->type].is_walkable)
    {
        return false;
    }

    if (tile->object)
    {
        const enum object_type object_type = tile->object->type;

        switch (object_type)
        {
        case OBJECT_TYPE_ALTAR:
        {
            return actor_pray(actor, x, y);
        }
        break;
        case OBJECT_TYPE_BRAZIER:
        {
        }
        break;
        case OBJECT_TYPE_CHEST:
        {
            return actor_open_chest(actor, x, y);
        }
        break;
        case OBJECT_TYPE_DOOR_CLOSED:
        {
            return actor_open_door(actor, x, y);
        }
        break;
        case OBJECT_TYPE_DOOR_OPEN:
        {
        }
        break;
        case OBJECT_TYPE_FOUNTAIN:
        {
            return actor_drink(actor, x, y);
        }
        break;
        case OBJECT_TYPE_STAIR_DOWN:
        {
        }
        break;
        case OBJECT_TYPE_STAIR_UP:
        {
        }
        break;
        case OBJECT_TYPE_THRONE:
        {
            return actor_sit(actor, x, y);
        }
        break;
        case OBJECT_TYPE_TRAP:
        {
            // TODO: trap effects

            TCOD_list_remove(map->objects, tile->object);

            object_delete(tile->object);
            tile->object = NULL;

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_white,
                "%s triggers a trap!",
                actor->name);
        }
        break;
        case NUM_OBJECT_TYPES:
            break;
        }

        if (!object_data[object_type].is_walkable)
        {
            return false;
        }
    }

    if (tile->actor && tile->actor != actor)
    {
        if (tile->actor->faction == actor->faction)
        {
            return actor_swap(actor, tile->actor);
        }
        else
        {
            const struct item *const weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];
            if (weapon && item_data[weapon->type].ranged)
            {
                return actor_shoot(world->player, x, y);
            }
            else
            {
                return actor_attack(actor, tile->actor, NULL);
            }
        }
    }

    struct tile *const current_tile = &map->tiles[actor->x][actor->y];
    current_tile->actor = NULL;

    tile->actor = actor;

    actor->x = x;
    actor->y = y;

    return true;
}

bool actor_swap(struct actor *const actor, struct actor *const other)
{
    // npc actors can't initiate a swap with the player
    if (other == world->player)
    {
        return false;
    }

    // swap actor coordinates
    const int temp_x = actor->x;
    const int temp_y = actor->y;
    actor->x = other->x;
    actor->y = other->y;
    other->x = temp_x;
    other->y = temp_y;

    // swap actor pointers in the tiles
    struct map *const map = &world->maps[actor->floor];
    map->tiles[actor->x][actor->y].actor = actor;
    map->tiles[other->x][other->y].actor = other;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s swaps with %s.",
        actor->name,
        other->name);

    return true;
}

bool actor_open_door(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (tile->object && tile->object->type == OBJECT_TYPE_DOOR_CLOSED)
    {
        tile->object->type = OBJECT_TYPE_DOOR_OPEN;

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_orange,
            "%s opens the door.",
            actor->name);

        return true;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't open the door.",
        actor->name);

    return false;
}

bool actor_close_door(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (tile->object && tile->object->type == OBJECT_TYPE_DOOR_OPEN)
    {
        tile->object->type = OBJECT_TYPE_DOOR_CLOSED;

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_orange,
            "%s closes the door.",
            actor->name);

        return true;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't close the door.",
        actor->name);

    return false;
}

bool actor_descend(
    struct actor *const actor,
    const bool is_leader,
    void ***const iterator)
{
    // is there a next map?
    if (actor->floor >= NUM_MAPS - 1)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s has reached the end.",
            actor->name);

        return false;
    }

    // is the actor on stairs down?
    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[actor->x][actor->y];
    if (is_leader &&
        (!tile->object || tile->object->type != OBJECT_TYPE_STAIR_DOWN))
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't descend here.",
            actor->name);

        return false;
    }

    // remove actor from current map
    if (iterator)
    {
        *iterator = TCOD_list_remove_iterator_fast(map->actors, *iterator);
    }
    else
    {
        TCOD_list_remove(map->actors, actor);
    }

    // remove actor from current tile
    tile->actor = NULL;

    // add actor to next map
    struct map *const next_map = &world->maps[actor->floor + 1];
    TCOD_list_push(next_map->actors, actor);

    // add actor to next tile
    struct tile *const next_tile = &next_map->tiles[next_map->stair_up_x][next_map->stair_up_y];
    next_tile->actor = actor;

    // change actor coordinates
    actor->floor = next_map->floor;
    actor->x = next_map->stair_up_x;
    actor->y = next_map->stair_up_y;

    // find all followers and move them to the next map as well
    if (is_leader)
    {
        TCOD_LIST_FOREACH(map->actors)
        {
            struct actor *other = *iterator;
            if (other && other->leader == actor)
            {
                actor_descend(other, false, &iterator);

                if (!iterator)
                {
                    break;
                }
            }
        }
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s descends.",
        actor->name);

    return true;
}

bool actor_ascend(struct actor *actor, bool is_leader, void ***iterator)
{
    // is there a previous map?
    if (actor->floor == 0)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't go any higher.",
            actor->name);

        return false;
    }

    // is the actor on stairs up?
    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];
    if (is_leader &&
        (!tile->object || tile->object->type != OBJECT_TYPE_STAIR_UP))
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't ascend here.",
            actor->name);

        return false;
    }

    // remove actor from current map
    if (iterator)
    {
        *iterator = TCOD_list_remove_iterator_fast(map->actors, *iterator);
    }
    else
    {
        TCOD_list_remove(map->actors, actor);
    }

    // remove actor from current tile
    tile->actor = NULL;

    // add actor to next map
    struct map *const next_map = &world->maps[actor->floor - 1];
    TCOD_list_push(next_map->actors, actor);

    // add actor to next tile
    struct tile *const next_tile = &next_map->tiles[next_map->stair_down_x][next_map->stair_down_y];
    next_tile->actor = actor;

    // change actor coordinates
    actor->floor = next_map->floor;
    actor->x = next_map->stair_down_x;
    actor->y = next_map->stair_down_y;

    // find all followers and move them to the next map as well
    if (is_leader)
    {
        TCOD_LIST_FOREACH(map->actors)
        {
            struct actor *other = *iterator;
            if (other && other->leader == actor)
            {
                actor_ascend(other, false, &iterator);

                if (!iterator)
                {
                    break;
                }
            }
        }
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s ascends.",
        actor->name);

    return true;
}

bool actor_open_chest(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (tile->object && tile->object->type == OBJECT_TYPE_CHEST)
    {
        // TODO: give item

        TCOD_list_remove(map->objects, tile->object);

        object_delete(tile->object);
        tile->object = NULL;

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_orange,
            "%s opens the chest.",
            actor->name);

        return true;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't open the chest.",
        actor->name);

    return false;
}

bool actor_pray(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (tile->object && tile->object->type == OBJECT_TYPE_ALTAR)
    {
        // TODO: prayer effects

        TCOD_list_remove(map->objects, tile->object);

        object_delete(tile->object);
        tile->object = NULL;

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_orange,
            "%s prays at the altar.",
            actor->name);

        return true;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't pray here.",
        actor->name);

    return false;
}

bool actor_drink(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (tile->object && tile->object->type == OBJECT_TYPE_FOUNTAIN)
    {
        if (actor->current_hp == actor->max_hp)
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_white,
                "%s is at full HP.",
                actor->name);

            return false;
        }

        const int hp = actor->max_hp - actor->current_hp;
        actor->current_hp += hp;
        if (actor->current_hp > actor->max_hp)
        {
            actor->current_hp = actor->max_hp;
        }

        TCOD_list_remove(map->objects, tile->object);

        object_delete(tile->object);
        tile->object = NULL;

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_orange,
            "%s drinks from the fountain, restoring %d health.",
            actor->name,
            hp);

        return true;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't drink here.",
        actor->name);

    return false;
}

bool actor_sit(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (tile->object && tile->object->type == OBJECT_TYPE_THRONE)
    {
        // TODO: throne effects
        // donate gold for a reward?

        TCOD_list_remove(map->objects, tile->object);

        object_delete(tile->object);
        tile->object = NULL;

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_orange,
            "%s sits on the throne.",
            actor->name);

        return true;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s can't sit here",
        actor->name);

    return false;
}

bool actor_grab(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *const map = &world->maps[actor->floor];
    struct tile *const tile = &map->tiles[x][y];
    if (TCOD_list_size(tile->items) == 0)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot find anything to pick up.",
            actor->name);

        return false;
    }

    if (TCOD_list_size(actor->items) >= 26)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s is carrying too many items.",
            actor->name);

        return false;
    }

    struct item *const item = TCOD_list_pop(tile->items);
    if (item->type == ITEM_TYPE_GOLD)
    {
        const int gold = item->current_stack;
        actor->gold += gold;

        TCOD_list_remove(map->items, item);

        item_delete(item);

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s picks up %d gold.",
            actor->name,
            gold);
    }
    else
    {
        TCOD_list_remove(map->items, item);

        TCOD_list_push(actor->items, item);

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s picks up %s.",
            actor->name,
            item_data[item->type].name);
    }

    return true;
}

bool actor_drop(struct actor *const actor, struct item *const item)
{
    // remove from actor's inventory
    TCOD_list_remove(actor->items, item);

    // change item coordinates
    item->floor = actor->floor;
    item->x = actor->x;
    item->y = actor->y;

    // add item to map
    struct map *const map = &world->maps[item->floor];
    TCOD_list_push(map->items, item);

    // add item to tile
    struct tile *const tile = &map->tiles[item->x][item->y];
    TCOD_list_push(tile->items, item);

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s drops %s.",
        actor->name,
        item_data[item->type].name);

    return true;
}

bool actor_equip(struct actor *const actor, struct item *const item)
{
    const struct item_datum *const item_datum = &item_data[item->type];

    // check if item is equippable
    enum equip_slot equip_slot = item_datum->equip_slot;
    if (equip_slot == EQUIP_SLOT_NONE)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot equip %s.",
            actor->name,
            item_datum->name);

        return false;
    }

    // unequip the current item in the slot
    if (actor->equipment[equip_slot])
    {
        actor_unequip(actor, equip_slot);
    }

    // if the item being equipped is two handed, also unequip the off hand
    if (item_datum->two_handed)
    {
        if (actor->equipment[EQUIP_SLOT_OFF_HAND])
        {
            actor_unequip(actor, EQUIP_SLOT_OFF_HAND);
        }
    }

    // if the item being equipped is an off hand and the equipped main hand is two handed, also unequip the main hand
    if (equip_slot == EQUIP_SLOT_OFF_HAND)
    {
        const struct item *const main_hand = actor->equipment[EQUIP_SLOT_MAIN_HAND];
        if (main_hand && item_data[main_hand->type].two_handed)
        {
            actor_unequip(actor, EQUIP_SLOT_MAIN_HAND);
        }
    }

    // remove from inventory
    TCOD_list_remove(actor->items, item);

    // add to slot
    actor->equipment[equip_slot] = item;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s equips %s.",
        actor->name,
        item_datum->name);

    return true;
}

bool actor_unequip(struct actor *const actor, const enum equip_slot equip_slot)
{
    // is something equipped in the slot?
    struct item *equipment = actor->equipment[equip_slot];
    if (!equipment)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s is not equipping anything their %s slot.",
            actor->name,
            equip_slot_data[equip_slot].name);

        return false;
    }

    // add to inventory
    TCOD_list_push(actor->items, equipment);

    // remove from slot
    actor->equipment[equip_slot] = NULL;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s unequips %s.",
        actor->name,
        item_data[equipment->type].name);

    return true;
}

bool actor_quaff(struct actor *const actor, struct item *const item)
{
    // is the item quaffable?
    const struct item_datum *const item_datum = &item_data[item->type];
    if (!item_datum->quaffable)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot quaff %s.",
            actor->name,
            item_datum->name);

        return false;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s quaffs %s.",
        actor->name,
        item_datum->name);

    // determine effects
    if (item->type == ITEM_TYPE_HEALING_POTION)
    {
        int hp = actor->max_hp - actor->current_hp;
        actor->current_hp += hp;
        if (actor->current_hp > actor->max_hp)
        {
            actor->current_hp = actor->max_hp;
        }

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s heals for %d.",
            actor->name,
            hp);
    }

    // decrement item stack
    item->current_stack--;

    // delete item if stack is empty
    if (item->current_stack <= 0)
    {
        // remove from inventory
        TCOD_list_remove(actor->items, item);

        // delete the item
        item_delete(item);
    }

    return true;
}

bool actor_bash(struct actor *const actor, struct object *const object)
{
    const struct object_datum *const object_datum = &object_data[object->type];

    // is the object destroyable
    // TODO: make this a property on the object_datum?
    if (object->type == OBJECT_TYPE_STAIR_DOWN ||
        object->type == OBJECT_TYPE_STAIR_UP)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot destroy the %s.",
            actor->name,
            object_datum->name);

        return false;
    }

    // TODO: calculate damage properly
    // this means objects should have health
    // move damage calculation in actor_attack to a function and call it here as well

    // remove from map
    struct map *const map = &world->maps[actor->floor];
    TCOD_list_remove(map->objects, object);

    // remove from tile
    struct tile *const tile = &map->tiles[object->x][object->y];
    tile->object = NULL;

    // delete the object
    object_delete(object);

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s destroys the %s.",
        actor->name,
        object_datum->name);

    return true;
}

bool actor_shoot(
    struct actor *const actor,
    const int x, const int y)
{
    // cant shoot itself!
    if (x == actor->x && y == actor->y)
    {
        return false;
    }

    // does the actor have a weapon?
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];
    if (!weapon)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot shoot without a weapon!",
            actor->name);

        return false;
    }

    // is the weapon ranged?
    const struct item_datum *const weapon_datum = &item_data[weapon->type];
    if (!weapon_datum->ranged)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot shoot without a ranged weapon!",
            actor->name);

        return false;
    }

    // is there ammo?
    struct item *const ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];
    if (!ammunition)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s has no ammunition equipped!",
            actor->name);

        return false;
    }

    // is it the right ammo?
    const struct item_datum *const ammunition_datum = &item_data[ammunition->type];
    if (ammunition_datum->ammunition_type != weapon_datum->ammunition_type)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s has unsuitable ammunition equipped!",
            actor->name);

        return false;
    }

    // create a projectile
    // the projectile gets a copy of the ammunition with the stack size set to 1
    // this is so that if the actor deletes its ammunition item (e.g. if it runs out), the projectile can still use the data
    struct projectile *projectile = projectile_new(
        PROJECTILE_TYPE_ARROW,
        actor->floor,
        actor->x,
        actor->y,
        x,
        y,
        actor,
        item_new(ammunition->type, ammunition->floor, ammunition->x, ammunition->y, 1));

    // add the projectile to the map
    struct map *map = &world->maps[actor->floor];
    TCOD_list_push(map->projectiles, projectile);

    // decrement the actor's ammunition
    ammunition->current_stack--;

    // unequip and delete the item if out of ammo
    if (ammunition->current_stack <= 0)
    {
        // remove from inventory
        TCOD_list_remove(actor->items, ammunition);

        // remove from slot
        actor->equipment[EQUIP_SLOT_AMMUNITION] = NULL;

        // delete the item
        item_delete(ammunition);
    }

    return true;
}

bool actor_attack(struct actor *actor, struct actor *other, struct item *ammunition)
{
    // TODO: better miss calculation
    if (TCOD_random_get_float(world->random, 0, 1) < 0.25f)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s misses %s.",
            actor->name,
            other->name);

        return true;
    }

    // calculate damage, initializing with unarmed damage
    int min_damage = 1;
    int max_damage = 3;

    // if the actor has a weapon, use that weapon's damage
    const struct item *const weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];
    if (weapon)
    {
        const struct item_datum *const weapon_datum = &item_data[weapon->type];
        min_damage = weapon_datum->min_damage;
        max_damage = weapon_datum->max_damage;
    }

    // roll for damage
    int damage = TCOD_random_get_int(
        world->random,
        min_damage,
        max_damage);

    // if the actor has ammunition, add the ammunition's damage roll to the total
    if (ammunition)
    {
        const struct item_datum *const ammunition_datum = &item_data[ammunition->type];
        damage += TCOD_random_get_int(
            world->random,
            ammunition_datum->min_damage,
            ammunition_datum->max_damage);
    }

    // if the other actor has armor, reduce the damage by the armor's value
    const struct item *const other_armor = other->equipment[EQUIP_SLOT_ARMOR];
    if (other_armor)
    {
        const struct item_datum *const other_armor_datum = &item_data[other_armor->type];
        // TODO: armor piercing weapons/ammunition
        damage -= other_armor_datum->armor;
        if (damage < 0)
        {
            damage = 0;
        }
    }

    // if the other actor has a shield, roll for shield block
    // shield block will prevent all damage
    const struct item *const other_shield = other->equipment[EQUIP_SLOT_OFF_HAND];
    if (other_shield)
    {
        const struct item_datum *const other_shield_datum = &item_data[other_shield->type];
        if (TCOD_random_get_float(world->random, 0, 1) <= other_shield_datum->block_chance)
        {
            damage = 0;
        }
    }

    // TODO: when projectiles come at the player from the dark, nothing gets logged
    // it'd be nice if there were a way to do something like "someone attacks <player> for <damage>"
    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s hits %s for %d.",
        actor->name,
        other->name,
        damage);

    const bool killed = actor_take_damage(other, actor, damage);

    // if the other actor wasn't killed, perform any other effects
    if (!killed)
    {
        if (weapon)
        {
            if (weapon->type == ITEM_TYPE_COLD_IRON_BLADE)
            {
                other->energy -= 1.0f;

                world_log(
                    actor->floor,
                    actor->x,
                    actor->y,
                    TCOD_white,
                    "%s has been slowed!",
                    other->name);
            }

            if (weapon->type == ITEM_TYPE_SCEPTER_OF_UNITY)
            {
                other->faction = actor->faction;

                world_log(
                    actor->floor,
                    actor->x,
                    actor->y,
                    TCOD_white,
                    "%s has become friendly to %s!",
                    other->name,
                    actor->name);
            }
        }

        if (other_shield)
        {
            if (other_shield->type == ITEM_TYPE_SPIKED_SHIELD && !ammunition)
            {
                const struct item_datum *const other_shield_datum = &item_data[other_shield->type];
                const int spike_damage = TCOD_random_get_int(
                    world->random,
                    other_shield_datum->min_damage,
                    other_shield_datum->max_damage);

                world_log(
                    actor->floor,
                    actor->x,
                    actor->y,
                    TCOD_white,
                    "%s's shield spike hits %s for %d.",
                    other->name,
                    actor->name,
                    spike_damage);

                actor_take_damage(actor, other, spike_damage);
            }
        }
    }

    return true;
}

bool actor_cast_spell(
    struct actor *const actor,
    const int x, const int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    // TODO: mana and/or some other kind of spell limiter

    const struct spell_datum *const spell_datum = &spell_data[actor->readied_spell];

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s casts %s.",
        actor->name,
        spell_datum->name);

    switch (actor->readied_spell)
    {
    case SPELL_TYPE_HEAL:
    {
        const int hp = actor->max_hp - actor->current_hp;
        actor->current_hp += hp;
        if (actor->current_hp > actor->max_hp)
        {
            actor->current_hp = actor->max_hp;
        }

        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s heals for %d.",
            actor->name,
            hp);
    }
    break;
    case SPELL_TYPE_LIGHTNING:
    {
        struct map *const map = &world->maps[actor->floor];
        struct tile *const tile = &map->tiles[x][y];
        struct actor *const other = tile->actor;
        if (other)
        {
            const int damage = TCOD_random_get_int(world->random, 1, 4);

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_white,
                "%s zaps %s for %d.",
                actor->name,
                other->name,
                damage);

            actor_take_damage(other, actor, damage);
        }
        else
        {
            world_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_white,
                "%s cannot cast %s here.",
                actor->name,
                spell_datum->name);

            return false;
        }
    }
    break;
    case SPELL_TYPE_FIREBALL:
    {
        if (x == actor->x && y == actor->y)
        {
            return false;
        }

        // create a fireball projectile
        struct projectile *const projectile = projectile_new(
            PROJECTILE_TYPE_FIREBALL,
            actor->floor,
            actor->x,
            actor->y,
            x,
            y,
            actor,
            NULL);

        // add the projectile to the map
        struct map *const map = &world->maps[actor->floor];
        TCOD_list_push(map->projectiles, projectile);

        return false;
    }
    break;
    case NUM_SPELL_TYPES:
        break;
    }

    return true;
}

bool actor_take_damage(struct actor *actor, struct actor *attacker, int damage)
{
    actor->current_hp -= damage;
    actor->flash_color = TCOD_red;
    actor->flash_fade_coef = 1.0f;

    if (actor->current_hp <= 0)
    {
        actor->current_hp = 0;

        actor_die(actor, attacker);

        return true;
    }

    return false;
}

void actor_die(struct actor *actor, struct actor *killer)
{
    // remove from map
    struct map *const map = &world->maps[actor->floor];
    TCOD_list_remove(map->actors, actor);

    // remove from tile
    struct tile *const tile = &map->tiles[actor->x][actor->y];
    tile->actor = NULL;

    // create a corpse
    struct corpse *const corpse = corpse_new(actor->name, actor->level, actor->floor, actor->x, actor->y);
    TCOD_list_push(map->corpses, corpse);
    TCOD_list_push(tile->corpses, corpse);

    // drop items
    if (actor != world->player)
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

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_red,
        "%s dies.",
        actor->name);

    // TODO: all actors involved in combat with the dead actor should gain XP
    if (killer)
    {
        int experience = TCOD_random_get_int(world->random, 50, 100) * actor->level;
        actor_give_experience(killer, experience);
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
