#include <libtcod.h>
#include <math.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "config.h"
#include "game.h"
#include "utils.h"

void game_init(void)
{
    player = NULL;
    should_render = false;
    should_quit = false;

    world_init();
    entities_init();
    msg_init();
}

void game_new(void)
{
    should_render = true;
    turn = 0;

    map_t *map = map_create(0);
    TCOD_list_push(maps, map);

    player = entity_create();
    position_t *player_position = (position_t *)component_add(player, COMPONENT_POSITION);
    player_position->map = map;
    player_position->x = map->stair_up_x;
    player_position->y = map->stair_up_y;
    TCOD_list_push(map->tiles[player_position->x][player_position->y].entities, player);
    TCOD_list_push(map->entities, player);
    physics_t *player_physics = (physics_t *)component_add(player, COMPONENT_PHYSICS);
    player_physics->is_walkable = false;
    player_physics->is_transparent = true;
    light_t *player_light = (light_t *)component_add(player, COMPONENT_LIGHT);
    player_light->radius = 5;
    player_light->color = TCOD_white;
    player_light->flicker = false;
    player_light->priority = LIGHT_PRIORITY_0;
    if (player_light->fov_map != NULL)
    {
        TCOD_map_delete(player_light->fov_map);
    }
    player_light->fov_map = NULL;
    fov_t *player_fov = (fov_t *)component_add(player, COMPONENT_FOV);
    player_fov->radius = 1;
    if (player_fov->fov_map != NULL)
    {
        TCOD_map_delete(player_fov->fov_map);
    }
    player_fov->fov_map = NULL;
    appearance_t *player_appearance = (appearance_t *)component_add(player, COMPONENT_APPEARANCE);
    player_appearance->name = "Blinky";
    player_appearance->glyph = '@';
    player_appearance->color = TCOD_white;
    player_appearance->layer = LAYER_1;
    health_t *player_health = (health_t *)component_add(player, COMPONENT_HEALTH);
    player_health->max = 20;
    player_health->current = player_health->max;
    alignment_t *player_alignment = (alignment_t *)component_add(player, COMPONENT_ALIGNMENT);
    player_alignment->type = ALIGNMENT_GOOD;
    targeting_t *player_targeting = (targeting_t *)component_add(player, COMPONENT_TARGETING);
    player_targeting->type = TARGETING_NONE;
    player_targeting->x = -1;
    player_targeting->y = -1;
    inventory_t *player_inventory = (inventory_t *)component_add(player, COMPONENT_INVENTORY);
    player_inventory->items = TCOD_list_new();
    ai_t *player_ai = (ai_t *)component_add(player, COMPONENT_AI);
    player_ai->type = AI_INPUT;
    player_ai->turn = true;
    player_ai->energy = 5.0f;
    player_ai->energy_per_turn = 5.0f;
    player_ai->follow_target = NULL;

    entity_t *pet = entity_create();
    position_t *pet_position = (position_t *)component_add(pet, COMPONENT_POSITION);
    pet_position->map = map;
    pet_position->x = map->stair_up_x + 1;
    pet_position->y = map->stair_up_y;
    TCOD_list_push(map->tiles[pet_position->x][pet_position->y].entities, pet);
    TCOD_list_push(map->entities, pet);
    physics_t *pet_physics = (physics_t *)component_add(pet, COMPONENT_PHYSICS);
    pet_physics->is_walkable = false;
    pet_physics->is_transparent = true;
    light_t *pet_light = (light_t *)component_add(pet, COMPONENT_LIGHT);
    pet_light->radius = 5;
    pet_light->color = TCOD_white;
    pet_light->flicker = false;
    pet_light->priority = LIGHT_PRIORITY_0;
    if (pet_light->fov_map != NULL)
    {
        TCOD_map_delete(pet_light->fov_map);
    }
    pet_light->fov_map = NULL;
    fov_t *pet_fov = (fov_t *)component_add(pet, COMPONENT_FOV);
    pet_fov->radius = 1;
    if (pet_fov->fov_map != NULL)
    {
        TCOD_map_delete(pet_fov->fov_map);
    }
    pet_fov->fov_map = NULL;
    appearance_t *pet_appearance = (appearance_t *)component_add(pet, COMPONENT_APPEARANCE);
    pet_appearance->name = "Spot";
    pet_appearance->glyph = 'd';
    pet_appearance->color = TCOD_white;
    pet_appearance->layer = LAYER_1;
    ai_t *pet_ai = (ai_t *)component_add(pet, COMPONENT_AI);
    pet_ai->type = AI_INPUT;
    pet_ai->turn = true;
    pet_ai->energy = 5.0f;
    pet_ai->energy_per_turn = 5.0f;
    pet_ai->follow_target = player;
    health_t *pet_health = (health_t *)component_add(pet, COMPONENT_HEALTH);
    pet_health->max = 20;
    pet_health->current = pet_health->max;
    alignment_t *pet_alignment = (alignment_t *)component_add(pet, COMPONENT_ALIGNMENT);
    pet_alignment->type = ALIGNMENT_GOOD;
    targeting_t *pet_targeting = (targeting_t *)component_add(pet, COMPONENT_TARGETING);
    pet_targeting->type = TARGETING_NONE;
    pet_targeting->x = -1;
    pet_targeting->y = -1;
    inventory_t *pet_inventory = (inventory_t *)component_add(pet, COMPONENT_INVENTORY);
    pet_inventory->items = TCOD_list_new();

    msg_log(NULL, TCOD_white, "Hail, %s!", player_appearance->name);
}

void game_input(void)
{
    ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            should_quit = true;

            break;
        }
        case TCODK_ENTER:
        {
            if (key.lalt)
            {
                fullscreen = !fullscreen;

                TCOD_console_set_fullscreen(fullscreen);
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

void game_update(void)
{
    for (void **iterator = TCOD_list_begin(maps); iterator != TCOD_list_end(maps); iterator++)
    {
        map_t *map = *iterator;

        TCOD_list_t lights = map_get_lights(map);

        entity_t *next = NULL;

        for (void **iterator = TCOD_list_begin(map->entities); iterator != TCOD_list_end(map->entities); iterator++)
        {
            entity_t *entity = *iterator;

            ai_t *ai = (ai_t *)component_get(entity, COMPONENT_AI);

            if (ai != NULL)
            {
                if (ai->turn)
                {
                    if (ai->type == AI_INPUT)
                    {
                        should_render = true;

                        player = entity;
                    }

                    next = entity;

                    break;
                }
            }
        }

        if (next == NULL)
        {
            turn++;

            for (void **iterator = TCOD_list_begin(map->entities); iterator != TCOD_list_end(map->entities); iterator++)
            {
                entity_t *entity = *iterator;

                ai_t *ai = (ai_t *)component_get(entity, COMPONENT_AI);

                if (ai != NULL)
                {
                    ai->turn = true;
                    ai->energy += ai->energy_per_turn;
                }
            }
        }
        else
        {
            entity_calc_ai(next);
            entity_calc_fov(next, lights);
        }

        TCOD_list_delete(lights);
    }
}

#define CONSTRAIN_VIEW 1

void game_render(void)
{
    if (player != NULL)
    {
        if (should_render)
        {
            should_render = false;

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
            msg_height = console_height / 4;
            msg_y = console_height - msg_height;
            msg_width = console_width;

            view_width = console_width;
            view_height = console_height - msg_height;
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

            for (void **iterator = TCOD_list_begin(player_position->map->entities); iterator != TCOD_list_end(player_position->map->entities); iterator++)
            {
                entity_t *entity = *iterator;

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
                            TCOD_console_set_char_foreground(NULL, position->x - view_x, position->y - view_y, entity == player ? TCOD_green : appearance->color);
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

            if (player_targeting->type != TARGETING_NONE)
            {
                TCOD_console_set_char_foreground(NULL, player_targeting->x - view_x, player_targeting->y - view_y, TCOD_red);
                TCOD_console_set_char(NULL, player_targeting->x - view_x, player_targeting->y - view_y, 'X');
            }

            static TCOD_console_t msg = NULL;

            if (msg == NULL)
            {
                msg = TCOD_console_new(console_width, console_height);
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
    }
}

void game_reset(void)
{
    world_reset();
    entities_reset();
    msg_reset();
}