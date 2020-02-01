#include "actor.h"

#include <assert.h>
#include <float.h>
#include <malloc.h>
#include <math.h>

#include "assets.h"
#include "item.h"
#include "projectile.h"
#include "room.h"
#include "util.h"
#include "world.h"

// TODO: actors should ascend/descend with their leader

struct actor *actor_new(const char *name, enum race race, enum class class, enum faction faction, int level, int floor, int x, int y)
{
    struct actor *actor = malloc(sizeof(struct actor));
    assert(actor);
    actor->name = TCOD_strdup(name);
    actor->race = race;
    actor->class = class;
    actor->faction = faction;
    actor->level = level;
    actor->experience = actor_calc_experience_to_level(actor->level);
    actor->max_hp = 10 * level;
    actor->current_hp = actor->max_hp;
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
    actor->energy_per_turn = TCOD_random_get_float(world->random, 0.5f, 2.0f);
    actor->last_seen_x = -1;
    actor->last_seen_y = -1;
    actor->turns_chased = 0;
    actor->leader = NULL;
    actor->light_radius = -1;
    actor->light_color = TCOD_white;
    actor->light_intensity = 0;
    actor->light_flicker = false;
    actor->light_fov = NULL;
    if (TCOD_random_get_int(world->random, 0, 20) == 0)
    {
        actor->light_radius = actor_common.torch_radius;
        actor->light_color = actor_common.torch_color;
        actor->light_intensity = actor_common.torch_intensity;
        actor->light_flicker = true;
    }
    actor->flash_color = TCOD_white;
    actor->flash_fade_coef = 0.0f;
    actor->controllable = false;
    actor->dead = false;
    return actor;
}

void actor_delete(struct actor *actor)
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

int actor_calc_experience_to_level(int level)
{
    return level * (level - 1) / 2 * 1000;
}

void actor_update_flash(struct actor *actor, float delta_time)
{
    if (actor->flash_fade_coef > 0)
    {
        // TODO: slower/faster fade depending on circumstances
        // TODO: different fade functions such as sin
        actor->flash_fade_coef -= 4.0f * delta_time;
    }
    else
    {
        actor->flash_fade_coef = 0.0f;
    }
}

void actor_calc_light(struct actor *actor)
{
    if (actor->light_fov)
    {
        TCOD_map_delete(actor->light_fov);
        actor->light_fov = NULL;
    }

    if (actor->light_radius >= 0)
    {
        struct map *map = &world->maps[actor->floor];
        actor->light_fov = map_to_fov_map(map, actor->x, actor->y, actor->light_radius);
    }
}

void actor_calc_fov(struct actor *actor)
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
                    struct object *object = *iterator;
                    if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }
                TCOD_LIST_FOREACH(map->actors)
                {
                    struct actor *other = *iterator;
                    if (other->light_fov && TCOD_map_is_in_fov(other->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }
            }
        }
    }
    TCOD_map_delete(los_map);
}

void actor_ai(struct actor *actor)
{
    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];

    // look for fountains to heal if low health
    if (actor->current_hp < actor->max_hp / 2)
    {
        struct object *target = NULL;
        float min_distance = FLT_MAX;
        TCOD_LIST_FOREACH(map->objects)
        {
            struct object *object = *iterator;
            if (TCOD_map_is_in_fov(actor->fov, object->x, object->y) &&
                object->type == OBJECT_TYPE_FOUNTAIN)
            {
                float distance = distance_between_sq(actor->x, actor->y, object->x, object->y);
                if (distance < min_distance)
                {
                    target = object;
                    min_distance = distance;
                }
            }
        }
        if (target)
        {
            if (distance_between(actor->x, actor->y, target->x, target->y) < 2.0f)
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
            struct actor *other = *iterator;
            if (!other->dead &&
                TCOD_map_is_in_fov(actor->fov, other->x, other->y) &&
                other->faction != actor->faction)
            {
                float distance = distance_between_sq(actor->x, actor->y, other->x, other->y);
                if (distance < min_distance)
                {
                    target = other;
                    min_distance = distance;
                }
            }
        }
        if (target)
        {
            actor->last_seen_x = target->x;
            actor->last_seen_y = target->y;
            actor->turns_chased = 0;

            bool ranged = false;
            struct item *weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];
            if (weapon)
            {
                struct item_datum item_datum = item_data[weapon->type];
                if (item_datum.ranged)
                {
                    // wielding a ranged weapon, but need to make sure I have ammo slotted
                    struct item *ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];
                    if (ammunition)
                    {
                        ranged = true;
                    }
                    else
                    {
                        // out of ammo! might as well unequip this weapon
                        // TODO: look in inventory to find suitable ammunition and equip it
                        // if not, look for another weapon to equip
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
                    return;
                }
            }
            else
            {
                if (distance_between(actor->x, actor->y, target->x, target->y) < 2.0f)
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

    // TODO: look for objects to interact with if needed

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
        int x = actor->x + TCOD_random_get_int(world->random, -1, 1);
        int y = actor->y + TCOD_random_get_int(world->random, -1, 1);
        actor_move(actor, x, y);

        goto done;
    }
done:;
    actor->took_turn = true;
}

void actor_give_experience(struct actor *actor, int experience)
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

void actor_level_up(struct actor *actor)
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

bool actor_path_towards(struct actor *actor, int x, int y)
{
    // TODO: cache paths someehow if the map hasn't changed?
    bool success = false;
    struct map *map = &world->maps[actor->floor];
    TCOD_map_t TCOD_map = map_to_TCOD_map(map);
    TCOD_map_set_properties(TCOD_map, x, y, TCOD_map_is_transparent(TCOD_map, x, y), true);
    TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1.0f);
    TCOD_path_compute(path, actor->x, actor->y, x, y);
    int next_x, next_y;
    if (!TCOD_path_is_empty(path) && TCOD_path_walk(path, &next_x, &next_y, false))
    {
        success = actor_move(actor, next_x, next_y);
    }
    else
    {
        success = actor_move_towards(actor, x, y);
    }
    TCOD_path_delete(path);
    TCOD_map_delete(TCOD_map);
    return success;
}

bool actor_move_towards(struct actor *actor, int x, int y)
{
    int dx = x - actor->x;
    int dy = y - actor->y;
    float d = distance_between(actor->x, actor->y, x, y);
    if (d > 0.0f)
    {
        dx = (int)round(dx / d);
        dy = (int)round(dy / d);
        return actor_move(actor, actor->x + dx, actor->y + dy);
    }
    return false;
}

bool actor_move(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];
    if (!tile_data[tile->type].is_walkable)
    {
        return false;
    }
    if (tile->object)
    {
        switch (tile->object->type)
        {
        case OBJECT_TYPE_ALTAR:
        {
            return actor_pray(actor, x, y);
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
        case OBJECT_TYPE_FOUNTAIN:
        {
            return actor_drink(actor, x, y);
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
            tile->object->destroyed = true;

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_white,
                "%s triggers a trap!",
                actor->name);
        }
        break;
        default:
            break;
        }

        if (!object_data[tile->object->type].is_walkable)
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
            struct item *weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];
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

    struct tile *previous_tile = &map->tiles[actor->x][actor->y];
    previous_tile->actor = NULL;
    struct tile *next_tile = &map->tiles[x][y];
    next_tile->actor = actor;
    actor->x = x;
    actor->y = y;
    return true;
}

bool actor_swap(struct actor *actor, struct actor *other)
{
    if (other == world->player)
    {
        // actors can't intiiate a swap with the player
        return false;
    }
    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];
    struct tile *other_tile = &map->tiles[other->x][other->y];
    tile->actor = NULL;
    other_tile->actor = NULL;
    int temp_x = actor->x;
    int temp_y = actor->y;
    actor->x = other->x;
    actor->y = other->y;
    other->x = temp_x;
    other->y = temp_y;
    other_tile->actor = actor;
    tile->actor = other;

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

bool actor_open_door(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];
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

bool actor_close_door(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];
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

bool actor_descend(struct actor *actor, bool with_leader, void ***iterator)
{
    if (actor->floor >= NUM_MAPS)
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

    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];
    if (!with_leader && (!tile->object || tile->object->type != OBJECT_TYPE_STAIR_DOWN))
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

    struct map *next_map = &world->maps[actor->floor + 1];
    struct tile *next_tile = &next_map->tiles[next_map->stair_up_x][next_map->stair_up_y];
    if (iterator)
    {
        *iterator = TCOD_list_remove_iterator(map->actors, *iterator);
    }
    else
    {
        TCOD_list_remove(map->actors, actor);
    }
    tile->actor = NULL;
    actor->floor++;
    actor->x = next_map->stair_up_x;
    actor->y = next_map->stair_up_y;
    TCOD_list_push(next_map->actors, actor);
    next_tile->actor = actor;

    if (!with_leader)
    {
        TCOD_LIST_FOREACH(map->actors)
        {
            struct actor *other = *iterator;
            if (other && other->leader == actor)
            {
                actor_descend(other, true, &iterator);
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

bool actor_ascend(struct actor *actor, bool with_leader, void ***iterator)
{
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

    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];
    if (!with_leader && (!tile->object || tile->object->type != OBJECT_TYPE_STAIR_UP))
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

    struct map *next_map = &world->maps[actor->floor - 1];
    struct tile *next_tile = &next_map->tiles[next_map->stair_up_x][next_map->stair_up_y];
    if (iterator)
    {
        *iterator = TCOD_list_remove_iterator(map->actors, *iterator);
    }
    else
    {
        TCOD_list_remove(map->actors, actor);
    }
    tile->actor = NULL;
    actor->floor--;
    actor->x = next_map->stair_down_x;
    actor->y = next_map->stair_down_y;
    TCOD_list_push(next_map->actors, actor);
    next_tile->actor = actor;

    if (!with_leader)
    {
        TCOD_LIST_FOREACH(map->actors)
        {
            struct actor *other = *iterator;
            if (other && other->leader == actor)
            {
                actor_ascend(other, true, &iterator);
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

bool actor_open_chest(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];
    if (tile->object && tile->object->type == OBJECT_TYPE_CHEST)
    {
        // TODO: give item
        tile->object->destroyed = true;

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

bool actor_pray(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];
    if (tile->object && tile->object->type == OBJECT_TYPE_ALTAR)
    {
        tile->object->destroyed = true;

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

bool actor_drink(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];
    if (tile->object && tile->object->type == OBJECT_TYPE_FOUNTAIN)
    {
        if (actor->current_hp == actor->max_hp)
        {
            return false;
        }
        int hp = actor->max_hp - actor->current_hp;
        actor->current_hp += hp;
        if (actor->current_hp > actor->max_hp)
        {
            actor->current_hp = actor->max_hp;
        }

        tile->object->destroyed = true;

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

bool actor_sit(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];
    if (tile->object && tile->object->type == OBJECT_TYPE_THRONE)
    {
        tile->object->destroyed = true;

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

bool actor_grab(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[x][y];
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

    struct item *item = TCOD_list_pop(tile->items);
    item->floor = actor->floor;
    item->x = actor->x;
    item->y = actor->y;
    TCOD_list_push(actor->items, item);
    TCOD_list_remove(map->items, item);

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s picks up %s.",
        actor->name,
        item_data[item->type].name);

    return true;
}

bool actor_drop(struct actor *actor, struct item *item)
{
    struct map *map = &world->maps[actor->floor];
    struct tile *tile = &map->tiles[actor->x][actor->y];
    item->floor = actor->floor;
    item->x = actor->x;
    item->y = actor->y;
    TCOD_list_push(tile->items, item);
    TCOD_list_push(map->items, item);
    TCOD_list_remove(actor->items, item);

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

bool actor_equip(struct actor *actor, struct item *item)
{
    struct item_datum item_datum = item_data[item->type];
    enum equip_slot equip_slot = item_datum.equip_slot;
    if (equip_slot == EQUIP_SLOT_NONE)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot equip %s.",
            actor->name,
            item_datum.name);

        return false;
    }

    if (actor->equipment[equip_slot])
    {
        actor_unequip(actor, equip_slot);
    }
    if (item_datum.two_handed)
    {
        if (actor->equipment[EQUIP_SLOT_OFF_HAND])
        {
            actor_unequip(actor, EQUIP_SLOT_OFF_HAND);
        }
    }
    if (equip_slot == EQUIP_SLOT_OFF_HAND)
    {
        struct item *main_hand = actor->equipment[EQUIP_SLOT_MAIN_HAND];
        if (main_hand && item_data[main_hand->type].two_handed)
        {
            actor_unequip(actor, EQUIP_SLOT_MAIN_HAND);
        }
    }
    TCOD_list_remove(actor->items, item);
    actor->equipment[equip_slot] = item;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s equips %s.",
        actor->name,
        item_datum.name);

    return true;
}

bool actor_unequip(struct actor *actor, enum equip_slot equip_slot)
{
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

    TCOD_list_push(actor->items, equipment);
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

bool actor_quaff(struct actor *actor, struct item *item)
{
    struct item_datum item_datum = item_data[item->type];
    if (!item_datum.quaffable)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot quaff %s.",
            actor->name,
            item_datum.name);

        return false;
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s quaffs %s.",
        actor->name,
        item_datum.name);

    switch (item->type)
    {
    case ITEM_TYPE_HEALING_POTION:
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
    break;
    default:
        break;
    }

    item->current_stack--;
    if (item->current_stack <= 0)
    {
        TCOD_list_remove(actor->items, item);
        item_delete(item);
    }

    return true;
}

bool actor_bash(struct actor *actor, struct object *object)
{
    struct object_datum object_datum = object_data[object->type];
    if (object->type == OBJECT_TYPE_STAIR_DOWN || object->type == OBJECT_TYPE_STAIR_UP)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot destroy the %s.",
            actor->name,
            object_datum.name);

        return false;
    }

    // TODO: calculate damage properly
    // this means objects should have health
    // move damage calculation in actor_attack to a function and call it here as well

    object->destroyed = true;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s destroys the %s.",
        actor->name,
        object_datum.name);

    return true;
}

bool actor_shoot(struct actor *actor, int x, int y)
{
    if (x == actor->x && y == actor->y)
    {
        return false;
    }

    struct item *weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];
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

    struct item_datum item_datum = item_data[weapon->type];
    if (!item_datum.ranged)
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

    struct item *ammunition = actor->equipment[EQUIP_SLOT_AMMUNITION];
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
    if (item_data[ammunition->type].ammunition_type != item_datum.ammunition_type)
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
    ammunition->current_stack--;
    if (ammunition->current_stack <= 0)
    {
        TCOD_list_remove(actor->items, ammunition);
        actor->equipment[EQUIP_SLOT_AMMUNITION] = NULL;
        item_delete(ammunition);
    }

    struct projectile *projectile = projectile_new(
        PROJECTILE_TYPE_ARROW,
        actor->floor,
        actor->x,
        actor->y,
        x,
        y,
        actor,
        ammunition);
    struct map *map = &world->maps[actor->floor];
    TCOD_list_push(map->projectiles, projectile);

    actor->energy -= 1.0f;

    return true;
}

bool actor_attack(struct actor *actor, struct actor *other, struct item *ammunition)
{
    int min_damage = 1;
    int max_damage = 3;
    struct item *weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];
    if (weapon)
    {
        struct item_datum item_datum = item_data[weapon->type];
        min_damage = item_datum.min_damage;
        max_damage = item_datum.max_damage;
    }
    int damage = TCOD_random_get_int(world->random, min_damage, max_damage);
    struct item *armor = other->equipment[EQUIP_SLOT_ARMOR];
    if (armor)
    {
        struct item_datum item_datum = item_data[armor->type];
        // TODO: armor piercing weapons/ammunition
        damage -= item_datum.armor;
        if (damage < 0)
        {
            damage = 0;
        }
    }
    struct item *shield = other->equipment[EQUIP_SLOT_OFF_HAND];
    if (shield)
    {
        struct item_datum item_datum = item_data[shield->type];
        if (TCOD_random_get_float(world->random, 0.0f, 1.0f) <= item_datum.block_chance)
        {
            damage = 0;
        }
    }

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s hits %s for %d.",
        actor->name,
        other->name,
        damage);

    actor_take_damage(other, actor, damage);

    return true;
}

bool actor_cast_spell(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    // TODO: mana and/or some other kind of spell limiter

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_white,
        "%s casts %s.",
        actor->name,
        spell_data[actor->readied_spell].name);

    switch (actor->readied_spell)
    {
    case SPELL_TYPE_HEAL:
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
    break;
    case SPELL_TYPE_LIGHTNING:
    {
        struct map *map = &world->maps[actor->floor];
        struct tile *tile = &map->tiles[x][y];
        struct actor *other = tile->actor;
        if (other)
        {
            int damage = TCOD_random_get_int(world->random, 1, 4);

            world_log(
                actor->floor,
                actor->x,
                actor->y,
                TCOD_white,
                "%s zaps %s for %d.",
                actor->name,
                other->name,
                damage);

            other->current_hp -= damage;
            other->flash_color = TCOD_red;
            other->flash_fade_coef = 1.0f;
            if (other->current_hp <= 0)
            {
                actor_die(other, actor);
            }
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
                spell_data[actor->readied_spell].name);

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

        struct projectile *projectile = projectile_new(
            PROJECTILE_TYPE_FIREBALL,
            actor->floor,
            actor->x,
            actor->y,
            x,
            y,
            actor,
            NULL);
        struct map *map = &world->maps[actor->floor];
        TCOD_list_push(map->projectiles, projectile);
    }
    break;
    default:
        break;
    }

    return true;
}

void actor_take_damage(struct actor *actor, struct actor *attacker, int damage)
{
    actor->current_hp -= damage;
    actor->flash_color = TCOD_red;
    actor->flash_fade_coef = 1.0f;
    if (actor->current_hp <= 0)
    {
        actor_die(actor, attacker);
    }
}

void actor_die(struct actor *actor, struct actor *killer)
{
    actor->dead = true;

    world_log(
        actor->floor,
        actor->x,
        actor->y,
        TCOD_red,
        "%s dies.",
        actor->name);

    if (killer)
    {
        int experience = TCOD_random_get_int(world->random, 50, 100) * actor->level;
        actor_give_experience(killer, experience);
    }

    if (actor == world->hero)
    {
        world_log(
            actor->floor,
            actor->x,
            actor->y,
            TCOD_green,
            "Game over! Press 'ESC' to return to the menu.");
    }
}
