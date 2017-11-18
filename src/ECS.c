#include <libtcod.h>
#include <math.h>

#include "utils.h"
#include "config.h"
#include "game.h"
#include "world.h"
#include "ECS.h"

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

            dx = round(dx / distance);
            dy = round(dy / distance);

            entity_move(entity, position->x + dx, position->y + dy);
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

            if (next_tile->entity != NULL)
            {
                if (next_tile->entity->id != ID_UNUSED)
                {
                    // TODO: check for combat
                    if (true)
                    {
                        can_move = false;

                        entity_attack(entity, next_tile->entity);
                    }
                    else
                    {
                        physics_t *physics = (physics_t *)component_get(next_tile->entity, COMPONENT_PHYSICS);

                        if (physics != NULL)
                        {
                            if (!physics->is_walkable)
                            {
                                can_move = false;
                            }
                        }
                    }
                }
            }

            if (can_move)
            {
                position->x = x;
                position->y = y;

                current_tile->entity = NULL;
                next_tile->entity = entity;
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

            if (other_tile->entity != NULL)
            {
                entity_attack(entity, other_tile->entity);
            }
            else
            {
                msg_log(position, TCOD_white, "%s swings at the air", appearance->name);
            }
        }
    }
}

void entity_attack(entity_t *entity, entity_t *other)
{
    if (entity->id != ID_UNUSED && other->id != ID_UNUSED)
    {
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
        appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

        appearance_t *other_appearance = (appearance_t *)component_get(other, COMPONENT_APPEARANCE);

        if (position != NULL && appearance != NULL &&
            other_appearance != NULL)
        {
            // TODO: calculate damage
            msg_log(position, TCOD_white, "%s attacks %s", appearance->name, other_appearance->name);

            // TODO: check health
            if (other != player)
            {
                position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                msg_log(other_position, TCOD_red, "%s dies", other_appearance->name);

                other_position->map->tiles[other_position->x][other_position->y].entity = NULL;

                entity_destroy(other);
            }
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
            game_status = STATUS_UPDATE;

            int x = player_position->x - 1;
            int y = player_position->y + 1;

            entity_move(player, x, y);

            break;
        }
        case TCODK_KP2:
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

            break;
        }
        case TCODK_KP3:
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

            break;
        }
        case TCODK_KP4:
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

            break;
        }
        case TCODK_KP5:
        {
            game_status = STATUS_UPDATE;

            break;
        }
        case TCODK_KP6:
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

            break;
        }
        case TCODK_KP7:
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

            break;
        }
        case TCODK_KP8:
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

            break;
        }
        case TCODK_KP9:
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

            break;
        }
        case TCODK_CHAR:
        {
            switch (key.c)
            {
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
            }

            break;
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

                        position_t *player_position = (position_t *)component_get(player, COMPONENT_POSITION);

                        if (position != NULL && fov != NULL)
                        {
                            // TODO: properly look for all hostile targets
                            if (TCOD_map_is_in_fov(fov->fov_map, player_position->x, player_position->y))
                            {
                                if (distance(position->x, position->y, player_position->x, player_position->y) < 2.0f)
                                {
                                    entity_attack(entity, player);
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

                    ai->energy -= 1.0f;
                }
            }
        }
    }
}

void movement_system(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
        {
            position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

            if (position != NULL)
            {
                position->map->tiles[position->x][position->y].entity = entity;
            }
        }
    }
}

void lighting_system(void)
{
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
    view_x = player_position->x - view_width / 2;
    view_y = player_position->y - view_height / 2;

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
    TCOD_list_t light_entities[NUM_LIGHT_PRIORITIES];

    for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
    {
        light_entities[i] = TCOD_list_new();
    }

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
        {
            position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
            light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);

            if (position != NULL && light != NULL)
            {
                TCOD_list_push(light_entities[light->priority], entity);
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
                    for (void **iterator = TCOD_list_begin(light_entities[i]); iterator != TCOD_list_end(light_entities[i]); iterator++)
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
                        for (void **iterator = TCOD_list_begin(light_entities[i]); iterator != TCOD_list_end(light_entities[i]); iterator++)
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

                TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, color);
                TCOD_console_set_char(NULL, x - view_x, y - view_y, tile_info[tile->type].glyph);
            }
        }
    }

    for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
    {
        TCOD_list_delete(light_entities[i]);
    }

    // TODO: maybe store a list of all entities on a given map?
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
        {
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