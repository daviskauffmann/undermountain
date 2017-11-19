#include <libtcod.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "game.h"
#include "CMemLeak.h"
#include "config.h"
#include "ECS.h"
#include "world.h"

/* Game */
void game_init(void)
{
    msg = TCOD_console_new(screen_width, screen_height);
    messages = TCOD_list_new();
}

void game_new(void)
{
    game_status = STATUS_UPDATE;
    turn = 0;

    map_t *map = map_create(0);
    TCOD_list_push(maps, map);

    player = entity_create();
    position_t *player_position = (position_t *)component_add(player, COMPONENT_POSITION);
    player_position->map = map;
    player_position->x = map->stair_up_x;
    player_position->y = map->stair_up_y;
    TCOD_list_push(map->tiles[player_position->x][player_position->y].entities, player);
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

    entity_t *pet = entity_create();
    position_t *pet_position = (position_t *)component_add(pet, COMPONENT_POSITION);
    pet_position->map = map;
    pet_position->x = map->stair_up_x + 1;
    pet_position->y = map->stair_up_y;
    TCOD_list_push(map->tiles[pet_position->x][pet_position->y].entities, pet);
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
    pet_ai->type = AI_PET;
    pet_ai->energy = 1.0f;
    pet_ai->energy_per_turn = 0.5f;

    msg_log(NULL, TCOD_white, "Hail, %s!", player_appearance->name);
}

void game_reset(void)
{
    TCOD_console_delete(msg);

    for (void **iterator = TCOD_list_begin(messages); iterator != TCOD_list_end(messages); iterator++)
    {
        message_t *message = *iterator;

        message_destroy(message);
    }

    TCOD_list_delete(messages);
}

/* Message Log */
message_t *message_create(char *text, TCOD_color_t color)
{
    message_t *message = (message_t *)malloc(sizeof(message_t));

    message->text = strdup(text);
    message->color = color;

    return message;
}

void message_destroy(message_t *message)
{
    free(message->text);
    free(message);
}

void msg_log(position_t *position, TCOD_color_t color, char *text, ...)
{
    position_t *player_position = (position_t *)component_get(player, COMPONENT_POSITION);
    fov_t *player_fov = (fov_t *)component_get(player, COMPONENT_FOV);

    if (position == NULL ||
        (position->map == player_position->map &&
         ((position->x == player_position->x && position->y == player_position->y) ||
          TCOD_map_is_in_fov(player_fov->fov_map, position->x, position->y))))
    {
        va_list ap;
        char buf[128];
        va_start(ap, text);
        vsprintf(buf, text, ap);
        va_end(ap);

        char *line_begin = buf;
        char *line_end;

        do
        {
            if (TCOD_list_size(messages) == 4)
            {
                message_t *message = TCOD_list_get(messages, 0);

                TCOD_list_remove(messages, message);

                message_destroy(message);
            }

            line_end = strchr(line_begin, '\n');

            if (line_end)
            {
                *line_end = '\0';
            }

            message_t *message = message_create(line_begin, color);

            TCOD_list_push(messages, message);

            line_begin = line_end + 1;
        } while (line_end);
    }
}