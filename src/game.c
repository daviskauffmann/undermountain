#include <assert.h>
#include <libtcod/libtcod.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "utils.h"
#include "message.h"
#include "tile.h"
#include "room.h"
#include "map.h"
#include "spell.h"
#include "entity.h"
#include "component.h"
#include "assemblage.h"
#include "panel.h"
#include "game.h"

void fn_should_update(game_t *game)
{
    game->should_update = true;
}

void game_run(void)
{
    local game_t game;

    game_init(&game);

    if (TCOD_sys_file_exists("../saves/save.gz"))
    {
        game_load(&game);
    }
    else
    {
        game_new(&game);
    }

    while (!TCOD_console_is_window_closed())
    {
        game_input(&game);
        game_update(&game);
        game_render(&game);

        if (game.should_restart)
        {
            game_reset(&game);
            game_init(&game);
            game_new(&game);
        }

        if (game.should_quit)
        {
            game_reset(&game);

            break;
        }
    }
}

void game_init(game_t *game)
{
    for (int id = 0; id < NUM_ENTITIES; id++)
    {
        entity_t *entity = &game->entities[id];

        entity_init(entity, ID_UNUSED, game);

        for (component_type_t component_type = 0; component_type < NUM_COMPONENTS; component_type++)
        {
            component_t *component = &game->components[component_type][id];

            component_init(component, ID_UNUSED, component_type);
        }
    }

    for (int level = 0; level < NUM_MAPS; level++)
    {
        map_t *map = &game->maps[level];

        map_init(map, game, level);
    }

    game->tile_common = (tile_common_t){
        .shadow_color = TCOD_color_RGB(16, 16, 32)};

    game->tile_info[TILE_EMPTY] = (tile_info_t){
        .name = "Empty",
        .glyph = ' ',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};
    game->tile_info[TILE_FLOOR] = (tile_info_t){
        .name = "Floor",
        .glyph = '.',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};
    game->tile_info[TILE_WALL] = (tile_info_t){
        .name = "Wall",
        .glyph = '#',
        .color = TCOD_white,
        .is_transparent = false,
        .is_walkable = false};
    game->tile_info[TILE_DOOR_CLOSED] = (tile_info_t){
        .name = "Closed Door",
        .glyph = '+',
        .color = TCOD_white,
        .is_transparent = false,
        .is_walkable = false};
    game->tile_info[TILE_DOOR_OPEN] = (tile_info_t){
        .name = "Open Door",
        .glyph = '-',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};
    game->tile_info[TILE_STAIR_DOWN] = (tile_info_t){
        .name = "Stair Down",
        .glyph = '>',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};
    game->tile_info[TILE_STAIR_UP] = (tile_info_t){
        .name = "Stair Up",
        .glyph = '<',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};

    game->player = NULL;

    game->current_panel = 0;
    game->panel_info[PANEL_CHARACTER] = (panel_info_t){
        .current = 0,
        .scroll = 0};
    game->panel_info[PANEL_INVENTORY] = (panel_info_t){
        .current = 0,
        .scroll = 0};

    game->turn = 0;
    game->turn_available = true;
    game->should_update = true;
    game->should_restart = false;
    game->should_quit = false;
    game->game_over = false;

    game->messages = TCOD_list_new();

    game->message_log_visible = true;
    game->panel_visible = false;
}

void game_new(game_t *game)
{
    TCOD_sys_delete_file("../saves/save.gz");

    for (int level = 0; level < NUM_MAPS; level++)
    {
        map_t *map = &game->maps[level];

        // map_generate_custom(map);
        map_generate_bsp(map);
        map_populate(map);
    }

    game->player = create_player(&game->maps[0], game->maps[0].stair_up_x, game->maps[0].stair_up_y);
    entity_t *pet = create_pet(&game->maps[0], game->maps[0].stair_up_x + 1, game->maps[0].stair_up_y);

    {
        appearance_t *player_appearance = (appearance_t *)component_get(game->player, COMPONENT_APPEARANCE);

        if (player_appearance)
        {
            game_log(game, NULL, TCOD_white, "Hail, %s!", player_appearance->name);
        }
    }
}

void game_save(game_t *game)
{
    TCOD_zip_t zip = TCOD_zip_new();

    for (int level = 0; level < NUM_MAPS; level++)
    {
        map_t *map = &game->maps[level];

        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                tile_t *tile = &map->tiles[x][y];

                TCOD_zip_put_int(zip, tile->type);
                TCOD_zip_put_int(zip, tile->seen);
            }
        }
    }

    for (int id = 0; id < NUM_ENTITIES; id++)
    {
        entity_t *entity = &game->entities[id];

        TCOD_zip_put_int(zip, entity->id);

        for (component_type_t component_type = 0; component_type < NUM_COMPONENTS; component_type++)
        {
            component_t *component = &game->components[component_type][id];

            TCOD_zip_put_int(zip, component->id);

            switch (component->type)
            {
            case COMPONENT_AI:
            {
                ai_t *ai = (ai_t *)component;

                TCOD_zip_put_float(zip, ai->energy);
                TCOD_zip_put_float(zip, ai->energy_per_turn);
                int follow_target_id = ID_UNUSED;
                if (ai->follow_target != NULL)
                {
                    follow_target_id = ai->follow_target->id;
                }
                TCOD_zip_put_int(zip, follow_target_id);
            }
            break;
            case COMPONENT_ALIGNMENT:
            {
                alignment_t *alignment = (alignment_t *)component;

                TCOD_zip_put_int(zip, alignment->type);
            }
            break;
            case COMPONENT_APPEARANCE:
            {
                appearance_t *appearance = (appearance_t *)component;

                TCOD_zip_put_string(zip, appearance->name);
                TCOD_zip_put_char(zip, appearance->glyph);
                TCOD_zip_put_color(zip, appearance->color);
                TCOD_zip_put_int(zip, appearance->layer);
            }
            break;
            case COMPONENT_CASTER:
            {
                caster_t *caster = (caster_t *)component;

                for (spell_type_t spell_type = 0; spell_type < NUM_SPELL_TYPES; spell_type++)
                {
                    TCOD_zip_put_int(zip, caster->spells[spell_type].type);
                    TCOD_zip_put_int(zip, caster->spells[spell_type].known);
                }
                TCOD_zip_put_int(zip, caster->current);
            }
            break;
            case COMPONENT_FLASH:
            {
                flash_t *flash = (flash_t *)component;

                TCOD_zip_put_color(zip, flash->color);
                TCOD_zip_put_float(zip, flash->fade);
            }
            break;
            case COMPONENT_FOV:
            {
                fov_t *fov = (fov_t *)component;

                TCOD_zip_put_int(zip, fov->radius);
            }
            break;
            case COMPONENT_HEALTH:
            {
                health_t *health = (health_t *)component;

                TCOD_zip_put_int(zip, health->max);
                TCOD_zip_put_int(zip, health->current);
            }
            break;
            case COMPONENT_INVENTORY:
            {
                inventory_t *inventory = (inventory_t *)component;
            }
            break;
            case COMPONENT_LIGHT:
            {
                light_t *light = (light_t *)component;

                TCOD_zip_put_int(zip, light->radius);
                TCOD_zip_put_color(zip, light->color);
                TCOD_zip_put_int(zip, light->flicker);
                TCOD_zip_put_int(zip, light->priority);
            }
            break;
            case COMPONENT_OPAQUE:
            {
            }
            break;
            case COMPONENT_PICKABLE:
            {
                pickable_t *pickable = (pickable_t *)component;

                TCOD_zip_put_float(zip, pickable->weight);
            }
            break;
            case COMPONENT_POSITION:
            {
                position_t *position = (position_t *)component;

                TCOD_zip_put_int(zip, position->level);
                TCOD_zip_put_int(zip, position->x);
                TCOD_zip_put_int(zip, position->y);
            }
            break;
            case COMPONENT_TARGETING:
            {
                targeting_t *targeting = (targeting_t *)component;

                TCOD_zip_put_int(zip, targeting->type);
                TCOD_zip_put_int(zip, targeting->x);
                TCOD_zip_put_int(zip, targeting->y);
            }
            break;
            case COMPONENT_SOLID:
            {
            }
            break;
            }
        }
    }

    TCOD_zip_put_int(zip, game->player->id);
    TCOD_zip_put_int(zip, game->turn);

    // int num_messages = TCOD_list_size(game->messages);
    // int num_bytes = sizeof(message_t) * num_messages;
    // message_t *messages = malloc(num_bytes);
    // int i = 0;

    // for (void **iterator = TCOD_list_begin(game->messages); iterator != TCOD_list_end(game->messages); iterator++)
    // {
    //     message_t *message = *iterator;

    //     messages[i] = *message;

    //     i++;
    // }

    // TCOD_zip_put_int(zip, num_messages);
    // TCOD_zip_put_data(zip, num_bytes, messages);

    TCOD_zip_save_to_file(zip, "../saves/save.gz");

    TCOD_zip_delete(zip);

    game_log(game, NULL, TCOD_green, "Game saved!");
}

void game_load(game_t *game)
{
    game_reset(game);
    game_init(game);

    TCOD_zip_t zip = TCOD_zip_new();

    TCOD_zip_load_from_file(zip, "../saves/save.gz");

    for (int level = 0; level < NUM_MAPS; level++)
    {
        map_t *map = &game->maps[level];

        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                tile_t *tile = &map->tiles[x][y];

                tile->type = TCOD_zip_get_int(zip);
                tile->seen = TCOD_zip_get_int(zip);
            }
        }
    }

    for (int id = 0; id < NUM_ENTITIES; id++)
    {
        entity_t *entity = &game->entities[id];

        entity->id = TCOD_zip_get_int(zip);

        for (component_type_t component_type = 0; component_type < NUM_COMPONENTS; component_type++)
        {
            component_t *component = &game->components[component_type][id];

            component->id = TCOD_zip_get_int(zip);

            switch (component->type)
            {
            case COMPONENT_AI:
            {
                ai_t *ai = (ai_t *)component;

                ai->energy = TCOD_zip_get_float(zip);
                ai->energy_per_turn = TCOD_zip_get_float(zip);
                int follow_target_id = TCOD_zip_get_int(zip);
                if (follow_target_id != ID_UNUSED)
                {
                    ai->follow_target = &game->entities[follow_target_id];
                }
            }
            break;
            case COMPONENT_ALIGNMENT:
            {
                alignment_t *alignment = (alignment_t *)component;

                alignment->type = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_APPEARANCE:
            {
                appearance_t *appearance = (appearance_t *)component;

                appearance->name = TCOD_zip_get_string(zip);
                appearance->glyph = TCOD_zip_get_char(zip);
                appearance->color = TCOD_zip_get_color(zip);
                appearance->layer = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_CASTER:
            {
                caster_t *caster = (caster_t *)component;

                for (spell_type_t spell_type = 0; spell_type < NUM_SPELL_TYPES; spell_type++)
                {
                    caster->spells[spell_type].type = TCOD_zip_get_int(zip);
                    caster->spells[spell_type].known = TCOD_zip_get_int(zip);
                }
                caster->current = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_FLASH:
            {
                flash_t *flash = (flash_t *)component;

                flash->color = TCOD_zip_get_color(zip);
                flash->fade = TCOD_zip_get_float(zip);
            }
            break;
            case COMPONENT_FOV:
            {
                fov_t *fov = (fov_t *)component;

                fov->radius = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_HEALTH:
            {
                health_t *health = (health_t *)component;

                health->max = TCOD_zip_get_int(zip);
                health->current = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_INVENTORY:
            {
                inventory_t *inventory = (inventory_t *)component;
            }
            break;
            case COMPONENT_LIGHT:
            {
                light_t *light = (light_t *)component;

                light->radius = TCOD_zip_get_int(zip);
                light->color = TCOD_zip_get_color(zip);
                light->flicker = TCOD_zip_get_int(zip);
                light->priority = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_OPAQUE:
            {
            }
            break;
            case COMPONENT_PICKABLE:
            {
                pickable_t *pickable = (pickable_t *)component;

                pickable->weight = TCOD_zip_get_float(zip);
            }
            break;
            case COMPONENT_POSITION:
            {
                position_t *position = (position_t *)component;

                position->level = TCOD_zip_get_int(zip);
                position->x = TCOD_zip_get_int(zip);
                position->y = TCOD_zip_get_int(zip);

                entity_map_place(entity);
            }
            break;
            case COMPONENT_TARGETING:
            {
                targeting_t *targeting = (targeting_t *)component;

                targeting->type = TCOD_zip_get_int(zip);
                targeting->x = TCOD_zip_get_int(zip);
                targeting->y = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_SOLID:
            {
            }
            break;
            }
        }
    }

    game->player = &game->entities[TCOD_zip_get_int(zip)];
    game->turn = TCOD_zip_get_int(zip);

    // int num_messages = TCOD_zip_get_int(zip);
    // int num_bytes = sizeof(message_t) * num_messages;
    // message_t *messages = malloc(num_bytes);

    // TCOD_zip_get_data(zip, num_bytes, messages);

    // for (int i = 0; i < num_messages; i++)
    // {
    //     TCOD_list_push(game->messages, &messages[i]);
    // }

    TCOD_zip_delete(zip);
}

void game_input(game_t *game)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            game->should_quit = true;
        }
        break;
        case TCODK_ENTER:
        {
            if (key.lalt)
            {
                fullscreen = !fullscreen;

                TCOD_console_set_fullscreen(fullscreen);
            }
        }
        break;
        case TCODK_CHAR:
        {
            switch (key.c)
            {
            case 'r':
            {
                game->should_restart = true;
            }
            break;
            }
        }
        break;
        }
    }
    break;
    }

    if (game->turn_available)
    {
        switch (ev)
        {
        case TCOD_EVENT_KEY_PRESS:
        {
            switch (key.vk)
            {
            case TCODK_KP5:
            {
                game->should_update = true;
            }
            break;
            }
        }
        break;
        }

        if (!game->game_over)
        {
            switch (ev)
            {
            case TCOD_EVENT_KEY_PRESS:
            {
                local action_t action = ACTION_NONE;

                switch (key.vk)
                {
                case TCODK_KP1:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        player_targeting->x--;
                        player_targeting->y++;
                    }
                    else if (player_position)
                    {
                        int x = player_position->x - 1;
                        int y = player_position->y + 1;

                        if (action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = entity_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = entity_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = entity_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP2:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        player_targeting->y++;
                    }
                    else if (player_position)
                    {
                        int x = player_position->x;
                        int y = player_position->y + 1;

                        if (action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = entity_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = entity_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = entity_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP3:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        player_targeting->x++;
                        player_targeting->y++;
                    }
                    else if (player_position)
                    {
                        int x = player_position->x + 1;
                        int y = player_position->y + 1;

                        if (action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = entity_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = entity_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = entity_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP4:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        player_targeting->x--;
                        player_targeting->y;
                    }
                    else if (player_position)
                    {
                        int x = player_position->x - 1;
                        int y = player_position->y;

                        if (action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = entity_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = entity_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = entity_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP5:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                    if (player_position)
                    {
                        int x = player_position->x;
                        int y = player_position->y;

                        if (action != ACTION_NONE)
                        {
                            game->should_update = entity_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP6:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        player_targeting->x++;
                        player_targeting->y;
                    }
                    else if (player_position)
                    {
                        int x = player_position->x + 1;
                        int y = player_position->y;

                        if (action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = entity_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = entity_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = entity_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP7:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        player_targeting->x--;
                        player_targeting->y--;
                    }
                    else if (player_position)
                    {
                        int x = player_position->x - 1;
                        int y = player_position->y - 1;

                        if (action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = entity_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = entity_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = entity_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP8:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        player_targeting->x;
                        player_targeting->y--;
                    }
                    else if (player_position)
                    {
                        int x = player_position->x;
                        int y = player_position->y - 1;

                        if (action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = entity_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = entity_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = entity_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_KP9:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        player_targeting->x++;
                        player_targeting->y--;
                    }
                    else if (player_position)
                    {
                        int x = player_position->x + 1;
                        int y = player_position->y - 1;

                        if (action == ACTION_NONE)
                        {
                            if (key.lctrl)
                            {
                                game->should_update = entity_swing(game->player, x, y);
                            }
                            else
                            {
                                game->should_update = entity_move(game->player, x, y);
                            }
                        }
                        else
                        {
                            game->should_update = entity_interact(game->player, x, y, action);

                            action = ACTION_NONE;
                        }
                    }
                }
                break;
                case TCODK_CHAR:
                {
                    switch (key.c)
                    {
                    case '<':
                    {
                        game->should_update = entity_ascend(game->player);
                    }
                    break;
                    case '>':
                    {
                        game->should_update = entity_descend(game->player);
                    }
                    break;
                    case 'C':
                    {
                        game->panel_visible = !game->panel_visible;
                    }
                    break;
                    case 'c':
                    {
                        action = ACTION_CLOSE_DOOR;

                        game_log(game, NULL, TCOD_white, "Choose a direction");
                    }
                    break;
                    case 'f':
                    {
                        position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                        if (player_position)
                        {
                            targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                            if (player_targeting && player_targeting->type == TARGETING_SHOOT)
                            {
                                entity_shoot(game->player, player_targeting->x, player_targeting->y, &fn_should_update, game);

                                component_remove(game->player, COMPONENT_TARGETING);
                            }
                            else
                            {
                                player_targeting = (targeting_t *)component_add(game->player, COMPONENT_TARGETING);
                                player_targeting->type = TARGETING_SHOOT;

                                bool target_found = false;

                                {
                                    alignment_t *player_alignment = (alignment_t *)component_get(game->player, COMPONENT_ALIGNMENT);
                                    fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);

                                    if (player_alignment && player_fov)
                                    {
                                        for (void **iterator = TCOD_list_begin(game->maps[player_position->level].entities); iterator != TCOD_list_end(game->maps[player_position->level].entities); iterator++)
                                        {
                                            entity_t *other = *iterator;

                                            alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);
                                            position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                                            if (other_alignment && other_position)
                                            {
                                                if (TCOD_map_is_in_fov(player_fov->fov_map, other_position->x, other_position->y) &&
                                                    other_alignment->type != player_alignment->type)
                                                {
                                                    target_found = true;

                                                    player_targeting->x = other_position->x;
                                                    player_targeting->y = other_position->y;

                                                    break;
                                                }
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
                        }
                    }
                    break;
                    case 'g':
                    {
                        inventory_t *player_inventory = (inventory_t *)component_get(game->player, COMPONENT_INVENTORY);
                        position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                        if (player_inventory && player_position)
                        {
                            tile_t *tile = &game->maps[player_position->level].tiles[player_position->x][player_position->y];

                            bool item_found = false;

                            for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
                            {
                                entity_t *other = *iterator;

                                pickable_t *pickable = (pickable_t *)component_get(other, COMPONENT_PICKABLE);

                                if (pickable)
                                {
                                    game->should_update = true;

                                    item_found = true;

                                    entity_pick(game->player, other);

                                    break;
                                }
                            }

                            if (!item_found)
                            {
                                game_log(game->player->game, player_position, TCOD_white, "There is nothing here!");
                            }
                        }
                    }
                    break;
                    case 'i':
                    {
                        game->current_panel = PANEL_INVENTORY;
                        game->panel_visible = !game->panel_visible;
                    }
                    break;
                    case 'l':
                    {
                        position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                        if (player_position)
                        {
                            targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                            if (player_targeting)
                            {
                                component_remove(game->player, COMPONENT_TARGETING);
                            }
                            else
                            {
                                player_targeting = (targeting_t *)component_add(game->player, COMPONENT_TARGETING);
                                player_targeting->type = TARGETING_LOOK;
                                player_targeting->x = player_position->x;
                                player_targeting->y = player_position->y;
                            }
                        }
                    }
                    break;
                    case 'm':
                    {
                        game->message_log_visible = !game->message_log_visible;
                    }
                    break;
                    case 'o':
                    {
                        action = ACTION_OPEN_DOOR;

                        game_log(game, NULL, TCOD_white, "Choose a direction");
                    }
                    break;
                    case 's':
                    {
                        if (key.lctrl)
                        {
                            game_save(game);
                        }
                    }
                    break;
                    case 't':
                    {
                        light_t *player_light = (light_t *)component_get(game->player, COMPONENT_LIGHT);

                        if (player_light)
                        {
                            game->should_update = true;

                            local bool torch = false;

                            torch = !torch;

                            if (torch)
                            {
                                player_light->radius = 10;
                                player_light->color = TCOD_light_amber;
                                player_light->flicker = true;
                                player_light->priority = LIGHT_PRIORITY_2;
                            }
                            else
                            {
                                player_light->radius = 5;
                                player_light->color = TCOD_white;
                                player_light->flicker = false;
                                player_light->priority = LIGHT_PRIORITY_0;
                            }
                        }
                    }
                    break;
                    case 'z':
                    {
                        caster_t *player_caster = (caster_t *)component_get(game->player, COMPONENT_CASTER);

                        if (player_caster)
                        {
                            spell_t *spell = &player_caster->spells[player_caster->current];

                            switch (spell->type)
                            {
                            case SPELL_HEAL_SELF:
                            {
                                game->should_update = true;

                                entity_cast_spell(game->player);
                            }
                            break;
                            case SPELL_INSTAKILL:
                            {
                                position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                                if (player_position)
                                {
                                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                                    if (player_targeting && player_targeting->type == TARGETING_ZAP)
                                    {
                                        game->should_update = true;

                                        entity_cast_spell(game->player);

                                        component_remove(game->player, COMPONENT_TARGETING);
                                    }
                                    else
                                    {
                                        player_targeting = (targeting_t *)component_add(game->player, COMPONENT_TARGETING);
                                        player_targeting->type = TARGETING_ZAP;

                                        bool target_found = false;

                                        {
                                            alignment_t *player_alignment = (alignment_t *)component_get(game->player, COMPONENT_ALIGNMENT);
                                            fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);

                                            if (player_alignment && player_fov)
                                            {
                                                for (void **iterator = TCOD_list_begin(game->maps[player_position->level].entities); iterator != TCOD_list_end(game->maps[player_position->level].entities); iterator++)
                                                {
                                                    entity_t *other = *iterator;

                                                    alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);
                                                    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                                                    if (other_alignment && other_position)
                                                    {
                                                        if (TCOD_map_is_in_fov(player_fov->fov_map, other_position->x, other_position->y) &&
                                                            other_alignment->type != player_alignment->type)
                                                        {
                                                            target_found = true;

                                                            player_targeting->x = other_position->x;
                                                            player_targeting->y = other_position->y;

                                                            break;
                                                        }
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
                                }
                            }
                            break;
                            }
                        }
                    }
                    break;
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

void game_update(game_t *game)
{
    game->turn_available = true;

    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

    if (player_position)
    {
        map_t *player_map = &game->maps[player_position->level];

        for (void **iterator = TCOD_list_begin(player_map->entities); iterator != TCOD_list_end(player_map->entities); iterator++)
        {
            entity_t *entity = *iterator;

            entity_update_flash(entity, false);
            entity_update_projectile(entity);
        }

        if (game->should_update)
        {
            game->should_update = false;
            game->turn++;

            TCOD_list_t lights = TCOD_list_new();

            for (void **iterator = TCOD_list_begin(player_map->entities); iterator != TCOD_list_end(player_map->entities); iterator++)
            {
                entity_t *entity = *iterator;

                entity_update_light(entity);

                light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);
                position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

                if (light && position)
                {
                    TCOD_list_push(lights, entity);
                }
            }

            for (void **iterator = TCOD_list_begin(player_map->entities); iterator != TCOD_list_end(player_map->entities); iterator++)
            {
                entity_t *entity = *iterator;

                entity_update_fov(entity, lights);
                entity_ai(entity);
            }

            TCOD_list_delete(lights);
        }
    }
}

void game_render(game_t *game)
{
    TCOD_console_set_default_background(NULL, TCOD_black);
    TCOD_console_set_default_foreground(NULL, TCOD_white);
    TCOD_console_clear(NULL);

    int message_log_x = 0;
    int message_log_height = console_height / 4;
    int message_log_y = console_height - message_log_height;
    int message_log_width = console_width;

    int panel_width = console_width / 2;
    int panel_x = console_width - panel_width;
    int panel_y = 0;
    int panel_height = console_height - (game->message_log_visible ? message_log_height : 0);

    fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);
    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

    if (player_fov && player_position)
    {
        map_t *player_map = &game->maps[player_position->level];

        int view_width = console_width - (game->panel_visible ? panel_width : 0);
        int view_height = console_height - (game->message_log_visible ? message_log_height : 0);
        int view_x = player_position->x - view_width / 2;
        int view_y = player_position->y - view_height / 2;

        if (view_width < MAP_WIDTH && view_height < MAP_HEIGHT)
        {
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
        }

        {
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

            for (void **iterator = TCOD_list_begin(player_map->entities); iterator != TCOD_list_end(player_map->entities); iterator++)
            {
                entity_t *entity = *iterator;

                appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

                if (appearance)
                {
                    TCOD_list_push(entities_by_layer[appearance->layer], entity);
                }

                light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);
                position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

                if (light && position)
                {
                    TCOD_list_push(lights_by_priority[light->priority], entity);
                }
            }

            {
                local TCOD_noise_t noise = NULL;
                if (noise == NULL)
                {
                    noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
                }

                local float noise_x = 0.0f;

                noise_x += 0.2f;
                float noise_dx = noise_x + 20.0f;
                float dx = TCOD_noise_get(noise, &noise_dx) * 0.5f;
                noise_dx += 30.0f;
                float dy = TCOD_noise_get(noise, &noise_dx) * 0.5f;
                float di = 0.2f * TCOD_noise_get(noise, &noise_x);

                for (int x = view_x; x < view_x + view_width; x++)
                {
                    for (int y = view_y; y < view_y + view_height; y++)
                    {
                        if (map_is_inside(x, y))
                        {
                            tile_t *tile = &player_map->tiles[x][y];
                            tile_info_t *tile_info = &game->tile_info[tile->type];

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

                                    if (light)
                                    {
                                        if (TCOD_map_is_in_fov(light->fov_map, x, y))
                                        {
                                            tile->seen = true;
                                        }
                                    }
                                }
                            }

                            TCOD_color_t color = game->tile_common.shadow_color;

                            if (TCOD_map_is_in_fov(player_fov->fov_map, x, y) || tile->seen)
                            {
                                for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
                                {
                                    for (void **iterator = TCOD_list_begin(lights_by_priority[i]); iterator != TCOD_list_end(lights_by_priority[i]); iterator++)
                                    {
                                        entity_t *entity = *iterator;

                                        light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);
                                        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

                                        if (light && position)
                                        {
                                            if (TCOD_map_is_in_fov(light->fov_map, x, y))
                                            {
                                                float r2 = pow(light->radius, 2);
                                                float d = pow(x - position->x + (light->flicker ? dx : 0), 2) + pow(y - position->y + (light->flicker ? dy : 0), 2);
                                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + (light->flicker ? di : 0));

                                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, light->color, l), l);
                                            }
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
                            TCOD_console_set_char(NULL, x - view_x, y - view_y, tile_info->glyph);
                        }
                    }
                }
            }

            for (int i = 0; i < NUM_LAYERS; i++)
            {
                for (void **iterator = TCOD_list_begin(entities_by_layer[i]); iterator != TCOD_list_end(entities_by_layer[i]); iterator++)
                {
                    entity_t *entity = *iterator;

                    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
                    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

                    if (appearance && position)
                    {
                        if (position->level == player_position->level &&
                            TCOD_map_is_in_fov(player_fov->fov_map, position->x, position->y))
                        {
                            TCOD_color_t color = appearance->color;

                            flash_t *flash = (flash_t *)component_get(entity, COMPONENT_FLASH);

                            if (flash)
                            {
                                color = TCOD_color_lerp(appearance->color, flash->color, flash->fade);
                            }

                            TCOD_console_set_char_foreground(NULL, position->x - view_x, position->y - view_y, color);
                            TCOD_console_set_char(NULL, position->x - view_x, position->y - view_y, appearance->glyph);
                        }
                    }
                }
            }

            for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
            {
                TCOD_list_delete(lights_by_priority[i]);
            }

            for (int i = 0; i < NUM_LAYERS; i++)
            {
                TCOD_list_delete(entities_by_layer[i]);
            }
        }

        {
            targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

            if (player_targeting)
            {
                TCOD_console_set_char_foreground(NULL, player_targeting->x - view_x, player_targeting->y - view_y, TCOD_red);
                TCOD_console_set_char(NULL, player_targeting->x - view_x, player_targeting->y - view_y, 'X');

                switch (player_targeting->type)
                {
                case TARGETING_LOOK:
                {
                    tile_t *tile = &game->maps[player_position->level].tiles[player_targeting->x][player_targeting->y];

                    if (TCOD_map_is_in_fov(player_fov->fov_map, player_targeting->x, player_targeting->y))
                    {
                        entity_t *entity = TCOD_list_peek(tile->entities);
                        appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

                        if (entity == NULL && appearance == NULL)
                        {
                            TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, game->tile_info[tile->type].name);
                        }
                        else
                        {
                            TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, appearance->name);
                        }
                    }
                    else
                    {
                        if (tile->seen)
                        {
                            TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, "%s (remembered)", game->tile_info[tile->type].name);
                        }
                        else
                        {
                            TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, "Unknown");
                        }
                    }
                    break;
                }
                }
            }
        }
    }

    if (game->message_log_visible)
    {
        local TCOD_console_t message_log = NULL;
        if (message_log == NULL)
        {
            message_log = TCOD_console_new(console_width, console_height);
        }

        TCOD_console_set_default_background(message_log, TCOD_black);
        TCOD_console_set_default_foreground(message_log, TCOD_white);
        TCOD_console_clear(message_log);

        int y = 1;
        for (void **i = TCOD_list_begin(game->messages); i != TCOD_list_end(game->messages); i++)
        {
            message_t *message = *i;

            TCOD_console_set_default_foreground(message_log, message->color);
            TCOD_console_print(message_log, message_log_x + 1, y, message->text);

            y++;
        }

        TCOD_console_set_default_foreground(message_log, TCOD_white);
        TCOD_console_print_frame(message_log, 0, 0, message_log_width, message_log_height, false, TCOD_BKGND_SET, "Log");

        TCOD_console_blit(message_log, 0, 0, message_log_width, message_log_height, NULL, message_log_x, message_log_y, 1, 1);
    }

    if (game->panel_visible)
    {
        local TCOD_console_t panel = NULL;
        if (panel == NULL)
        {
            panel = TCOD_console_new(console_width, console_height);
        }

        TCOD_console_set_default_background(panel, TCOD_black);
        TCOD_console_set_default_foreground(panel, TCOD_white);
        TCOD_console_clear(panel);

        switch (game->current_panel)
        {
        case PANEL_CHARACTER:
        {
            TCOD_console_set_default_foreground(panel, TCOD_white);
            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Character");
        }
        break;
        case PANEL_INVENTORY:
        {
            TCOD_console_set_default_foreground(panel, TCOD_white);
            TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Inventory");
        }
        break;
        }

        TCOD_console_blit(panel, 0, 0, panel_width, panel_height, NULL, panel_x, panel_y, 1, 1);
    }

    TCOD_console_print(NULL, 0, 0, "Turn: %d", game->turn);

    TCOD_console_flush();
}

void game_log(game_t *game, position_t *position, TCOD_color_t color, char *text, ...)
{
    fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);
    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

    if (player_fov && player_position)
    {
        if (!position ||
            (position->level == player_position->level &&
             ((position->x == player_position->x && position->y == player_position->y) ||
              TCOD_map_is_in_fov(player_fov->fov_map, position->x, position->y))))
        {
            char buffer[128];

            va_list ap;
            va_start(ap, text);
            vsprintf(buffer, text, ap);
            va_end(ap);

            char *line_begin = buffer;
            char *line_end;

            do
            {
                if (TCOD_list_size(game->messages) == (console_height / 4) - 2)
                {
                    message_t *message = TCOD_list_get(game->messages, 0);

                    TCOD_list_remove(game->messages, message);

                    message_destroy(message);
                }

                line_end = strchr(line_begin, '\n');

                if (line_end)
                {
                    *line_end = '\0';
                }

                message_t *message = message_create(line_begin, color);

                TCOD_list_push(game->messages, message);

                line_begin = line_end + 1;
            } while (line_end);
        }
    }
}

void game_reset(game_t *game)
{
    for (int i = 0; i < NUM_ENTITIES; i++)
    {
        entity_t *entity = &game->entities[i];

        entity_reset(entity);

        for (int j = 0; j < NUM_COMPONENTS; j++)
        {
            component_t *component = &game->components[j][i];

            component_reset(component);
        }
    }

    for (int i = 0; i < NUM_MAPS; i++)
    {
        map_t *map = &game->maps[i];

        map_reset(map);
    }

    for (void **iterator = TCOD_list_begin(game->messages); iterator != TCOD_list_end(game->messages); iterator++)
    {
        message_t *message = *iterator;

        message_destroy(message);
    }

    TCOD_list_delete(game->messages);
}
