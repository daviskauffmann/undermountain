#include <libtcod.h>
#include <stdio.h>
#include <math.h>

#include "config.h"
#include "game.h"
#include "utils.h"

void entity_init(entity_t *entity, int id, game_t *game)
{
    entity->id = id;
    entity->game = game;
}

entity_t *entity_create(game_t *game)
{
    for (int i = 0; i < NUM_ENTITIES; i++)
    {
        entity_t *entity = &game->entities[i];

        if (entity->id == ID_UNUSED)
        {
            entity_init(entity, i, game);

            return entity;
        }
    }

    return NULL;
}

void entity_path_towards(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position != NULL)
    {
        TCOD_map_t TCOD_map = map_to_TCOD_map(&entity->game->maps[position->level]);
        TCOD_map_set_properties(TCOD_map, x, y, TCOD_map_is_transparent(TCOD_map, x, y), true);

        TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1.0f);
        TCOD_path_compute(path, position->x, position->y, x, y);

        int next_x, next_y;
        if (!TCOD_path_is_empty(path) && TCOD_path_walk(path, &next_x, &next_y, false))
        {
            entity_move(entity, next_x, next_y);
        }
        else
        {
            entity_move_towards(entity, x, y);
        }

        TCOD_path_delete(path);

        TCOD_map_delete(TCOD_map);
    }
}

void entity_move_towards(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position != NULL)
    {
        int dx = x - position->x;
        int dy = y - position->y;
        float d = distance(position->x, position->y, x, y);

        if (d > 0)
        {
            dx = round(dx / d);
            dy = round(dy / d);

            entity_move(entity, position->x + dx, position->y + dy);
        }
    }
}

void entity_move_random(entity_t *entity)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position != NULL)
    {
        int x = position->x + TCOD_random_get_int(NULL, -1, 1);
        int y = position->y + TCOD_random_get_int(NULL, -1, 1);

        entity_move(entity, x, y);
    }
}

void entity_move(entity_t *entity, int x, int y)
{
    if (map_is_inside(x, y))
    {
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

        if (position != NULL)
        {
            tile_t *current_tile = &entity->game->maps[position->level].tiles[position->x][position->y];
            tile_t *next_tile = &entity->game->maps[position->level].tiles[x][y];

            bool can_move = true;

            if (!entity->game->tile_info[next_tile->type].is_walkable)
            {
                can_move = false;
            }

            for (void **iterator = TCOD_list_begin(next_tile->entities); iterator != TCOD_list_end(next_tile->entities); iterator++)
            {
                entity_t *other = *iterator;

                if (other != NULL)
                {
                    if (other->id != ID_UNUSED)
                    {
                        physics_t *other_physics = (physics_t *)component_get(other, COMPONENT_PHYSICS);

                        if (other_physics != NULL)
                        {
                            if (!other_physics->is_walkable)
                            {
                                can_move = false;
                            }
                        }

                        health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);

                        if (other_health != NULL)
                        {
                            alignment_t *alignment = (alignment_t *)component_get(entity, COMPONENT_ALIGNMENT);
                            alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);

                            if (alignment != NULL && other_alignment != NULL && alignment->type != other_alignment->type)
                            {
                                entity_attack(entity, other);
                            }
                            else
                            {
                                if (entity == entity->game->player)
                                {
                                    entity_swap(entity, other);
                                }
                            }
                        }
                    }
                }
            }

            if (can_move)
            {
                position->x = x;
                position->y = y;

                TCOD_list_remove(current_tile->entities, entity);
                TCOD_list_push(next_tile->entities, entity);
            }
        }
    }
}

void entity_swap(entity_t *entity, entity_t *other)
{
    if (entity != other)
    {
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
        appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

        position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);
        appearance_t *other_appearance = (appearance_t *)component_get(other, COMPONENT_APPEARANCE);

        if (position != NULL && appearance != NULL &&
            other_position != NULL && other_appearance != NULL)
        {
            game_log(entity->game, position, TCOD_white, "%s swaps with %s", appearance->name, other_appearance->name);

            tile_t *tile = &entity->game->maps[position->level].tiles[position->x][position->y];
            tile_t *other_tile = &other->game->maps[other_position->level].tiles[other_position->x][other_position->y];

            int x = position->x;
            int y = position->y;

            position->x = other_position->x;
            position->y = other_position->y;

            other_position->x = x;
            other_position->y = y;

            TCOD_list_remove(tile->entities, entity);
            TCOD_list_push(other_tile->entities, entity);

            TCOD_list_remove(other_tile->entities, other);
            TCOD_list_push(tile->entities, other);
        }
    }
}

void entity_pick(entity_t *entity, entity_t *other)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
    inventory_t *inventory = (inventory_t *)component_get(entity, COMPONENT_INVENTORY);

    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);
    appearance_t *other_appearance = (appearance_t *)component_get(other, COMPONENT_APPEARANCE);
    pickable_t *other_pickable = (pickable_t *)component_get(other, COMPONENT_PICKABLE);

    if (position != NULL && appearance != NULL && inventory != NULL &&
        other_appearance != NULL && other_position != NULL && other_pickable != NULL)
    {
        TCOD_list_remove(other->game->maps[other_position->level].tiles[other_position->x][other_position->y].entities, other);
        TCOD_list_push(inventory->items, other);

        component_remove(other, COMPONENT_POSITION);

        game_log(entity->game, position, TCOD_white, "%s picks up %s", appearance->name, other_appearance->name);
    }
}

void entity_swing(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

    if (position != NULL && appearance != NULL)
    {
        tile_t *other_tile = &entity->game->maps[position->level].tiles[x][y];

        bool hit = false;

        for (void **iterator = TCOD_list_begin(other_tile->entities); iterator != TCOD_list_end(other_tile->entities); iterator++)
        {
            entity_t *other = *iterator;

            health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);

            if (other_health != NULL)
            {
                hit = true;

                entity_attack(entity, other);

                break;
            }
        }

        if (!hit)
        {
            game_log(entity->game, position, TCOD_white, "%s swings at the air", appearance->name);
        }
    }
}

void entity_shoot(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

    if (position != NULL && appearance != NULL)
    {
        if (position->x == x && position->y == y)
        {
            game_log(entity->game, position, TCOD_white, "%s thinks that's a bad idea!", appearance->name);
        }
        else
        {
            game_log(entity->game, position, TCOD_white, "%s shoots", appearance->name);

            tile_t *other_tile = &entity->game->maps[position->level].tiles[x][y];

            bool hit = false;

            for (void **iterator = TCOD_list_begin(other_tile->entities); iterator != TCOD_list_end(other_tile->entities); iterator++)
            {
                entity_t *other = *iterator;

                health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);

                if (other_health != NULL)
                {
                    hit = true;

                    entity_attack(entity, other);

                    break;
                }
            }

            if (!hit)
            {
                game_log(entity->game, position, TCOD_white, "%s's arrow flies through the air", appearance->name);
            }
        }
    }
}

void entity_attack(entity_t *entity, entity_t *other)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);
    appearance_t *other_appearance = (appearance_t *)component_get(other, COMPONENT_APPEARANCE);
    health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);

    if (position != NULL && appearance != NULL &&
        other_position != NULL && other_appearance != NULL && other_health != NULL)
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

            game_log(entity->game, position, crit ? TCOD_yellow : TCOD_white, "%s %s %s for %d", appearance->name, crit ? "crits" : "hits", other_appearance->name, total_damage);

            other_health->current -= total_damage;

            took_damage_t *other_took_damage = (took_damage_t *)component_add(other, COMPONENT_TOOK_DAMAGE);
            other_took_damage->fade = 1.0f;

            if (other_health->current <= 0)
            {
                entity_die(other, entity);
            }
        }
        else
        {
            game_log(entity->game, position, TCOD_white, "%s misses", appearance->name);
        }
    }
}

void entity_die(entity_t *entity, entity_t *killer)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
    physics_t *physics = (physics_t *)component_get(entity, COMPONENT_PHYSICS);
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

    if (position != NULL && physics != NULL && appearance != NULL)
    {
        game_log(entity->game, position, TCOD_red, "%s dies", appearance->name);

        physics->is_walkable = true;

        appearance->glyph = '%';
        appearance->layer = LAYER_0;

        component_remove(entity, COMPONENT_AI);
        component_remove(entity, COMPONENT_HEALTH);
        component_remove(entity, COMPONENT_ALIGNMENT);

        if (killer != NULL)
        {
            position_t *killer_position = (position_t *)component_get(killer, COMPONENT_POSITION);
            appearance_t *killer_appearance = (appearance_t *)component_get(killer, COMPONENT_APPEARANCE);

            if (killer_position != NULL && killer_appearance != NULL)
            {
                game_log(killer->game, killer_position, TCOD_azure, "%s gains %d experience", killer_appearance->name, TCOD_random_get_int(NULL, 50, 100));
            }
        }

        if (entity == entity->game->player)
        {
            entity->game->game_over = true;

            TCOD_sys_delete_file("../saves/save.gz");

            game_log(entity->game, position, TCOD_green, "Game over! Press 'r' to restart");
        }
    }
}

void entity_destroy(entity_t *entity)
{
    if (entity != NULL && entity->id != ID_UNUSED)
    {
        for (int i = 0; i < NUM_COMPONENTS; i++)
        {
            component_remove(entity, i);
        }

        entity_reset(entity);
    }
}

void entity_reset(entity_t *entity)
{
    entity->id = ID_UNUSED;
    entity->game = NULL;
}

void component_init(component_t *component, int id, component_type_t component_type)
{
    component->id = id;
    component->type = component_type;

    switch (component->type)
    {
    case COMPONENT_AI:
    {
        ai_t *ai = (ai_t *)component;

        ai->energy = 0.0f;
        ai->energy_per_turn = 0.0f;
        ai->follow_target = NULL;

        break;
    }
    case COMPONENT_ALIGNMENT:
    {
        alignment_t *alignment = (alignment_t *)component;

        alignment->type = 0;

        break;
    }
    case COMPONENT_APPEARANCE:
    {
        appearance_t *appearance = (appearance_t *)component;

        appearance->name = NULL;
        appearance->glyph = ' ';
        appearance->color = TCOD_white;
        appearance->layer = 0;

        break;
    }
    case COMPONENT_FOV:
    {
        fov_t *fov = (fov_t *)component;

        fov->radius = 0;
        fov->fov_map = NULL;

        break;
    }
    case COMPONENT_HEALTH:
    {
        health_t *health = (health_t *)component;

        health->max = 0;
        health->current = 0;

        break;
    }
    case COMPONENT_INVENTORY:
    {
        inventory_t *inventory = (inventory_t *)component;

        inventory->items = NULL;
        for (int i = 0; i < NUM_EQUIPMENT_SLOTS; i++)
        {
            inventory->equipment[i] = NULL;
        }

        break;
    }
    case COMPONENT_LIGHT:
    {
        light_t *light = (light_t *)component;

        light->radius = 0;
        light->color = TCOD_white;
        light->flicker = false;
        light->priority = 0;
        light->fov_map = NULL;

        break;
    }
    case COMPONENT_PHYSICS:
    {
        physics_t *physics = (physics_t *)component;

        physics->is_walkable = false;
        physics->is_transparent = false;

        break;
    }
    case COMPONENT_PICKABLE:
    {
        pickable_t *pickable = (pickable_t *)component;

        pickable->weight = 0.0f;

        break;
    }
    case COMPONENT_POSITION:
    {
        position_t *position = (position_t *)component;

        position->level = 0;
        position->x = 0;
        position->y = 0;

        break;
    }
    case COMPONENT_TARGETING:
    {
        targeting_t *targeting = (targeting_t *)component;

        targeting->type = 0;
        targeting->x = 0;
        targeting->y = 0;

        break;
    }
    case COMPONENT_TOOK_DAMAGE:
    {
        took_damage_t *took_damage = (took_damage_t *)component;

        took_damage->fade = 0.0f;

        break;
    }
    }
}

component_t *component_add(entity_t *entity, component_type_t component_type)
{
    component_t *component = NULL;

    if (entity != NULL && entity->id != ID_UNUSED)
    {
        component = &entity->game->components[component_type][entity->id];

        component_init(component, entity->id, component_type);
    }

    return component;
}

component_t *component_get(entity_t *entity, component_type_t component_type)
{
    component_t *component = NULL;

    if (entity != NULL && entity->id != ID_UNUSED)
    {
        component = &entity->game->components[component_type][entity->id];

        if (component->id == ID_UNUSED)
        {
            component = NULL;
        }
    }

    return component;
}

void component_remove(entity_t *entity, component_type_t component_type)
{
    if (entity != NULL && entity->id != ID_UNUSED)
    {
        component_t *component = &entity->game->components[component_type][entity->id];

        component_reset(component);
    }
}

void component_reset(component_t *component)
{
    component->id = ID_UNUSED;

    switch (component->type)
    {
    case COMPONENT_AI:
    {
        ai_t *ai = (ai_t *)component;

        break;
    }
    case COMPONENT_ALIGNMENT:
    {
        alignment_t *alignment = (alignment_t *)component;

        break;
    }
    case COMPONENT_APPEARANCE:
    {
        appearance_t *appearance = (appearance_t *)component;

        break;
    }
    case COMPONENT_FOV:
    {
        fov_t *fov = (fov_t *)component;

        if (fov->fov_map != NULL)
        {
            TCOD_map_delete(fov->fov_map);
        }

        break;
    }
    case COMPONENT_HEALTH:
    {
        health_t *health = (health_t *)component;

        break;
    }
    case COMPONENT_INVENTORY:
    {
        inventory_t *inventory = (inventory_t *)component;

        if (inventory->items != NULL)
        {
            TCOD_list_delete(inventory->items);
        }

        break;
    }
    case COMPONENT_LIGHT:
    {
        light_t *light = (light_t *)component;

        if (light->fov_map != NULL)
        {
            TCOD_map_delete(light->fov_map);
        }

        break;
    }
    case COMPONENT_PHYSICS:
    {
        physics_t *physics = (physics_t *)component;

        break;
    }
    case COMPONENT_PICKABLE:
    {
        pickable_t *pickable = (pickable_t *)component;

        break;
    }
    case COMPONENT_POSITION:
    {
        position_t *position = (position_t *)component;

        break;
    }
    case COMPONENT_TARGETING:
    {
        targeting_t *targeting = (targeting_t *)component;

        break;
    }
    case COMPONENT_TOOK_DAMAGE:
    {
        took_damage_t *took_damage = (took_damage_t *)component;

        break;
    }
    }
}