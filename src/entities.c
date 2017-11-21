#include <libtcod.h>

#include "game.h"
#include "utils.h"

void entities_init(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        entity->id = ID_UNUSED;

        for (int j = 0; j < NUM_COMPONENTS; j++)
        {
            component_t *component = &components[j][i];

            component->id = ID_UNUSED;
        }
    }
}

void entities_reset(void)
{
}

entity_t *entity_create(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

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
        ai->energy += ai->energy_per_turn;

        while (ai->energy >= 1.0f)
        {
            ai->energy -= 1.0f;

            switch (ai->type)
            {
            case AI_MONSTER:
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

                            if (entity->id != ID_UNUSED && other != entity)
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

                    if (!target_found)
                    {
                        entity_move_random(entity);
                    }
                }

                break;
            }
            case AI_PET:
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

                                        entity_path_towards(entity, other_position->x, other_position->y);
                                    }
                                }
                            }
                        }
                    }

                    if (!target_found)
                    {
                        position_t *player_position = (position_t *)component_get(player, COMPONENT_POSITION);

                        if (TCOD_map_is_in_fov(fov->fov_map, player_position->x, player_position->y) &&
                            distance(position->x, position->y, player_position->x, player_position->y) < 5.0f)
                        {
                            entity_move_random(entity);
                        }
                        else
                        {
                            entity_path_towards(entity, player_position->x, player_position->y);
                        }
                    }
                }

                break;
            }
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

            if (!tile_info[next_tile->type].is_walkable)
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

                        // TODO: properly check for combat

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
                                if (entity == player)
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
            msg_log(position, TCOD_white, "%s swaps with %s", appearance->name, other_appearance->name);

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

        msg_log(position, TCOD_white, "%s picks up %s", appearance->name, other_appearance->name);
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
            msg_log(position, TCOD_white, "%s swings at the air", appearance->name);
        }
    }
}

void entity_shoot(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

    if (position != NULL && appearance != NULL)
    {
        msg_log(position, TCOD_white, "%s shoots", appearance->name);

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
            msg_log(position, TCOD_white, "%s's arrow flies through the air", appearance->name);
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
        // TODO: calculate damage
        bool hit = TCOD_random_get_int(NULL, 0, 4) > 0;
        bool crit = TCOD_random_get_int(NULL, 0, 5) == 0;

        if (hit)
        {
            if (other == player)
            {
                msg_log(position, TCOD_white, "%s's attack fizzles", appearance->name);
            }
            else
            {
                int damage = TCOD_random_get_int(NULL, 1, 8) * (crit ? 2 : 1);

                msg_log(position, TCOD_white, "%s %s %s for %d", appearance->name, (crit ? "crits" : "hits"), other_appearance->name, damage);

                other_health->current -= damage;

                if (other_health->current <= 0)
                {
                    entity_die(other);
                }
            }
        }
        else
        {
            msg_log(position, TCOD_white, "%s misses", appearance->name);
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
        msg_log(position, TCOD_red, "%s dies", appearance->name);

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
    component_t *component = &components[component_type][entity->id];

    component->id = entity->id;

    return component;
}

component_t *component_get(entity_t *entity, component_type_t component_type)
{
    if (entity->id == ID_UNUSED)
    {
        return NULL;
    }

    component_t *component = &components[component_type][entity->id];

    if (component->id != ID_UNUSED)
    {
        return component;
    }

    return NULL;
}

void component_remove(entity_t *entity, component_type_t component_type)
{
    component_t *component = &components[component_type][entity->id];

    component->id = ID_UNUSED;
}