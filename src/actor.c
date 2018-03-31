#include <libtcod/libtcod.h>
#include <malloc.h>
#include <math.h>

#include "actor.h"
#include "game.h"
#include "item.h"
#include "map.h"
#include "projectile.h"
#include "tile.h"
#include "util.h"
#include "window.h"

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
    actor->energy = 1.0f;
    actor->last_seen_x = -1;
    actor->last_seen_y = -1;
    actor->glow = false;
    actor->glow_fov = NULL;
    actor->torch = TCOD_random_get_int(NULL, 0, 10) == 0;
    actor->torch_fov = NULL;
    actor->fov = NULL;
    actor->items = TCOD_list_new();
    actor->dead = false;

    return actor;
}

void actor_update_inventory(struct actor *actor)
{
    for (void **iterator = TCOD_list_begin(actor->items); iterator != TCOD_list_end(actor->items); iterator++)
    {
        struct item *item = *iterator;

        item->x = actor->x;
        item->y = actor->y;
    }
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

        for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
        {
            struct actor *other = *iterator;

            if (TCOD_map_is_in_fov(actor->fov, other->x, other->y) && other->faction != actor->faction && !other->dead)
            {
                actor->last_seen_x = other->x;
                actor->last_seen_y = other->y;

                if (distance(actor->x, actor->y, other->x, other->y) < 2.0f)
                {
                    actor_attack(actor, other);
                }
                else
                {
                    actor_path_towards(actor, other->x, other->y);
                }

                goto done;
            }
        }

        if (actor->last_seen_x != -1 && actor->last_seen_y != -1)
        {
            if (actor->x == actor->last_seen_x && actor->y == actor->last_seen_y)
            {
                actor->last_seen_x = -1;
                actor->last_seen_y = -1;
            }
            else
            {
                actor_path_towards(actor, actor->last_seen_x, actor->last_seen_y);

                goto done;
            }
        }

        for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
        {
            struct object *object = *iterator;

            switch (object->type)
            {
            case OBJECT_STAIR_DOWN:
            {
                if (actor_descend(actor))
                {
                    goto done;
                }
            }
            break;
            case OBJECT_STAIR_UP:
            {
                if (actor_ascend(actor))
                {
                    goto done;
                }
            }
            break;
            }
        }

        int x = actor->x + TCOD_random_get_int(NULL, -1, 1);
        int y = actor->y + TCOD_random_get_int(NULL, -1, 1);

        actor_move(actor, x, y);
    done:;
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

    return true;
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

        if (object->type == OBJECT_DOOR_CLOSED)
        {
            return actor_open_door(actor, x, y);
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
            return actor_attack(actor, other);
        }
    }

    struct tile *current_tile = &map->tiles[actor->x][actor->y];

    TCOD_list_remove(current_tile->actors, actor);

    actor->x = x;
    actor->y = y;

    TCOD_list_push(tile->actors, actor);

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

    game_log(
        game,
        actor->level,
        actor->x,
        actor->y,
        TCOD_white,
        "%s %s swaps with %s %s",
        game->race_info[actor->race].name,
        game->class_info[actor->class].name,
        game->race_info[other->race].name,
        game->class_info[other->class].name);

    return false;
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

        if (object->type == OBJECT_DOOR_CLOSED)
        {
            success = true;

            object->type = OBJECT_DOOR_OPEN;

            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_white,
                "%s %s opens the door",
                game->race_info[actor->race].name,
                game->class_info[actor->class].name);

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
            "%s %s can't open that",
            game->race_info[actor->race].name,
            game->class_info[actor->class].name);
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

        if (object->type == OBJECT_DOOR_OPEN)
        {
            success = true;

            object->type = OBJECT_DOOR_CLOSED;

            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_white,
                "%s %s closes the door",
                game->race_info[actor->race].name,
                game->class_info[actor->class].name);

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
            "%s %s can't close that",
            game->race_info[actor->race].name,
            game->class_info[actor->class].name);
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
            "%s %s has reached the end",
            game->race_info[actor->race].name,
            game->class_info[actor->class].name);

        return false;
    }

    bool success = false;

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (object->type == OBJECT_STAIR_DOWN)
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

            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_white,
                "%s %s descends",
                game->race_info[actor->race].name,
                game->class_info[actor->class].name);

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
            "%s %s can't descend here",
            game->race_info[actor->race].name,
            game->class_info[actor->class].name);
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
            "%s %s can't go any higher",
            game->race_info[actor->race].name,
            game->class_info[actor->class].name);

        return false;
    }

    bool success = false;

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (object->type == OBJECT_STAIR_UP)
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

            game_log(
                game,
                actor->level,
                actor->x,
                actor->y,
                TCOD_white,
                "%s %s descends",
                game->race_info[actor->race].name,
                game->class_info[actor->class].name);
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
            "%s %s can't ascend here",
            game->race_info[actor->race].name,
            game->class_info[actor->class].name);
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

    if (TCOD_list_size(tile->items) > 0)
    {
        struct item *item = TCOD_list_pop(tile->items);

        TCOD_list_push(actor->items, item);

        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s %s picks up %s",
            game->race_info[actor->race].name,
            game->class_info[actor->class].name,
            game->item_info[item->type].name);

        return true;
    }
    else
    {
        game_log(
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "There is nothing to pick up!");
    }

    return false;
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

    game_log(
        game,
        actor->level,
        actor->x,
        actor->y,
        TCOD_white,
        "%s %s swings at the air!",
        game->race_info[actor->race].name,
        game->class_info[actor->class].name);

    return true;
}

#include <stdio.h>

bool actor_shoot(struct actor *actor, int x, int y, void (*on_hit)(void *on_hit_params), void *on_hit_params)
{
    struct game *game = actor->game;
    struct map *map = &game->maps[actor->level];

    if (x == actor->x && y == actor->y)
    {
        return false;
    }

    float dx = (float)x - (float)actor->x;
    float dy = (float)y - (float)actor->y;
    float d = (float)distance(actor->x, actor->y, x, y);

    if (d > 0)
    {
        dx /= d;
        dy /= d;
    }

    struct projectile *projectile = projectile_create(
        game,
        '`',
        actor->level,
        (float)actor->x,
        (float)actor->y,
        dx,
        dy,
        actor,
        on_hit,
        on_hit_params);

    TCOD_list_push(map->projectiles, projectile);

    return true;
}

bool actor_attack(struct actor *actor, struct actor *other)
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
            "%s %s cannot attack that!",
            game->race_info[actor->race].name,
            game->class_info[actor->class].name);

        return false;
    }

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
            game,
            actor->level,
            actor->x,
            actor->y,
            crit ? TCOD_yellow : TCOD_white,
            "%s %s %s %s %s for %d",
            game->race_info[actor->race].name,
            game->class_info[actor->class].name,
            crit ? "crits" : "hits",
            game->race_info[other->race].name,
            game->class_info[other->class].name,
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
            game,
            actor->level,
            actor->x,
            actor->y,
            TCOD_white,
            "%s %s misses",
            game->race_info[actor->race].name,
            game->class_info[actor->class].name);
    }

    return true;
}

bool actor_cast_spell(struct actor *actor)
{
    return false;
}

void actor_die(struct actor *actor, struct actor *killer)
{
    struct game *game = actor->game;

    actor->dead = true;
    actor->glow = false;
    actor->torch = false;

    game_log(
        game,
        actor->level,
        actor->x,
        actor->y,
        TCOD_red,
        "%s %s dies",
        game->race_info[actor->race].name,
        game->class_info[actor->class].name);

    if (killer)
    {
        int experience = TCOD_random_get_int(NULL, 50, 100);

        game_log(
            game,
            killer->level,
            killer->x,
            killer->y,
            TCOD_azure,
            "%s %s gains %d experience",
            game->race_info[killer->race].name,
            game->class_info[killer->class].name,
            experience);
    }

    if (actor == game->player)
    {
        game->game_over = true;

        TCOD_sys_delete_file("../saves/save.gz");

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

    free(actor);
}
