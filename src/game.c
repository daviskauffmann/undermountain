#include <libtcod.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "config.h"
#include "game.h"
#include "utils.h"

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

    game->turn_available = true;
    game->should_update = false;
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

    game->turn = 0;
    game->should_update = true;

    {
        entity_t *entity = entity_create(game);
        game->player = entity;

        alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
        alignment->type = ALIGNMENT_GOOD;

        appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
        appearance->name = "Blinky";
        appearance->glyph = '@';
        appearance->color = TCOD_white;
        appearance->layer = LAYER_1;

        component_add(entity, COMPONENT_CASTER);

        fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
        fov->radius = 1;

        health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
        health->max = 20;
        health->current = health->max;

        component_add(entity, COMPONENT_INVENTORY);

        light_t *light = (light_t *)component_add(entity, COMPONENT_LIGHT);
        light->radius = 5;
        light->color = TCOD_white;
        light->flicker = false;
        light->priority = LIGHT_PRIORITY_0;

        position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
        position->level = 0;
        map_t *map = &game->maps[position->level];
        TCOD_list_push(map->entities, entity);
        position->x = map->stair_up_x;
        position->y = map->stair_up_y;
        tile_t *tile = &map->tiles[position->x][position->y];
        TCOD_list_push(tile->entities, entity);

        component_add(entity, COMPONENT_SOLID);
    }

    {
        entity_t *entity = entity_create(game);

        ai_t *ai = (ai_t *)component_add(entity, COMPONENT_AI);
        ai->energy = 1.0f;
        ai->energy_per_turn = 0.5f;
        ai->follow_target = game->player;

        alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
        alignment->type = ALIGNMENT_GOOD;

        appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
        appearance->name = "Spot";
        appearance->glyph = 'd';
        appearance->color = TCOD_white;
        appearance->layer = LAYER_1;

        fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
        fov->radius = 1;

        health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
        health->max = 20;
        health->current = health->max;

        light_t *light = (light_t *)component_add(entity, COMPONENT_LIGHT);
        light->radius = 5;
        light->color = TCOD_white;
        light->flicker = false;
        light->priority = LIGHT_PRIORITY_0;

        position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
        position->level = 0;
        map_t *map = &game->maps[position->level];
        TCOD_list_push(map->entities, entity);
        position->x = map->stair_up_x + 1;
        position->y = map->stair_up_y;
        tile_t *tile = &map->tiles[position->x][position->y];
        TCOD_list_push(tile->entities, entity);

        component_add(entity, COMPONENT_SOLID);
    }

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

                TCOD_list_push(game->maps[position->level].entities, entity);
                TCOD_list_push(game->maps[position->level].tiles[position->x][position->y].entities, entity);
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
        game->turn_available = false;

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
            entity_t *entity = game->player;

            switch (ev)
            {
            case TCOD_EVENT_KEY_PRESS:
            {
                switch (key.vk)
                {
                case TCODK_KP1:
                {
                    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                    targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                    if (targeting)
                    {
                        targeting->x--;
                        targeting->y++;
                    }
                    else if (position)
                    {
                        game->should_update = true;

                        int x = position->x - 1;
                        int y = position->y + 1;

                        if (key.lctrl)
                        {
                            entity_swing(entity, x, y);
                        }
                        else
                        {
                            entity_move(entity, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP2:
                {
                    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                    targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                    if (targeting)
                    {
                        targeting->y++;
                    }
                    else if (position)
                    {
                        game->should_update = true;

                        int x = position->x;
                        int y = position->y + 1;

                        if (key.lctrl)
                        {
                            entity_swing(entity, x, y);
                        }
                        else
                        {
                            entity_move(entity, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP3:
                {
                    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                    targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                    if (targeting)
                    {
                        targeting->x++;
                        targeting->y++;
                    }
                    else if (position)
                    {
                        game->should_update = true;

                        int x = position->x + 1;
                        int y = position->y + 1;

                        if (key.lctrl)
                        {
                            entity_swing(entity, x, y);
                        }
                        else
                        {
                            entity_move(entity, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP4:
                {
                    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                    targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                    if (targeting)
                    {
                        targeting->x--;
                        targeting->y;
                    }
                    else if (position)
                    {
                        game->should_update = true;

                        int x = position->x - 1;
                        int y = position->y;

                        if (key.lctrl)
                        {
                            entity_swing(entity, x, y);
                        }
                        else
                        {
                            entity_move(entity, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP6:
                {
                    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                    targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                    if (targeting)
                    {
                        targeting->x++;
                        targeting->y;
                    }
                    else if (position)
                    {
                        game->should_update = true;

                        int x = position->x + 1;
                        int y = position->y;

                        if (key.lctrl)
                        {
                            entity_swing(entity, x, y);
                        }
                        else
                        {
                            entity_move(entity, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP7:
                {
                    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                    targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                    if (targeting)
                    {
                        targeting->x--;
                        targeting->y--;
                    }
                    else if (position)
                    {
                        game->should_update = true;

                        int x = position->x - 1;
                        int y = position->y - 1;

                        if (key.lctrl)
                        {
                            entity_swing(entity, x, y);
                        }
                        else
                        {
                            entity_move(entity, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP8:
                {
                    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                    targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                    if (targeting)
                    {
                        targeting->x;
                        targeting->y--;
                    }
                    else if (position)
                    {
                        game->should_update = true;

                        int x = position->x;
                        int y = position->y - 1;

                        if (key.lctrl)
                        {
                            entity_swing(entity, x, y);
                        }
                        else
                        {
                            entity_move(entity, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP9:
                {
                    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
                    targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                    if (targeting)
                    {
                        targeting->x++;
                        targeting->y--;
                    }
                    else if (position)
                    {
                        game->should_update = true;

                        int x = position->x + 1;
                        int y = position->y - 1;

                        if (key.lctrl)
                        {
                            entity_swing(entity, x, y);
                        }
                        else
                        {
                            entity_move(entity, x, y);
                        }
                    }
                }
                break;
                case TCODK_CHAR:
                {
                    switch (key.c)
                    {
                    case 'c':
                    {
                        game->panel_visible = !game->panel_visible;
                    }
                    break;
                    case 'f':
                    {
                        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

                        if (position)
                        {
                            targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                            if (targeting && targeting->type == TARGETING_SHOOT)
                            {
                                game->should_update = true;

                                entity_shoot(entity, targeting->x, targeting->y);

                                component_remove(entity, COMPONENT_TARGETING);
                            }
                            else
                            {
                                targeting = (targeting_t *)component_add(entity, COMPONENT_TARGETING);
                                targeting->type = TARGETING_SHOOT;

                                bool target_found = false;

                                {
                                    alignment_t *alignment = (alignment_t *)component_get(entity, COMPONENT_ALIGNMENT);
                                    fov_t *fov = (fov_t *)component_get(entity, COMPONENT_FOV);

                                    if (alignment && fov)
                                    {
                                        for (void **iterator = TCOD_list_begin(game->maps[position->level].entities); iterator != TCOD_list_end(game->maps[position->level].entities); iterator++)
                                        {
                                            entity_t *other = *iterator;

                                            alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);
                                            position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                                            if (other_alignment && other_position)
                                            {
                                                if (TCOD_map_is_in_fov(fov->fov_map, other_position->x, other_position->y) &&
                                                    other_alignment->type != alignment->type)
                                                {
                                                    target_found = true;

                                                    targeting->x = other_position->x;
                                                    targeting->y = other_position->y;

                                                    break;
                                                }
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
                    }
                    break;
                    case 'g':
                    {
                        inventory_t *inventory = (inventory_t *)component_get(entity, COMPONENT_INVENTORY);
                        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

                        if (inventory && position)
                        {
                            tile_t *tile = &game->maps[position->level].tiles[position->x][position->y];

                            bool item_found = false;

                            for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
                            {
                                entity_t *other = *iterator;

                                pickable_t *pickable = (pickable_t *)component_get(other, COMPONENT_PICKABLE);

                                if (pickable != NULL)
                                {
                                    game->should_update = true;

                                    item_found = true;

                                    entity_pick(entity, other);

                                    break;
                                }
                            }

                            if (!item_found)
                            {
                                game_log(entity->game, position, TCOD_white, "There is nothing here!");
                            }
                        }
                    }
                    break;
                    case 'i':
                    {
                        game->panel_visible = !game->panel_visible;
                    }
                    break;
                    case 'l':
                    {
                        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

                        if (position)
                        {
                            targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                            if (targeting)
                            {
                                component_remove(entity, COMPONENT_TARGETING);
                            }
                            else
                            {
                                targeting = (targeting_t *)component_add(entity, COMPONENT_TARGETING);
                                targeting->type = TARGETING_LOOK;
                                targeting->x = position->x;
                                targeting->y = position->y;
                            }
                        }
                    }
                    break;
                    case 'm':
                    {
                        game->message_log_visible = !game->message_log_visible;
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
                        light_t *light = (light_t *)component_get(entity, COMPONENT_LIGHT);

                        if (light)
                        {
                            game->should_update = true;

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
                    }
                    break;
                    case 'z':
                    {
                        caster_t *caster = (caster_t *)component_get(entity, COMPONENT_CASTER);

                        if (caster != NULL)
                        {
                            spell_t *spell = &caster->spells[caster->current];

                            switch (spell->type)
                            {
                            case SPELL_HEAL_SELF:
                            {
                                game->should_update = true;

                                entity_cast_spell(entity);
                            }
                            break;
                            case SPELL_INSTAKILL:
                            {
                                position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

                                if (position)
                                {
                                    targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

                                    if (targeting && targeting->type == TARGETING_ZAP)
                                    {
                                        game->should_update = true;

                                        entity_cast_spell(entity);

                                        component_remove(entity, COMPONENT_TARGETING);
                                    }
                                    else
                                    {
                                        targeting = (targeting_t *)component_add(entity, COMPONENT_TARGETING);
                                        targeting->type = TARGETING_ZAP;

                                        bool target_found = false;

                                        {
                                            alignment_t *alignment = (alignment_t *)component_get(entity, COMPONENT_ALIGNMENT);
                                            fov_t *fov = (fov_t *)component_get(entity, COMPONENT_FOV);

                                            if (alignment && fov)
                                            {
                                                for (void **iterator = TCOD_list_begin(game->maps[position->level].entities); iterator != TCOD_list_end(game->maps[position->level].entities); iterator++)
                                                {
                                                    entity_t *other = *iterator;

                                                    alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);
                                                    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                                                    if (other_alignment && other_position)
                                                    {
                                                        if (TCOD_map_is_in_fov(fov->fov_map, other_position->x, other_position->y) &&
                                                            other_alignment->type != alignment->type)
                                                        {
                                                            target_found = true;

                                                            targeting->x = other_position->x;
                                                            targeting->y = other_position->y;

                                                            break;
                                                        }
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
            entity_t *current = *iterator;

            flash_t *current_flash = (flash_t *)component_get(current, COMPONENT_FLASH);

            if (current_flash)
            {
                current_flash->fade -= (1.0f / FPS) / 0.25f;

                if (current_flash->fade <= 0)
                {
                    component_remove(current, COMPONENT_FLASH);
                }

                {
                    fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);

                    position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                    if (player_fov && current_position)
                    {
                        if (TCOD_map_is_in_fov(player_fov->fov_map, current_position->x, current_position->y))
                        {
                            game->turn_available = false;
                        }
                    }
                }
            }
        }

        if (game->should_update)
        {
            game->should_update = false;
            game->turn++;

            TCOD_list_t lights = TCOD_list_new();

            for (void **iterator = TCOD_list_begin(player_map->entities); iterator != TCOD_list_end(player_map->entities); iterator++)
            {
                entity_t *current = *iterator;

                light_t *current_light = (light_t *)component_get(current, COMPONENT_LIGHT);
                position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                if (current_light && current_position)
                {
                    if (current_light->fov_map != NULL)
                    {
                        TCOD_map_delete(current_light->fov_map);
                    }

                    map_t *current_map = &game->maps[current_position->level];

                    current_light->fov_map = map_to_fov_map(current_map, current_position->x, current_position->y, current_light->radius);

                    TCOD_list_push(lights, current);
                }
            }

            for (void **iterator = TCOD_list_begin(player_map->entities); iterator != TCOD_list_end(player_map->entities); iterator++)
            {
                entity_t *current = *iterator;

                position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                if (current_position)
                {
                    map_t *current_map = &game->maps[current_position->level];

                    fov_t *current_fov = (fov_t *)component_get(current, COMPONENT_FOV);

                    if (current_fov)
                    {
                        if (current_fov->fov_map != NULL)
                        {
                            TCOD_map_delete(current_fov->fov_map);
                        }

                        current_fov->fov_map = map_to_fov_map(current_map, current_position->x, current_position->y, current_fov->radius);

                        if (current == game->player)
                        {
                            TCOD_map_t los_map = map_to_fov_map(current_map, current_position->x, current_position->y, 0);

                            for (int x = 0; x < MAP_WIDTH; x++)
                            {
                                for (int y = 0; y < MAP_HEIGHT; y++)
                                {
                                    if (TCOD_map_is_in_fov(los_map, x, y))
                                    {
                                        tile_t *tile = &current_map->tiles[x][y];

                                        for (void **iterator = TCOD_list_begin(lights); iterator != TCOD_list_end(lights); iterator++)
                                        {
                                            entity_t *other = *iterator;

                                            light_t *other_light = (light_t *)component_get(other, COMPONENT_LIGHT);
                                            position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                                            if (other_light && other_position)
                                            {
                                                if (TCOD_map_is_in_fov(other_light->fov_map, x, y))
                                                {
                                                    TCOD_map_set_in_fov(current_fov->fov_map, x, y, true);
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            TCOD_map_delete(los_map);
                        }
                    }

                    ai_t *current_ai = (ai_t *)component_get(current, COMPONENT_AI);

                    if (current_ai)
                    {
                        current_ai->energy += current_ai->energy_per_turn;

                        while (current_ai->energy >= 1.0f)
                        {
                            current_ai->energy -= 1.0f;

                            bool target_found = false;

                            alignment_t *current_alignment = (alignment_t *)component_get(current, COMPONENT_ALIGNMENT);

                            if (current_alignment && current_fov)
                            {
                                for (void **iterator = TCOD_list_begin(current_map->entities); iterator != TCOD_list_end(current_map->entities); iterator++)
                                {
                                    entity_t *other = *iterator;

                                    alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);
                                    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                                    if (other_alignment && other_position)
                                    {
                                        if (TCOD_map_is_in_fov(current_fov->fov_map, other_position->x, other_position->y) &&
                                            other_alignment->type != current_alignment->type)
                                        {
                                            target_found = true;

                                            if (distance(current_position->x, current_position->y, other_position->x, other_position->y) < 2.0f)
                                            {
                                                entity_attack(current, other);
                                            }
                                            else
                                            {
                                                entity_path_towards(current, other_position->x, other_position->y);
                                            }

                                            break;
                                        }
                                    }
                                }
                            }

                            if (current_fov && current_ai->follow_target)
                            {
                                position_t *follow_position = (position_t *)component_get(current_ai->follow_target, COMPONENT_POSITION);

                                if (follow_position)
                                {
                                    if (!TCOD_map_is_in_fov(current_fov->fov_map, follow_position->x, follow_position->y) ||
                                        distance(current_position->x, current_position->y, follow_position->x, follow_position->y) > 5.0f)
                                    {
                                        target_found = true;

                                        entity_path_towards(current, follow_position->x, follow_position->y);
                                    }
                                }
                            }

                            if (!target_found)
                            {
                                entity_move_random(current);
                            }
                        }
                    }
                }
            }

            TCOD_list_delete(lights);
        }
    }
}

void game_log(game_t *game, position_t *position, TCOD_color_t color, char *text, ...)
{
    fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);
    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

    if (player_fov && player_position)
    {
        if (position == NULL ||
            (position->level == player_position->level &&
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
                entity_t *current = *iterator;

                appearance_t *current_appearance = (appearance_t *)component_get(current, COMPONENT_APPEARANCE);

                if (current_appearance)
                {
                    TCOD_list_push(entities_by_layer[current_appearance->layer], current);
                }

                light_t *current_light = (light_t *)component_get(current, COMPONENT_LIGHT);
                position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                if (current_light && current_position)
                {
                    TCOD_list_push(lights_by_priority[current_light->priority], current);
                }
            }

            {
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
                                    entity_t *current = *iterator;

                                    light_t *current_light = (light_t *)component_get(current, COMPONENT_LIGHT);

                                    if (current_light)
                                    {
                                        if (TCOD_map_is_in_fov(current_light->fov_map, x, y))
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
                                        entity_t *current = *iterator;

                                        light_t *current_light = (light_t *)component_get(current, COMPONENT_LIGHT);
                                        position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                                        if (current_light && current_position)
                                        {
                                            if (TCOD_map_is_in_fov(current_light->fov_map, x, y))
                                            {
                                                float r2 = pow(current_light->radius, 2);
                                                float d = pow(x - current_position->x + (current_light->flicker ? dx : 0), 2) + pow(y - current_position->y + (current_light->flicker ? dy : 0), 2);
                                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + (current_light->flicker ? di : 0));

                                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, current_light->color, l), l);
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
                    entity_t *current = *iterator;

                    appearance_t *current_appearance = (appearance_t *)component_get(current, COMPONENT_APPEARANCE);
                    position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                    if (current_appearance && current_position)
                    {
                        if (current_position->level == player_position->level &&
                            TCOD_map_is_in_fov(player_fov->fov_map, current_position->x, current_position->y))
                        {
                            TCOD_color_t color = current_appearance->color;

                            flash_t *current_flash = (flash_t *)component_get(current, COMPONENT_FLASH);

                            if (current_flash)
                            {
                                color = TCOD_color_lerp(current_appearance->color, current_flash->color, current_flash->fade);
                            }

                            TCOD_console_set_char_foreground(NULL, current_position->x - view_x, current_position->y - view_y, color);
                            TCOD_console_set_char(NULL, current_position->x - view_x, current_position->y - view_y, current_appearance->glyph);
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

    if (game->message_log_visible)
    {
        static TCOD_console_t message_log = NULL;

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
        static TCOD_console_t panel = NULL;

        if (panel == NULL)
        {
            panel = TCOD_console_new(console_height, console_width);
        }

        TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Panel");

        TCOD_console_blit(panel, 0, 0, panel_width, panel_height, NULL, panel_x, panel_y, 1, 1);
    }

    TCOD_console_print(NULL, 0, 0, "Turn: %d", game->turn);

    TCOD_console_flush();
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