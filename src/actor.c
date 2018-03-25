#include <libtcod/libtcod.h>
#include <malloc.h>
#include <math.h>

#include "actor.h"
#include "game.h"
#include "item.h"
#include "map.h"
#include "util.h"

struct actor *actor_create(struct game *game, enum race race, enum class class, enum faction faction, int level, int x, int y)
{
    struct actor *actor = malloc(sizeof(struct actor));

    actor->game = game;
    actor->race = race;
    actor->class = class;
    actor->faction = faction;
    actor->level = level;
    actor->x = x;
    actor->y = y;
    actor->health = 20;
    actor->glow = false;
    actor->glow_fov = NULL;
    actor->torch = false;
    actor->torch_fov = NULL;
    actor->fov = NULL;
    actor->items = TCOD_list_new();

    return actor;
}

void actor_calc_fov(struct actor *actor)
{
    struct map *map = &actor->game->maps[actor->level];

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

void actor_calc_light(struct actor *actor)
{
    struct map *map = &actor->game->maps[actor->level];

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
        actor->torch_fov = map_to_fov_map(map, actor->x, actor->y, actor->game->actor_common.torch_radius);
    }
    else if (actor->glow)
    {
        actor->glow_fov = map_to_fov_map(map, actor->x, actor->y, actor->game->actor_common.glow_radius);
    }
}

void actor_ai(struct actor *actor)
{
    if (actor == actor->game->player)
    {
        return;
    }

    int x = actor->x + TCOD_random_get_int(NULL, -1, 1);
    int y = actor->y + TCOD_random_get_int(NULL, -1, 1);

    actor_move(actor, x, y);
}

bool actor_path_towards(struct actor *actor, int x, int y)
{
    bool success = false;

    struct map *map = &actor->game->maps[actor->level];

    TCOD_map_t TCOD_map = map_to_TCOD_map(map);
    TCOD_map_set_properties(TCOD_map, x, y, TCOD_map_is_transparent(TCOD_map, x, y), true);

    TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1.0f);
    TCOD_path_compute(path, actor->x, actor->y, x, y);

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
    double d = distance(actor->x, actor->y, x, y);

    if (d > 0.0)
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

    struct map *map = &actor->game->maps[actor->level];
    struct tile *tile = &map->tiles[x][y];
    struct tile_info *tile_info = &actor->game->tile_info[tile->type];

    if (tile->type == TILE_DOOR_CLOSED)
    {
        return actor_open_door(actor, tile);
    }

    if (!tile_info->is_walkable)
    {
        return false;
    }

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

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

        actor_attack(actor, other);

        return true;
    }

    struct tile *current_tile = &map->tiles[actor->x][actor->y];

    TCOD_list_remove(current_tile->actors, actor);

    actor->x = x;
    actor->y = y;

    TCOD_list_push(tile->actors, actor);

    return true;
}

bool actor_interact(struct actor *actor, int x, int y, enum action action)
{
}

bool actor_open_door(struct actor *actor, struct tile *tile)
{
}

bool actor_close_door(struct actor *actor, struct tile *tile)
{
}

bool actor_ascend(struct actor *actor)
{
}

bool actor_descend(struct actor *actor)
{
}

void actor_swap(struct actor *actor, struct actor *other)
{
}

void actor_pick(struct actor *actor, struct actor *other)
{
}

bool actor_swing(struct actor *actor, int x, int y)
{
}

void actor_shoot(struct actor *actor, int x, int y, void (*on_hit)(void *on_hit_params), void *on_hit_params)
{
}

void actor_attack(struct actor *actor, struct actor *other)
{
    int attack_roll = roll(1, 20);
    int attack_bonus = 1;
    int total_attack = attack_roll + attack_bonus;
    int other_armor_class = 5;
    bool hit = attack_roll == 1
                   ? false
                   : attack_roll == 20
                         ? true
                         : total_attack >= other_armor_class;

    if (hit)
    {
        // 1d8 19-20x2
        int weapon_a = 1;
        int weapon_x = 8;
        int weapon_threat_range = 19;
        int weapon_crit_multiplier = 2;

        int damage_rolls = 1;

        bool crit = false;
        if (attack_roll >= weapon_threat_range)
        {
            int threat_roll = roll(1, 20);
            int total_threat = threat_roll + attack_bonus;

            if (total_threat >= other_armor_class)
            {
                crit = true;
                damage_rolls *= weapon_crit_multiplier;
            }
        }

        int total_damage = 0;
        int damage_bonus = 0;
        for (int i = 0; i < damage_rolls; i++)
        {
            int damage_roll = roll(weapon_a, weapon_x);
            int damage = damage_roll + damage_bonus;

            total_damage += damage;
        }

        game_log(
            actor->game,
            actor->level,
            actor->x, actor->y,
            crit ? TCOD_yellow : TCOD_white,
            "%s %s %s %s %s for %d",
            actor->game->race_info[actor->race].name,
            actor->game->class_info[actor->class].name,
            crit ? "crits" : "hits",
            other->game->race_info[other->race].name,
            other->game->class_info[other->class].name,
            total_damage);

        other->health -= total_damage;
        other->flash_color = TCOD_red;
        other->flash_fade = 1.0f;

        if (other->health <= 0)
        {
            actor_die(other, actor);
        }
    }
    else
    {
        game_log(
            actor->game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s %s misses",
            actor->game->race_info[actor->race].name,
            actor->game->class_info[actor->class].name);
    }
}

void actor_cast_spell(struct actor *actor)
{
}

void actor_die(struct actor *actor, struct actor *killer)
{
    game_log(
        actor->game,
        actor->level,
        actor->x,
        actor->y,
        TCOD_red,
        "%s %s dies",
        actor->game->race_info[actor->race].name,
        actor->game->class_info[actor->class].name);
    game_log(
        killer->game,
        killer->level,
        killer->x,
        killer->y,
        TCOD_azure,
        "%s %s gains %d experience",
        killer->game->race_info[killer->race].name,
        killer->game->class_info[killer->class].name,
        TCOD_random_get_int(NULL, 50, 100));
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

    for (void **iterator = TCOD_list_begin(actor->items); iterator != TCOD_list_end(actor->items); iterator++)
    {
        struct item *item = *iterator;

        item_destroy(item);
    }

    TCOD_list_delete(actor->items);

    free(actor);
}
