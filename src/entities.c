#include <libtcod.h>
#include <stdio.h>

#include "config.h"
#include "game.h"
#include "utils.h"

entity_t *entity_create(game_t *game)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &game->entities[i];

        if (entity->id == ID_UNUSED)
        {
            entity->id = i;

            return entity;
        }
    }

    return NULL;
}

void entity_calc_fov(entity_t *entity, TCOD_list_t lights)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
    fov_t *fov = (fov_t *)component_get(entity, COMPONENT_FOV);

    if (position != NULL && fov != NULL)
    {
        if (fov->fov_map != NULL)
        {
            TCOD_map_delete(fov->fov_map);
        }

        fov->fov_map = map_to_fov_map(position->map, position->x, position->y, fov->radius);

        if (lights != NULL)
        {
            TCOD_map_t los_map = map_to_fov_map(position->map, position->x, position->y, 0);

            for (int x = 0; x < MAP_WIDTH; x++)
            {
                for (int y = 0; y < MAP_HEIGHT; y++)
                {
                    if (TCOD_map_is_in_fov(los_map, x, y))
                    {
                        tile_t *tile = &position->map->tiles[x][y];

                        for (void **iterator = TCOD_list_begin(lights); iterator != TCOD_list_end(lights); iterator++)
                        {
                            entity_t *entity = *iterator;

                            position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                            light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);

                            if (position != NULL && light != NULL)
                            {
                                if (TCOD_map_is_in_fov(light->fov_map, x, y))
                                {
                                    TCOD_map_set_in_fov(fov->fov_map, x, y, true);
                                }
                            }
                        }
                    }
                }
            }

            TCOD_map_delete(los_map);
        }
    }
}

void entity_calc_ai(entity_t *entity)
{
    ai_t *ai = (ai_t *)component_get(entity, COMPONENT_AI);

    if (ai != NULL)
    {
        bool took_turn = false;

        if (ai->energy >= 1.0f)
        {
            switch (ai->type)
            {
            case AI_INPUT:
            {
                position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                fov_t *fov = (fov_t *)component_get(entity, COMPONENT_FOV);
                alignment_t *alignment = (alignment_t *)component_get(entity, COMPONENT_ALIGNMENT);
                targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                switch (entity->game->ev)
                {
                case TCOD_EVENT_KEY_PRESS:
                {
                    switch (entity->game->key.vk)
                    {
                    case TCODK_KP1:
                    {
                        if (targeting != NULL && targeting->type != TARGETING_NONE)
                        {
                            targeting->x--;
                            targeting->y++;
                        }
                        else
                        {
                            took_turn = true;

                            int x = position->x - 1;
                            int y = position->y + 1;

                            if (entity->game->key.lctrl)
                            {
                                entity_swing(entity, x, y);
                            }
                            else
                            {
                                entity_move(entity, x, y);
                            }
                        }

                        break;
                    }
                    case TCODK_KP2:
                    {
                        if (targeting != NULL && targeting->type != TARGETING_NONE)
                        {
                            targeting->y++;
                        }
                        else
                        {
                            took_turn = true;

                            int x = position->x;
                            int y = position->y + 1;

                            if (entity->game->key.lctrl)
                            {
                                entity_swing(entity, x, y);
                            }
                            else
                            {
                                entity_move(entity, x, y);
                            }
                        }

                        break;
                    }
                    case TCODK_KP3:
                    {
                        if (targeting != NULL && targeting->type != TARGETING_NONE)
                        {
                            targeting->x++;
                            targeting->y++;
                        }
                        else
                        {
                            took_turn = true;

                            int x = position->x + 1;
                            int y = position->y + 1;

                            if (entity->game->key.lctrl)
                            {
                                entity_swing(entity, x, y);
                            }
                            else
                            {
                                entity_move(entity, x, y);
                            }
                        }

                        break;
                    }
                    case TCODK_KP4:
                    {
                        if (targeting != NULL && targeting->type != TARGETING_NONE)
                        {
                            targeting->x--;
                            targeting->y;
                        }
                        else
                        {
                            took_turn = true;

                            int x = position->x - 1;
                            int y = position->y;

                            if (entity->game->key.lctrl)
                            {
                                entity_swing(entity, x, y);
                            }
                            else
                            {
                                entity_move(entity, x, y);
                            }
                        }

                        break;
                    }
                    case TCODK_KP5:
                    {
                        took_turn = true;

                        break;
                    }
                    case TCODK_KP6:
                    {
                        if (targeting != NULL && targeting->type != TARGETING_NONE)
                        {
                            targeting->x++;
                            targeting->y;
                        }
                        else
                        {
                            took_turn = true;

                            int x = position->x + 1;
                            int y = position->y;

                            if (entity->game->key.lctrl)
                            {
                                entity_swing(entity, x, y);
                            }
                            else
                            {
                                entity_move(entity, x, y);
                            }
                        }

                        break;
                    }
                    case TCODK_KP7:
                    {
                        if (targeting != NULL && targeting->type != TARGETING_NONE)
                        {
                            targeting->x--;
                            targeting->y--;
                        }
                        else
                        {
                            took_turn = true;

                            int x = position->x - 1;
                            int y = position->y - 1;

                            if (entity->game->key.lctrl)
                            {
                                entity_swing(entity, x, y);
                            }
                            else
                            {
                                entity_move(entity, x, y);
                            }
                        }
                        break;
                    }
                    case TCODK_KP8:
                    {
                        if (targeting != NULL && targeting->type != TARGETING_NONE)
                        {
                            targeting->x;
                            targeting->y--;
                        }
                        else
                        {
                            took_turn = true;

                            int x = position->x;
                            int y = position->y - 1;

                            if (entity->game->key.lctrl)
                            {
                                entity_swing(entity, x, y);
                            }
                            else
                            {
                                entity_move(entity, x, y);
                            }
                        }

                        break;
                    }
                    case TCODK_KP9:
                    {
                        if (targeting != NULL && targeting->type != TARGETING_NONE)
                        {
                            targeting->x++;
                            targeting->y--;
                        }
                        else
                        {
                            took_turn = true;

                            int x = position->x + 1;
                            int y = position->y - 1;

                            if (entity->game->key.lctrl)
                            {
                                entity_swing(entity, x, y);
                            }
                            else
                            {
                                entity_move(entity, x, y);
                            }
                        }

                        break;
                    }
                    case TCODK_CHAR:
                    {
                        switch (entity->game->key.c)
                        {
                        case 'f':
                        {
                            if (targeting != NULL)
                            {
                                if (targeting->type == TARGETING_SHOOT)
                                {
                                    took_turn = true;

                                    targeting->type = TARGETING_NONE;

                                    entity_shoot(entity, targeting->x, targeting->y);
                                }
                                else
                                {
                                    targeting->type = TARGETING_SHOOT;

                                    bool target_found = false;

                                    if (alignment != NULL)
                                    {
                                        for (void **iterator = TCOD_list_begin(position->map->entities); iterator != TCOD_list_end(position->map->entities); iterator++)
                                        {
                                            entity_t *other = *iterator;

                                            position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);
                                            alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);

                                            if (other_position != NULL && other_alignment != NULL)
                                            {
                                                if (TCOD_map_is_in_fov(fov->fov_map, other_position->x, other_position->y) &&
                                                    other_alignment->type != alignment->type)
                                                {
                                                    target_found = true;

                                                    targeting->x = other_position->x;
                                                    targeting->y = other_position->y;
                                                }
                                            }
                                        }
                                    }

                                    if (!target_found)
                                    {
                                        targeting->x = position->x;
                                        targeting->y = position->y;
                                    }
                                }
                            }

                            break;
                        }
                        case 'g':
                        {
                            inventory_t *inventory = (inventory_t *)component_get(entity, COMPONENT_INVENTORY);

                            if (inventory != NULL)
                            {
                                tile_t *tile = &position->map->tiles[position->x][position->y];

                                bool item_found = false;

                                for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
                                {
                                    entity_t *other = *iterator;

                                    pickable_t *pickable = (pickable_t *)component_get(other, COMPONENT_PICKABLE);

                                    if (pickable != NULL)
                                    {
                                        took_turn = true;

                                        item_found = true;

                                        entity_pick(entity, other);

                                        break;
                                    }
                                }

                                if (!item_found)
                                {
                                    msg_log(entity->game, position, TCOD_white, "There is nothing here!");
                                }
                            }

                            break;
                        }
                        case 'l':
                        {
                            if (targeting != NULL)
                            {
                                if (targeting->type == TARGETING_LOOK)
                                {
                                    targeting->type = TARGETING_NONE;

                                    msg_log(entity->game, position, TCOD_white, "Look!");
                                }
                                else
                                {
                                    targeting->type = TARGETING_LOOK;
                                    targeting->x = position->x;
                                    targeting->y = position->y;
                                }
                            }

                            break;
                        }
                        case 't':
                        {
                            light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);

                            if (light != NULL)
                            {
                                took_turn = true;

                                static bool torch = false;

                                torch = !torch;

                                if (torch)
                                {
                                    light->radius = 10;
                                    light->color = TCOD_light_amber;
                                    light->flicker = true;
                                    light->priority = LIGHT_PRIORITY_2;
                                }
                                else
                                {
                                    light->radius = 5;
                                    light->color = TCOD_white;
                                    light->flicker = false;
                                    light->priority = LIGHT_PRIORITY_0;
                                }
                            }

                            break;
                        }
                        case 'z':
                        {
                            if (targeting != NULL)
                            {
                                if (targeting->type == TARGETING_ZAP)
                                {
                                    took_turn = true;

                                    targeting->type = TARGETING_NONE;

                                    msg_log(entity->game, position, TCOD_white, "Zap!");
                                }
                                else
                                {
                                    targeting->type = TARGETING_ZAP;

                                    bool target_found = false;

                                    if (alignment != NULL)
                                    {
                                        for (void **iterator = TCOD_list_begin(position->map->entities); iterator != TCOD_list_end(position->map->entities); iterator++)
                                        {
                                            entity_t *other = *iterator;

                                            position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);
                                            alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);

                                            if (other_position != NULL && other_alignment != NULL)
                                            {
                                                if (TCOD_map_is_in_fov(fov->fov_map, other_position->x, other_position->y) &&
                                                    other_alignment->type != alignment->type)
                                                {
                                                    target_found = true;

                                                    targeting->x = other_position->x;
                                                    targeting->y = other_position->y;
                                                }
                                            }
                                        }
                                    }

                                    if (!target_found)
                                    {
                                        targeting->x = position->x;
                                        targeting->y = position->y;
                                    }
                                }
                            }

                            break;
                        }
                        }

                        break;
                    }
                    }
                }
                }

                break;
            }
            case AI_GENERIC:
            {
                position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

                if (position != NULL)
                {
                    bool target_found = false;

                    fov_t *fov = (fov_t *)component_get(entity, COMPONENT_FOV);
                    alignment_t *alignment = (alignment_t *)component_get(entity, COMPONENT_ALIGNMENT);

                    if (fov != NULL && alignment != NULL)
                    {
                        for (void **iterator = TCOD_list_begin(position->map->entities); iterator != TCOD_list_end(position->map->entities); iterator++)
                        {
                            entity_t *other = *iterator;

                            if (other->id != ID_UNUSED)
                            {
                                position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);
                                alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);

                                if (other_position != NULL && other_alignment != NULL)
                                {
                                    if (TCOD_map_is_in_fov(fov->fov_map, other_position->x, other_position->y) &&
                                        other_alignment->type != alignment->type)
                                    {
                                        target_found = true;

                                        if (distance(position->x, position->y, other_position->x, other_position->y) < 2.0f)
                                        {
                                            entity_attack(entity, other);
                                        }
                                        else
                                        {
                                            entity_path_towards(entity, other_position->x, other_position->y);
                                        }

                                        break;
                                    }
                                }
                            }
                        }
                    }

                    if (ai->follow_target != NULL)
                    {
                        position_t *follow_position = (position_t *)component_get(ai->follow_target, COMPONENT_POSITION);

                        if (follow_position != NULL)
                        {
                            if (!TCOD_map_is_in_fov(fov->fov_map, follow_position->x, follow_position->y) ||
                                distance(position->x, position->y, follow_position->x, follow_position->y) > 5.0f)
                            {
                                target_found = true;

                                entity_path_towards(entity, follow_position->x, follow_position->y);
                            }
                        }
                    }

                    if (!target_found)
                    {
                        entity_move_random(entity);
                    }

                    took_turn = true;
                }

                break;
            }
            }
        }
        else
        {
            ai->turn = false;
        }

        if (took_turn)
        {
            ai->energy -= 1.0f;

            if (ai->energy < 1.0f)
            {
                ai->turn = false;
            }
        }
    }
}

void entity_path_towards(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position != NULL)
    {
        TCOD_map_t TCOD_map = map_to_TCOD_map(position->map);
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
        float distance = distance(position->x, position->y, x, y);

        if (distance > 0)
        {
            dx = round(dx / distance);
            dy = round(dy / distance);

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
            tile_t *current_tile = &position->map->tiles[position->x][position->y];
            tile_t *next_tile = &position->map->tiles[x][y];

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
            msg_log(entity->game, position, TCOD_white, "%s swaps with %s", appearance->name, other_appearance->name);

            tile_t *tile = &position->map->tiles[position->x][position->y];
            tile_t *other_tile = &other_position->map->tiles[other_position->x][other_position->y];

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
        TCOD_list_remove(other_position->map->tiles[other_position->x][other_position->y].entities, other);
        TCOD_list_push(inventory->items, other);

        component_remove(other, COMPONENT_POSITION);

        msg_log(entity->game, position, TCOD_white, "%s picks up %s", appearance->name, other_appearance->name);
    }
}

void entity_swing(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

    if (position != NULL && appearance != NULL)
    {
        tile_t *other_tile = &position->map->tiles[x][y];

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
            msg_log(entity->game, position, TCOD_white, "%s swings at the air", appearance->name);
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
            msg_log(entity->game, position, TCOD_white, "%s tries to shoot himself!", appearance->name);
        }
        else
        {
            msg_log(entity->game, position, TCOD_white, "%s shoots", appearance->name);

            tile_t *other_tile = &position->map->tiles[x][y];

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
                msg_log(entity->game, position, TCOD_white, "%s's arrow flies through the air", appearance->name);
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
            int bonus_damage = 0;
            for (int i = 0; i < damage_rolls; i++)
            {
                total_damage += roll(weapon_a, weapon_x) + bonus_damage;
            }

            msg_log(entity->game, position, crit ? TCOD_yellow : TCOD_white, "%s %s %s for %d", appearance->name, crit ? "crits" : "hits", other_appearance->name, total_damage);

            other_health->current -= total_damage;

            if (other_health->current <= 0)
            {
                entity_die(other);
            }
        }
        else
        {
            msg_log(entity->game, position, TCOD_white, "%s misses", appearance->name);
        }
    }
}

void entity_die(entity_t *entity)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
    physics_t *physics = (physics_t *)component_get(entity, COMPONENT_PHYSICS);
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

    if (position != NULL && physics != NULL && appearance != NULL)
    {
        msg_log(entity->game, position, TCOD_red, "%s dies", appearance->name);

        physics->is_walkable = true;

        appearance->glyph = '%';
        appearance->layer = LAYER_0;

        component_remove(entity, COMPONENT_AI);
        component_remove(entity, COMPONENT_LIGHT);
        component_remove(entity, COMPONENT_HEALTH);
        component_remove(entity, COMPONENT_ALIGNMENT);
    }
}

void entity_destroy(entity_t *entity)
{
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        component_remove(entity, i);
    }

    entity->id = ID_UNUSED;
}

component_t *component_add(entity_t *entity, component_type_t component_type)
{
    if (entity->id == ID_UNUSED)
    {
        return NULL;
    }

    component_t *component = &entity->game->components[component_type][entity->id];

    component->id = entity->id;

    return component;
}

component_t *component_get(entity_t *entity, component_type_t component_type)
{
    if (entity->id == ID_UNUSED)
    {
        return NULL;
    }

    component_t *component = &entity->game->components[component_type][entity->id];

    if (component->id != ID_UNUSED)
    {
        return component;
    }

    return NULL;
}

void component_remove(entity_t *entity, component_type_t component_type)
{
    if (entity->id == ID_UNUSED)
    {
        return;
    }

    component_t *component = &entity->game->components[component_type][entity->id];

    component->id = ID_UNUSED;
}