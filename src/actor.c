#include <libtcod/libtcod.h>
#include <malloc.h>
#include <math.h>
#include <string.h>

#include "actor.h"
#include "game.h"
#include "item.h"
#include "map.h"
#include "object.h"
#include "projectile.h"
#include "tile.h"
#include "util.h"
#include "window.h"

#include "CMemleak.h"

static int calc_ability_modifier(int ability);

struct actor *actor_create(struct game *game, const char *name, enum race race, enum class class, enum faction faction, int level, int x, int y)
{
    struct actor *actor = malloc(sizeof(struct actor));

    actor->game = game;
    actor->name = strdup(name);
    actor->race = race;
    actor->class = class;
    actor->faction = faction;
    actor->experience = 0;
    actor->strength = 10;
    actor->dexterity = 10;
    actor->constitution = 10;
    actor->intelligence = 10;
    actor->wisdom = 10;
    actor->charisma = 10;
    for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
    {
        actor->equipment[i] = NULL;
    }
    actor->items = TCOD_list_new();
    actor->level = level;
    actor->x = x;
    actor->y = y;
    actor->base_hp = 10; // dependent on class
    actor->current_hp = actor_calc_max_hp(actor);
    actor->energy = 1.0f;
    actor->last_seen_x = -1;
    actor->last_seen_y = -1;
    actor->turns_chased = 0;
    actor->kills = 0;
    actor->glow = false;
    actor->glow_fov = NULL;
    actor->torch = TCOD_random_get_int(NULL, 0, 20) == 0;
    actor->torch_fov = NULL;
    actor->fov = NULL;
    actor->flash_fade = 0;
    actor->dead = false;

    actor_calc_light(actor);
    actor_calc_fov(actor);

    return actor;
}

int actor_calc_max_hp(struct actor *actor)
{
    return actor->base_hp + calc_ability_modifier(actor->constitution);
}

int actor_calc_enhancement_bonus(struct actor *actor)
{
    struct game *game = actor->game;

    int enhancement_bonus = 0;

    for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
    {
        struct item *equipment = actor->equipment[i];

        if (equipment)
        {
            struct item_info *item_info = &game->item_info[equipment->type];

            for (int j = 0; j < NUM_ITEM_PROPERTIES; j++)
            {
                if (item_info->item_properties[j])
                {
                    struct item_property_info *item_property_info = &game->item_property_info[j];

                    enhancement_bonus += item_property_info->enhancement_bonus;
                }
            }
        }
    }

    return enhancement_bonus;
}

int actor_calc_attack_bonus(struct actor *actor)
{
    int base_attack_bonus = 0;

    return base_attack_bonus + calc_ability_modifier(actor->strength) + actor_calc_enhancement_bonus(actor);
}

int actor_calc_armor_class(struct actor *actor)
{
    struct game *game = actor->game;

    int armor_class = 10;

    for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
    {
        struct item *equipment = actor->equipment[i];

        if (equipment)
        {
            struct item_info *item_info = &game->item_info[equipment->type];
            struct base_item_info *base_item_info = &game->base_item_info[item_info->base_item];

            armor_class += base_item_info->base_ac;

            // TODO: deal with stacking AC types
            for (int j = 0; j < NUM_ITEM_PROPERTIES; j++)
            {
                if (item_info->item_properties[j])
                {
                    struct item_property_info *item_property_info = &game->item_property_info[j];

                    armor_class += item_property_info->ac_bonus;
                }
            }
        }
    }

    return armor_class;
}

void actor_calc_weapon(struct actor *actor, int *num_dice, int *die_to_roll, int *crit_threat, int *crit_mult, bool ranged)
{
    struct game *game = actor->game;
    struct item *weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];

    if (ranged)
    {
        *num_dice = 0;
        *die_to_roll = 0;
        *crit_threat = 0;
        *crit_mult = 0;
    }
    else
    {
        *num_dice = 1;
        *die_to_roll = 3;
        *crit_threat = 20;
        *crit_mult = 2;
    }

    if (weapon)
    {
        struct item_info *item_info = &game->item_info[weapon->type];
        struct base_item_info *base_item_info = &game->base_item_info[item_info->base_item];

        if (base_item_info->ranged == ranged)
        {
            *num_dice = base_item_info->num_dice;
            *die_to_roll = base_item_info->die_to_roll;
            *crit_threat = base_item_info->crit_threat;
            *crit_mult = base_item_info->crit_mult;
        }
    }
}

int actor_calc_damage_bonus(struct actor *actor)
{
    return calc_ability_modifier(actor->strength) + actor_calc_enhancement_bonus(actor);
}

void actor_update_flash(struct actor *actor)
{
    if (actor->flash_fade > 0)
    {
        actor->flash_fade -= (1.0f / (float)FPS) * 4.0f;
    }
    else
    {
        actor->flash_fade = 0.0f;
    }
}

void actor_calc_light(struct actor *actor)
{
    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];

    if (actor->glow_fov)
    {
        TCOD_map_delete(actor->glow_fov);

        actor->glow_fov = NULL;
    }

    if (actor->torch_fov)
    {
        TCOD_map_delete(actor->torch_fov);

        actor->torch_fov = NULL;
    }

    if (actor->torch)
    {
        actor->torch_fov = map_to_fov_map(map, actor->x, actor->y, game->actor_common.torch_radius);
    }
    else if (actor->glow)
    {
        actor->glow_fov = map_to_fov_map(map, actor->x, actor->y, game->actor_common.glow_radius);
    }
}

void actor_calc_fov(struct actor *actor)
{
    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];

    if (actor->fov)
    {
        TCOD_map_delete(actor->fov);
    }

    actor->fov = map_to_fov_map(map, actor->x, actor->y, 1);

    TCOD_map_t los_map = map_to_fov_map(map, actor->x, actor->y, 0);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            if (TCOD_map_is_in_fov(los_map, x, y))
            {
                for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
                {
                    struct object *object = *iterator;

                    if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }
                }

                for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
                {
                    struct actor *other = *iterator;

                    if (other->glow_fov && TCOD_map_is_in_fov(other->glow_fov, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov, x, y, true);
                    }

                    if (other->torch_fov && TCOD_map_is_in_fov(other->torch_fov, x, y))
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
    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[actor->x][actor->y];

    if (actor == game->player)
    {
        return;
    }

    actor->energy += game->race_info[actor->race].energy_per_turn;

    while (actor->energy >= 1.0f)
    {
        actor->energy -= 1.0f;

        if (actor->current_hp < actor_calc_max_hp(actor) / 2)
        {
            struct object *target = NULL;
            float min_distance = 1000.0f;

            for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
            {
                struct object *object = *iterator;

                if (TCOD_map_is_in_fov(actor->fov, object->x, object->y) &&
                    object->type == OBJECT_TYPE_FOUNTAIN)
                {
                    float dist = distance(actor->x, actor->y, object->x, object->y);

                    if (dist < min_distance)
                    {
                        target = object;
                        min_distance = dist;
                    }
                }
            }

            if (target)
            {
                if (distance(actor->x, actor->y, target->x, target->y) < 2.0f)
                {
                    if (actor_drink(actor, target->x, target->y))
                    {
                        continue;
                    }
                }
                else
                {
                    if (actor_path_towards(actor, target->x, target->y))
                    {
                        continue;
                    }
                }
            }
        }

        {
            struct actor *target = NULL;
            float min_distance = 1000.0f;

            for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
            {
                struct actor *other = *iterator;

                if (TCOD_map_is_in_fov(actor->fov, other->x, other->y) &&
                    other->faction != actor->faction &&
                    !other->dead)
                {
                    float dist = distance(actor->x, actor->y, other->x, other->y);

                    if (dist < min_distance)
                    {
                        target = other;
                        min_distance = dist;
                    }
                }
            }

            if (target)
            {
                actor->last_seen_x = target->x;
                actor->last_seen_y = target->y;
                actor->turns_chased = 0;

                if (distance(actor->x, actor->y, target->x, target->y) < 2.0f &&
                    actor_attack(actor, target, false))
                {
                    continue;
                }

                struct item *weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];

                if (weapon)
                {
                    struct item_info *item_info = &game->item_info[weapon->type];
                    struct base_item_info *base_item_info = &game->base_item_info[item_info->base_item];

                    if (base_item_info->ranged && actor_shoot(actor, target->x, target->y, NULL, NULL))
                    {
                        continue;
                    }
                }

                if (actor_path_towards(actor, target->x, target->y))
                {
                    continue;
                }
            }
        }

        if (actor->last_seen_x != -1 && actor->last_seen_y != -1)
        {
            if ((actor->x == actor->last_seen_x && actor->y == actor->last_seen_y) ||
                actor->turns_chased > game->actor_common.turns_to_chase)
            {
                actor->last_seen_x = -1;
                actor->last_seen_y = -1;
            }
            else if (actor_path_towards(actor, actor->last_seen_x, actor->last_seen_y))
            {
                actor->turns_chased++;

                continue;
            }
        }

        {
            bool interacted = false;

            for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
            {
                struct object *object = *iterator;

                switch (object->type)
                {
                case OBJECT_TYPE_STAIR_DOWN:
                {
                    if (actor_descend(actor))
                    {
                        interacted = true;
                    }
                }
                break;
                case OBJECT_TYPE_STAIR_UP:
                {
                    if (actor_ascend(actor))
                    {
                        interacted = true;
                    }
                }
                break;
                }

                if (interacted)
                {
                    break;
                }
            }

            if (interacted)
            {
                continue;
            }
        }

        if (TCOD_list_size(tile->items) > 0)
        {
            if (actor_grab(actor, actor->x, actor->y))
            {
                continue;
            }
        }

        if (TCOD_random_get_int(NULL, 0, 1) == 0)
        {
            int x = actor->x + TCOD_random_get_int(NULL, -1, 1);
            int y = actor->y + TCOD_random_get_int(NULL, -1, 1);

            actor_move(actor, x, y);

            continue;
        }
    }
}

bool actor_path_towards(struct actor *actor, int x, int y)
{
    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];

    TCOD_map_t TCOD_map = map_to_TCOD_map(map);
    TCOD_map_set_properties(TCOD_map, x, y, TCOD_map_is_transparent(TCOD_map, x, y), true);

    TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1.0f);
    TCOD_path_compute(path, actor->x, actor->y, x, y);

    bool success = false;

    {
        int next_x, next_y;
        if (!TCOD_path_is_empty(path) && TCOD_path_walk(path, &next_x, &next_y, false))
        {
            success = actor_move(actor, next_x, next_y);
        }
        else
        {
            success = actor_move_towards(actor, x, y);
        }
    }

    TCOD_path_delete(path);

    TCOD_map_delete(TCOD_map);

    return success;
}

bool actor_move_towards(struct actor *actor, int x, int y)
{
    int dx = x - actor->x;
    int dy = y - actor->y;
    float d = distance(actor->x, actor->y, x, y);

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

    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[x][y];
    struct tile_info *tile_info = &game->tile_info[tile->type];

    if (!tile_info->is_walkable)
    {
        return false;
    }

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        switch (object->type)
        {
        case OBJECT_TYPE_ALTAR:
        {
            return actor_pray(actor, x, y);
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
        }

        if (!actor->game->object_info[object->type].is_walkable)
        {
            return false;
        }
    }

    for (void **iterator = TCOD_list_begin(tile->actors); iterator != TCOD_list_end(tile->actors); iterator++)
    {
        struct actor *other = *iterator;

        if (other == actor)
        {
            continue;
        }

        if (other->dead)
        {
            continue;
        }

        if (other->faction == actor->faction)
        {
            return actor_swap(actor, other);
        }
        else
        {
            return actor_attack(actor, other, false);
        }
    }

    struct tile *current_tile = &map->tiles[actor->x][actor->y];

    TCOD_list_remove(current_tile->actors, actor);

    actor->x = x;
    actor->y = y;

    TCOD_list_push(tile->actors, actor);

    for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
    {
        struct item *equipment = actor->equipment[i];

        if (equipment)
        {
            equipment->x = actor->x;
            equipment->y = actor->y;
        }
    }

    for (void **iterator = TCOD_list_begin(actor->items); iterator != TCOD_list_end(actor->items); iterator++)
    {
        struct item *item = *iterator;

        item->x = actor->x;
        item->y = actor->y;
    }

    return true;
}

bool actor_swap(struct actor *actor, struct actor *other)
{
    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];

    if (other == game->player)
    {
        return false;
    }

    struct tile *tile = &map->tiles[actor->x][actor->y];
    struct tile *other_tile = &map->tiles[other->x][other->y];

    TCOD_list_remove(tile->actors, actor);
    TCOD_list_remove(other_tile->actors, other);

    int temp_x = actor->x;
    int temp_y = actor->y;

    actor->x = other->x;
    actor->y = other->y;

    other->x = temp_x;
    other->y = temp_y;

    TCOD_list_push(other_tile->actors, actor);
    TCOD_list_push(tile->actors, other);

    for (void **iterator = TCOD_list_begin(actor->items); iterator != TCOD_list_end(actor->items); iterator++)
    {
        struct item *item = *iterator;

        item->x = actor->x;
        item->y = actor->y;
    }

    for (void **iterator = TCOD_list_begin(other->items); iterator != TCOD_list_end(other->items); iterator++)
    {
        struct item *item = *iterator;

        item->x = other->x;
        item->y = other->y;
    }

    game_log(
        game,
        actor->level,
        actor->x,
        actor->y,
        TCOD_white,
        "%s swaps with %s",
        actor->name,
        other->name);

    return true;
}

bool actor_interact(struct actor *actor, int x, int y, enum action action)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    switch (action)
    {
    case ACTION_DESCEND:
        return actor_descend(actor);
    case ACTION_ASCEND:
        return actor_ascend(actor);
    case ACTION_CLOSE_DOOR:
        return actor_close_door(actor, x, y);
    case ACTION_OPEN_DOOR:
        return actor_open_door(actor, x, y);
    case ACTION_PRAY:
        return actor_pray(actor, x, y);
    case ACTION_DRINK:
        return actor_drink(actor, x, y);
    case ACTION_SIT:
        return actor_sit(actor, x, y);
    }

    return false;
}

bool actor_open_door(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[x][y];

    bool success = false;

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (object->type == OBJECT_TYPE_DOOR_CLOSED)
        {
            success = true;

            object->type = OBJECT_TYPE_DOOR_OPEN;

            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_white,
                "%s opens the door",
                actor->name);

            break;
        }
    }

    if (!success)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't open that",
            actor->name);
    }

    return success;
}

bool actor_close_door(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[x][y];

    bool success = false;

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (object->type == OBJECT_TYPE_DOOR_OPEN)
        {
            success = true;

            object->type = OBJECT_TYPE_DOOR_CLOSED;

            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_white,
                "%s closes the door",
                actor->name);

            break;
        }
    }

    if (!success)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't close that",
            actor->name);
    }

    return success;
}

bool actor_descend(struct actor *actor)
{
    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[actor->x][actor->y];

    if (actor->level >= NUM_MAPS)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s has reached the end",
            actor->name);

        return false;
    }

    bool success = false;

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (object->type == OBJECT_TYPE_STAIR_DOWN)
        {
            success = true;

            struct map *next_map = &game->maps[actor->level + 1];
            struct tile *next_tile = &next_map->tiles[next_map->stair_up_x][next_map->stair_up_y];

            TCOD_list_remove(map->actors, actor);
            TCOD_list_remove(tile->actors, actor);

            actor->level++;
            actor->x = next_map->stair_up_x;
            actor->y = next_map->stair_up_y;

            TCOD_list_push(next_map->actors, actor);
            TCOD_list_push(next_tile->actors, actor);

            for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
            {
                struct item *equipment = actor->equipment[i];

                if (equipment)
                {
                    equipment->level = actor->level;
                    equipment->x = actor->x;
                    equipment->y = actor->y;

                    TCOD_list_remove(map->items, equipment);
                    TCOD_list_push(next_map->items, equipment);
                }
            }

            for (void **iterator2 = TCOD_list_begin(actor->items); iterator2 != TCOD_list_end(actor->items); iterator2++)
            {
                struct item *item = *iterator2;

                item->level = actor->level;
                item->x = actor->x;
                item->y = actor->y;

                TCOD_list_remove(map->items, item);
                TCOD_list_push(next_map->items, item);
            }

            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_white,
                "%s descends",
                actor->name);

            break;
        }
    }

    if (!success)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't descend here",
            actor->name);
    }

    return success;
}

bool actor_ascend(struct actor *actor)
{
    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[actor->x][actor->y];

    if (actor->level == 0)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't go any higher",
            actor->name);

        return false;
    }

    bool success = false;

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (object->type == OBJECT_TYPE_STAIR_UP)
        {
            success = true;

            struct map *next_map = &game->maps[actor->level - 1];
            struct tile *next_tile = &next_map->tiles[next_map->stair_up_x][next_map->stair_up_y];

            TCOD_list_remove(map->actors, actor);
            TCOD_list_remove(tile->actors, actor);

            actor->level--;
            actor->x = next_map->stair_down_x;
            actor->y = next_map->stair_down_y;

            TCOD_list_push(next_map->actors, actor);
            TCOD_list_push(next_tile->actors, actor);

            for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
            {
                struct item *equipment = actor->equipment[i];

                if (equipment)
                {
                    equipment->level = actor->level;
                    equipment->x = actor->x;
                    equipment->y = actor->y;

                    TCOD_list_remove(map->items, equipment);
                    TCOD_list_push(next_map->items, equipment);
                }
            }

            for (void **iterator2 = TCOD_list_begin(actor->items); iterator2 != TCOD_list_end(actor->items); iterator2++)
            {
                struct item *item = *iterator2;

                item->level = actor->level;
                item->x = actor->x;
                item->y = actor->y;

                TCOD_list_remove(map->items, item);
                TCOD_list_push(next_map->items, item);
            }

            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_white,
                "%s descends",
                actor->name);

            break;
        }
    }

    if (!success)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't ascend here",
            actor->name);
    }

    return success;
}

bool actor_pray(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[x][y];

    bool success = false;

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (object->type == OBJECT_TYPE_ALTAR)
        {
            success = true;

            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_white,
                "%s prays at the altar",
                actor->name);

            break;
        }
    }

    if (!success)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't pray here",
            actor->name);
    }

    return success;
}

bool actor_drink(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[x][y];

    bool success = false;

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (object->type == OBJECT_TYPE_FOUNTAIN)
        {
            success = true;

            int hp = roll(1, 4);
            int max_hp = actor_calc_max_hp(actor);

            actor->current_hp += hp;

            if (actor->current_hp > max_hp)
            {
                actor->current_hp = max_hp;
            }

            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_white,
                "%s drinks from the fountain, restoring %d health",
                actor->name,
                hp);

            break;
        }
    }

    if (!success)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't drink here",
            actor->name);
    }

    return success;
}

bool actor_sit(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[x][y];

    bool success = false;

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (object->type == OBJECT_TYPE_THRONE)
        {
            success = true;

            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_white,
                "%s sits on the throne",
                actor->name);

            break;
        }
    }

    if (!success)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s can't sit here",
            actor->name);
    }

    return success;
}

bool actor_grab(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[x][y];

    if (TCOD_list_size(tile->items) == 0)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot find anything to pick up",
            actor->name);

        return false;
    }

    struct item *item = TCOD_list_pop(tile->items);

    item->level = actor->level;
    item->x = actor->x;
    item->y = actor->y;

    TCOD_list_push(actor->items, item);

    game_log(
        game,
        actor->level,
        actor->x,
        actor->y,
        TCOD_white,
        "%s picks up %s",
        actor->name,
        game->item_info[item->type].name);

    return true;
}

bool actor_drop(struct actor *actor, struct item *item)
{
    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[actor->x][actor->y];

    item->level = actor->level;
    item->x = actor->x;
    item->y = actor->y;

    TCOD_list_push(tile->items, item);
    TCOD_list_remove(actor->items, item);

    game_log(
        game,
        actor->level,
        actor->x,
        actor->y,
        TCOD_white,
        "%s drops %s",
        actor->name,
        game->item_info[item->type].name);

    return true;
}

bool actor_equip(struct actor *actor, struct item *item)
{
    struct game *game = actor->game;
    struct item_info *item_info = &game->item_info[item->type];
    struct base_item_info *base_item_info = &game->base_item_info[item_info->base_item];
    struct item *equipment = actor->equipment[base_item_info->equip_slot];

    if (equipment)
    {
        TCOD_list_push(actor->items, equipment);
        actor->equipment[base_item_info->equip_slot] = NULL;
    }

    TCOD_list_remove(actor->items, item);
    actor->equipment[base_item_info->equip_slot] = item;

    game_log(
        game,
        actor->level,
        actor->x,
        actor->y,
        TCOD_white,
        "%s equips %s",
        actor->name,
        item_info->name);

    return true;
}

bool actor_bash(struct actor *actor, struct object *object)
{
    struct game *game = actor->game;

    object->destroyed = true;

    game_log(
        game,
        actor->level,
        actor->x,
        actor->y,
        TCOD_white,
        "%s destroys the %s",
        actor->name,
        game->object_info[object->type].name);

    return true;
}

bool actor_swing(struct actor *actor, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[x][y];

    bool hit = false;

    for (void **iterator = TCOD_list_begin(tile->actors); iterator != TCOD_list_end(tile->actors); iterator++)
    {
        struct actor *other = *iterator;

        if (other == actor)
        {
            continue;
        }

        hit = true;

        if (actor_attack(actor, other, false))
        {
            return true;
        }
    }

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        hit = true;

        if (actor_bash(actor, object))
        {
            return true;
        }
    }

    if (!hit)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s swings at the air!",
            actor->name);
    }

    return true;
}

bool actor_shoot(struct actor *actor, int x, int y, void (*on_hit)(void *on_hit_params), void *on_hit_params)
{
    struct game *game = actor->game;

    if (x == actor->x && y == actor->y)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot shoot themselves!",
            actor->name);

        return false;
    }

    struct map *map = &game->maps[actor->level];
    struct item *weapon = actor->equipment[EQUIP_SLOT_MAIN_HAND];

    if (!weapon)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot shoot without a weapon!",
            actor->name);

        return false;
    }

    struct item_info *item_info = &game->item_info[weapon->type];
    struct base_item_info *base_item_info = &game->base_item_info[item_info->base_item];

    if (!base_item_info->ranged)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot shoot without a ranged weapon!",
            actor->name);

        return false;
    }

    struct projectile *projectile = projectile_create(
        game,
        '`',
        actor->level,
        actor->x,
        actor->y,
        x,
        y,
        actor,
        on_hit,
        on_hit_params);

    TCOD_list_push(map->projectiles, projectile);

    return true;
}

bool actor_attack(struct actor *actor, struct actor *other, bool ranged)
{
    struct game *game = actor->game;

    if (other->dead)
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s cannot attack that!",
            actor->name);

        return false;
    }

    int attack_roll = roll(1, 20);
    int attack_bonus = actor_calc_attack_bonus(actor);
    int total_attack = attack_roll + attack_bonus;
    int armor_class = actor_calc_armor_class(other);
    bool hit = attack_roll == 1
                   ? false
                   : attack_roll == 20
                         ? true
                         : total_attack >= armor_class;

    if (hit)
    {
        int num_dice;
        int die_to_roll;
        int crit_threat;
        int crit_mult;
        actor_calc_weapon(actor, &num_dice, &die_to_roll, &crit_threat, &crit_mult, ranged);

        int damage_rolls = 1;

        bool crit = false;
        if (attack_roll >= crit_threat)
        {
            int threat_roll = roll(1, 20);
            int total_threat = threat_roll + attack_bonus;

            if (total_threat >= armor_class)
            {
                crit = true;
                damage_rolls *= crit_mult;
            }
        }

        int total_damage = 0;
        int damage_bonus = actor_calc_damage_bonus(actor);
        for (int i = 0; i < damage_rolls; i++)
        {
            int damage_roll = roll(num_dice, die_to_roll);
            int damage = damage_roll + damage_bonus;

            total_damage += damage;
        }

        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            crit ? TCOD_yellow : TCOD_white,
            "%s %s %s for %d",
            actor->name,
            crit ? "crits" : "hits",
            other->name,
            total_damage);

        other->current_hp -= total_damage;
        other->flash_color = TCOD_red;
        other->flash_fade = 1.0f;

        if (other->current_hp <= 0)
        {
            other->current_hp = 0;

            actor_die(other, actor);
        }
    }
    else
    {
        switch (TCOD_random_get_int(NULL, 0, 2))
        {
        case 0:
        {
            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_grey,
                "%s is parried by %s",
                actor->name,
                other->name);
        }
        break;
        case 1:
        {
            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_grey,
                "%s is blocked by %s",
                actor->name,
                other->name);
        }
        break;
        case 2:
        {
            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_grey,
                "%s misses %s",
                actor->name,
                other->name);
        }
        break;
        }
    }

    return true;
}

bool actor_cast_spell(struct actor *actor)
{
    (void)actor;

    return false;
}

void actor_die(struct actor *actor, struct actor *killer)
{
    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];
    struct tile *tile = &map->tiles[actor->x][actor->y];

    actor->dead = true;
    actor->glow = false;
    actor->torch = false;

    for (int i = 0; i < NUM_EQUIP_SLOTS; i++)
    {
        struct item *equipment = actor->equipment[i];

        if (equipment)
        {
            TCOD_list_push(actor->items, equipment);
            actor->equipment[i] = NULL;
        }
    }

    if (actor != game->player)
    {
        for (void **iterator = TCOD_list_begin(actor->items); iterator != TCOD_list_end(actor->items); iterator++)
        {
            struct item *item = *iterator;

            TCOD_list_push(tile->items, item);

            iterator = TCOD_list_remove_iterator(actor->items, iterator);
        }
    }

    game_log(
        game,
        actor->level,
        actor->x,
        actor->y,
        TCOD_red,
        "%s dies",
        actor->name);

    if (killer)
    {
        int experience = TCOD_random_get_int(NULL, 50, 100);

        killer->experience += experience;
        killer->kills++;

        game_log(
            game,
            killer->level,
            killer->x,
            killer->y,
            TCOD_azure,
            "%s gains %d experience",
            killer->name,
            experience);
    }

    if (actor == game->player)
    {
        game->game_over = true;

        TCOD_sys_delete_file(SAVE_PATH);

        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_green,
            "Game over! Press 'r' to restart");
    }
}

void actor_destroy(struct actor *actor)
{
    if (actor->fov != NULL)
    {
        TCOD_map_delete(actor->fov);
    }

    if (actor->glow_fov != NULL)
    {
        TCOD_map_delete(actor->glow_fov);
    }

    if (actor->torch_fov != NULL)
    {
        TCOD_map_delete(actor->torch_fov);
    }

    TCOD_list_delete(actor->items);

    free(actor->name);

    free(actor);
}

static int calc_ability_modifier(int ability)
{
    return (ability - 10) / 2;
}
