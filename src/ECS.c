#include <libtcod.h>
#include <math.h>
#include <stdio.h>

#include "ECS.h"
#include "CMemLeak.h"
#include "utils.h"
#include "config.h"
#include "game.h"
#include "world.h"

void ECS_init(void)
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

void ECS_reset(void)
{
}

/* Entities */
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

void entity_path_towards(entity_t *entity, int x, int y)
{
    if (entity->id != ID_UNUSED)
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
}

void entity_move_towards(entity_t *entity, int x, int y)
{
    if (entity->id != ID_UNUSED)
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
}

void entity_move_random(entity_t *entity)
{
    if (entity->id != ID_UNUSED)
    {
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

        if (position != NULL)
        {
            int x = position->x + TCOD_random_get_int(NULL, -1, 1);
            int y = position->y + TCOD_random_get_int(NULL, -1, 1);

            entity_move(entity, x, y);
        }
    }
}

void entity_move(entity_t *entity, int x, int y)
{
    if (entity->id != ID_UNUSED)
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
    if (entity->id != ID_UNUSED &&
        other->id != ID_UNUSED)
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
}

void entity_swing(entity_t *entity, int x, int y)
{
    if (entity->id != ID_UNUSED)
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
}

void entity_shoot(entity_t *entity, int x, int y)
{
    if (entity->id != ID_UNUSED)
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
}

void entity_attack(entity_t *entity, entity_t *other)
{
    if (entity->id != ID_UNUSED &&
        other->id != ID_UNUSED)
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
}

void entity_die(entity_t *entity)
{
    if (entity->id != ID_UNUSED)
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
}

void entity_destroy(entity_t *entity)
{
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        component_t *component = &components[i][entity->id];

        component->id = ID_UNUSED;
    }

    entity->id = ID_UNUSED;
}

/* Components */
component_t *component_add(entity_t *entity, component_type_t component_type)
{
    component_t *component = &components[component_type][entity->id];

    component->id = entity->id;

    return component;
}

component_t *component_get(entity_t *entity, component_type_t component_type)
{
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

/* Systems */
void input_system(void)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    position_t *player_position = (position_t *)component_get(player, COMPONENT_POSITION);
    fov_t *player_fov = (fov_t *)component_get(player, COMPONENT_FOV);
    alignment_t *player_alignment = (alignment_t *)component_get(player, COMPONENT_ALIGNMENT);
    targeting_t *player_targeting = (targeting_t *)component_get(player, COMPONENT_TARGETING);

    if (player_position != NULL && player_targeting != NULL)
    {
        switch (ev)
        {
        case TCOD_EVENT_KEY_PRESS:
        {
            switch (key.vk)
            {
            case TCODK_ESCAPE:
            {
                game_status = STATUS_QUIT;

                break;
            }
            case TCODK_KP1:
            {
                if (player_targeting->active == true)
                {
                    player_targeting->x--;
                    player_targeting->y++;
                }
                else
                {
                    game_status = STATUS_UPDATE;

                    int x = player_position->x - 1;
                    int y = player_position->y + 1;

                    if (key.lctrl)
                    {
                        entity_swing(player, x, y);
                    }
                    else
                    {
                        entity_move(player, x, y);
                    }
                }

                break;
            }
            case TCODK_KP2:
            {
                if (player_targeting->active == true)
                {
                    player_targeting->y++;
                }
                else
                {
                    game_status = STATUS_UPDATE;

                    int x = player_position->x;
                    int y = player_position->y + 1;

                    if (key.lctrl)
                    {
                        entity_swing(player, x, y);
                    }
                    else
                    {
                        entity_move(player, x, y);
                    }
                }

                break;
            }
            case TCODK_KP3:
            {
                if (player_targeting->active == true)
                {
                    player_targeting->x++;
                    player_targeting->y++;
                }
                else
                {
                    game_status = STATUS_UPDATE;

                    int x = player_position->x + 1;
                    int y = player_position->y + 1;

                    if (key.lctrl)
                    {
                        entity_swing(player, x, y);
                    }
                    else
                    {
                        entity_move(player, x, y);
                    }
                }

                break;
            }
            case TCODK_KP4:
            {
                if (player_targeting->active == true)
                {
                    player_targeting->x--;
                    player_targeting->y;
                }
                else
                {
                    game_status = STATUS_UPDATE;

                    int x = player_position->x - 1;
                    int y = player_position->y;

                    if (key.lctrl)
                    {
                        entity_swing(player, x, y);
                    }
                    else
                    {
                        entity_move(player, x, y);
                    }
                }

                break;
            }
            case TCODK_KP5:
            {
                game_status = STATUS_UPDATE;

                break;
            }
            case TCODK_KP6:
            {
                if (player_targeting->active == true)
                {
                    player_targeting->x++;
                    player_targeting->y;
                }
                else
                {
                    game_status = STATUS_UPDATE;

                    int x = player_position->x + 1;
                    int y = player_position->y;

                    if (key.lctrl)
                    {
                        entity_swing(player, x, y);
                    }
                    else
                    {
                        entity_move(player, x, y);
                    }
                }

                break;
            }
            case TCODK_KP7:
            {
                if (player_targeting->active == true)
                {
                    player_targeting->x--;
                    player_targeting->y--;
                }
                else
                {
                    game_status = STATUS_UPDATE;

                    int x = player_position->x - 1;
                    int y = player_position->y - 1;

                    if (key.lctrl)
                    {
                        entity_swing(player, x, y);
                    }
                    else
                    {
                        entity_move(player, x, y);
                    }
                }
                break;
            }
            case TCODK_KP8:
            {
                if (player_targeting->active == true)
                {
                    player_targeting->x;
                    player_targeting->y--;
                }
                else
                {
                    game_status = STATUS_UPDATE;

                    int x = player_position->x;
                    int y = player_position->y - 1;

                    if (key.lctrl)
                    {
                        entity_swing(player, x, y);
                    }
                    else
                    {
                        entity_move(player, x, y);
                    }
                }

                break;
            }
            case TCODK_KP9:
            {
                if (player_targeting->active == true)
                {
                    player_targeting->x++;
                    player_targeting->y--;
                }
                else
                {
                    game_status = STATUS_UPDATE;

                    int x = player_position->x + 1;
                    int y = player_position->y - 1;

                    if (key.lctrl)
                    {
                        entity_swing(player, x, y);
                    }
                    else
                    {
                        entity_move(player, x, y);
                    }
                }

                break;
            }
            case TCODK_CHAR:
            {
                switch (key.c)
                {
                case 'f':
                {
                    if (player_targeting->active)
                    {
                        game_status = STATUS_UPDATE;

                        player_targeting->active = false;

                        entity_shoot(player, player_targeting->x, player_targeting->y);
                    }
                    else
                    {
                        player_targeting->active = true;

                        bool target_found = false;

                        for (int i = 0; i < MAX_ENTITIES; i++)
                        {
                            entity_t *entity = &entities[i];

                            if (entity->id != ID_UNUSED)
                            {
                                position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                                alignment_t *alignment = (alignment_t *)component_get(entity, COMPONENT_ALIGNMENT);

                                if (position != NULL && alignment != NULL)
                                {
                                    if (TCOD_map_is_in_fov(player_fov->fov_map, position->x, position->y) &&
                                        alignment->type != player_alignment->type)
                                    {
                                        target_found = true;

                                        player_targeting->x = position->x;
                                        player_targeting->y = position->y;
                                    }
                                }
                            }
                        }

                        if (!target_found)
                        {
                            player_targeting->x = player_position->x;
                            player_targeting->y = player_position->y;
                        }
                    }

                    break;
                }
                case 'r':
                {
                    game_reset();
                    world_reset();
                    ECS_reset();

                    ECS_init();
                    world_init();
                    game_init();

                    game_new();

                    break;
                }
                case 't':
                {
                    game_status = STATUS_UPDATE;

                    static bool torch = false;

                    light_t *player_light = (light_t *)component_get(player, COMPONENT_LIGHT);

                    torch = !torch;

                    if (torch)
                    {
                        player_light->radius = 10;
                        player_light->color = TCOD_light_amber;
                        player_light->flicker = true;
                        player_light->priority = LIGHT_PRIORITY_1;
                    }
                    else
                    {
                        player_light->radius = 5;
                        player_light->color = TCOD_white;
                        player_light->flicker = false;
                        player_light->priority = LIGHT_PRIORITY_0;
                    }

                    break;
                }
                }

                break;
            }
            }
        }
        }
    }
}

void ai_system(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
        {
            ai_t *ai = (ai_t *)component_get(entity, COMPONENT_AI);

            if (ai != NULL)
            {
                ai->energy += ai->energy_per_turn;

                while (ai->energy >= 1.0f)
                {
                    switch (ai->type)
                    {
                    case AI_MONSTER:
                    {
                        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                        fov_t *fov = (fov_t *)component_get(entity, COMPONENT_FOV);
                        alignment_t *alignment = (alignment_t *)component_get(entity, COMPONENT_ALIGNMENT);

                        if (position != NULL && fov != NULL)
                        {
                            bool target_found = false;

                            for (int i = 0; i < MAX_ENTITIES; i++)
                            {
                                entity_t *other = &entities[i];

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
                        fov_t *fov = (fov_t *)component_get(entity, COMPONENT_FOV);
                        alignment_t *alignment = (alignment_t *)component_get(entity, COMPONENT_ALIGNMENT);

                        if (position != NULL && fov != NULL)
                        {
                            bool target_found = false;

                            for (int i = 0; i < MAX_ENTITIES; i++)
                            {
                                entity_t *other = &entities[i];

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

                            if (!target_found)
                            {
                                position_t *player_position = (position_t *)component_get(player, COMPONENT_POSITION);

                                if (player_position != NULL)
                                {
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
                        }

                        break;
                    }
                    }

                    ai->energy -= 1.0f;
                }
            }
        }
    }
}

void fov_system(void)
{
    TCOD_list_t light_entities = TCOD_list_new();

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
        {
            position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
            light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);

            if (position != NULL && light != NULL)
            {
                if (light->fov_map != NULL)
                {
                    TCOD_map_delete(light->fov_map);
                }

                light->fov_map = map_to_fov_map(position->map, position->x, position->y, light->radius);

                TCOD_list_push(light_entities, entity);
            }
        }
    }

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
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

                TCOD_map_t los_map = map_to_fov_map(position->map, position->x, position->y, 0);

                for (int x = 0; x < MAP_WIDTH; x++)
                {
                    for (int y = 0; y < MAP_HEIGHT; y++)
                    {
                        if (TCOD_map_is_in_fov(los_map, x, y))
                        {
                            tile_t *tile = &position->map->tiles[x][y];

                            for (void **iterator = TCOD_list_begin(light_entities); iterator != TCOD_list_end(light_entities); iterator++)
                            {
                                entity_t *entity = *iterator;

                                position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                                light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);

                                if (TCOD_map_is_in_fov(light->fov_map, x, y))
                                {
                                    TCOD_map_set_in_fov(fov->fov_map, x, y, true);
                                }
                            }
                        }
                    }
                }

                TCOD_map_delete(los_map);
            }
        }
    }

    TCOD_list_delete(light_entities);
}

#define CONSTRAIN_VIEW 1

void render_system(void)
{
    TCOD_console_set_default_background(NULL, TCOD_black);
    TCOD_console_set_default_foreground(NULL, TCOD_white);
    TCOD_console_clear(NULL);

    position_t *player_position = (position_t *)component_get(player, COMPONENT_POSITION);

    static int msg_x;
    static int msg_y;
    static int msg_width;
    static int msg_height;

    static int view_x;
    static int view_y;
    static int view_width;
    static int view_height;

    msg_x = 0;
    msg_height = screen_height / 4;
    msg_y = screen_height - msg_height;
    msg_width = screen_width;

    view_width = screen_width;
    view_height = screen_height - msg_height;
    if (player_position != NULL)
    {
        view_x = player_position->x - view_width / 2;
        view_y = player_position->y - view_height / 2;
    }

#if CONSTRAIN_VIEW
    view_x = view_x < 0
                 ? 0
                 : view_x + view_width > MAP_WIDTH
                       ? MAP_WIDTH - view_width
                       : view_x;
    view_y = view_y < 0
                 ? 0
                 : view_y + view_height > MAP_HEIGHT
                       ? MAP_HEIGHT - view_height
                       : view_y;
#endif

    static TCOD_noise_t noise = NULL;
    if (noise == NULL)
    {
        noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
    }

    static float noise_x = 0.0f;

    noise_x += 0.2f;
    float noise_dx = noise_x + 20.0f;
    float dx = TCOD_noise_get(noise, &noise_dx) * 0.5f;
    noise_dx += 30.0f;
    float dy = TCOD_noise_get(noise, &noise_dx) * 0.5f;
    float di = 0.2f * TCOD_noise_get(noise, &noise_x);

    // TODO: sort lights so that certain types get priority when drawing
    // torches > stationary lights > entity glow
    TCOD_list_t entities_by_layer[NUM_LAYERS];
    TCOD_list_t lights_by_priority[NUM_LIGHT_PRIORITIES];

    for (int i = 0; i < NUM_LAYERS; i++)
    {
        entities_by_layer[i] = TCOD_list_new();
    }

    for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
    {
        lights_by_priority[i] = TCOD_list_new();
    }

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
        {
            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

            if (appearance != NULL)
            {
                TCOD_list_push(entities_by_layer[appearance->layer], entity);
            }

            position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
            light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);

            if (light != NULL)
            {
                TCOD_list_push(lights_by_priority[light->priority], entity);
            }
        }
    }

    fov_t *player_fov = (fov_t *)component_get(player, COMPONENT_FOV);

    for (int x = view_x; x < view_x + view_width; x++)
    {
        for (int y = view_y; y < view_y + view_height; y++)
        {
            if (map_is_inside(x, y))
            {
                tile_t *tile = &player_position->map->tiles[x][y];

                if (TCOD_map_is_in_fov(player_fov->fov_map, x, y))
                {
                    tile->seen = true;
                }

                for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
                {
                    for (void **iterator = TCOD_list_begin(lights_by_priority[i]); iterator != TCOD_list_end(lights_by_priority[i]); iterator++)
                    {
                        entity_t *entity = *iterator;

                        light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);

                        if (TCOD_map_is_in_fov(light->fov_map, x, y))
                        {
                            tile->seen = true;
                        }
                    }
                }

                TCOD_color_t color = tile_common.shadow_color;

                if (TCOD_map_is_in_fov(player_fov->fov_map, x, y) || tile->seen)
                {
                    for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
                    {
                        for (void **iterator = TCOD_list_begin(lights_by_priority[i]); iterator != TCOD_list_end(lights_by_priority[i]); iterator++)
                        {
                            entity_t *entity = *iterator;

                            position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                            light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);

                            if (TCOD_map_is_in_fov(light->fov_map, x, y))
                            {
                                float r2 = pow(light->radius, 2);
                                float d = pow(x - position->x + (light->flicker ? dx : 0), 2) + pow(y - position->y + (light->flicker ? dy : 0), 2);
                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + (light->flicker ? di : 0));

                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info[tile->type].color, light->color, l), l);
                            }
                        }
                    }
                }
                else
                {
                    if (!tile->seen)
                    {
                        continue;
                    }
                }

                if (TCOD_list_peek(tile->entities) != NULL)
                {
                    // TCOD_console_set_char_background(NULL, x - view_x, y - view_y, TCOD_dark_gray, TCOD_BKGND_SET);
                }

                TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, color);
                TCOD_console_set_char(NULL, x - view_x, y - view_y, tile_info[tile->type].glyph);
            }
        }
    }

    for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
    {
        TCOD_list_delete(lights_by_priority[i]);
    }

    for (int i = 0; i < NUM_LAYERS; i++)
    {
        for (void **iterator = TCOD_list_begin(entities_by_layer[i]); iterator != TCOD_list_end(entities_by_layer[i]); iterator++)
        {
            entity_t *entity = *iterator;

            position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

            if (position != NULL && appearance != NULL)
            {
                if (position->map == player_position->map && TCOD_map_is_in_fov(player_fov->fov_map, position->x, position->y))
                {
                    TCOD_console_set_char_foreground(NULL, position->x - view_x, position->y - view_y, appearance->color);
                    TCOD_console_set_char(NULL, position->x - view_x, position->y - view_y, appearance->glyph);
                }
            }
        }
    }

    for (int i = 0; i < NUM_LAYERS; i++)
    {
        TCOD_list_delete(entities_by_layer[i]);
    }

    targeting_t *player_targeting = (targeting_t *)component_get(player, COMPONENT_TARGETING);

    if (player_targeting != NULL)
    {
        if (player_targeting->active)
        {
            TCOD_console_set_char_foreground(NULL, player_targeting->x - view_x, player_targeting->y - view_y, TCOD_red);
            TCOD_console_set_char(NULL, player_targeting->x - view_x, player_targeting->y - view_y, 'X');
        }
    }

    TCOD_console_set_default_background(msg, TCOD_black);
    TCOD_console_set_default_foreground(msg, TCOD_white);
    TCOD_console_clear(msg);

    int y = 1;
    for (void **i = TCOD_list_begin(messages); i != TCOD_list_end(messages); i++)
    {
        message_t *message = *i;

        TCOD_console_set_default_foreground(msg, message->color);
        TCOD_console_print(msg, msg_x + 1, y, message->text);

        y++;
    }

    TCOD_console_set_default_foreground(msg, TCOD_white);
    TCOD_console_print_frame(msg, 0, 0, msg_width, msg_height, false, TCOD_BKGND_SET, "Log");

    TCOD_console_blit(msg, 0, 0, msg_width, msg_height, NULL, msg_x, msg_y, 1, 1);

    TCOD_console_print(NULL, 0, 0, "Turn: %d", turn);

    TCOD_console_flush();
}