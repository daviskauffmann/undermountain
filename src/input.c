#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

void input_init(void)
{
}

game_input_t input_handle(void)
{
    game_input_t input = GAME_INPUT_TICK;

    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    mouse_x = mouse.cx;
    mouse_y = mouse.cy;
    mouse_tile_x = mouse.cx + view_x;
    mouse_tile_y = mouse.cy + view_y;

    interactions_t default_interactions = {
        .light_on = true,
        .light_off = true,
        .attack = true,
        .take_item = true,
        .take_items = true};

    switch (ev)
    {
    case TCOD_EVENT_MOUSE_PRESS:
    {
        if (mouse.lbutton)
        {
            if (tooltip_visible)
            {
                if (tooltip_is_inside(mouse_x, mouse_y))
                {
                    tooltip_option_t *selected = NULL;

                    int y = 1;
                    for (void **i = TCOD_list_begin(tooltip_options); i != TCOD_list_end(tooltip_options); i++)
                    {
                        tooltip_option_t *option = *i;

                        if (mouse_y == y + tooltip_y)
                        {
                            selected = option;

                            break;
                        }
                        else
                        {
                            y++;
                        }
                    }

                    if (selected != NULL)
                    {
                        input = GAME_INPUT_TURN;

                        selected->fn(selected->data);

                        tooltip_hide();
                    }
                }
                else
                {
                    input = GAME_INPUT_DRAW;

                    tooltip_hide();
                }
            }
            else
            {
                if (view_is_inside(mouse_x, mouse_y))
                {
                    if (TCOD_map_is_in_fov(player->fov_map, mouse_tile_x, mouse_tile_y))
                    {
                        actor_target_set(player, mouse_tile_x, mouse_tile_y, default_interactions);
                    }
                    else
                    {
                        interactions_t interactions = {
                            .light_on = false,
                            .light_off = false,
                            .attack = false,
                            .take_item = false,
                            .take_items = false};

                        actor_target_set(player, mouse_tile_x, mouse_tile_y, interactions);
                    }
                }
            }
        }
        else if (mouse.rbutton)
        {
            if (view_is_inside(mouse_x, mouse_y))
            {
                input = GAME_INPUT_DRAW;

                tooltip_show(mouse_x, mouse_y);

                tooltip_data_t data = {
                    .tile_x = mouse_tile_x,
                    .tile_y = mouse_tile_y};

                tile_t *tile = &player->map->tiles[data.tile_x][data.tile_y];

                tooltip_options_add("Move", &tooltip_move, data);

                if (TCOD_map_is_in_fov(player->fov_map, data.tile_x, data.tile_y))
                {
                    if (tile->light != NULL)
                    {
                        if (tile->light->on)
                        {
                            tooltip_options_add("Turn Off", &tooltip_light_off, data);
                        }
                        else
                        {
                            tooltip_options_add("Turn On", &tooltip_light_on, data);
                        }
                    }

                    if (tile->actor != NULL)
                    {
                        if (tile->actor == player)
                        {
                            tooltip_options_add("Character", &tooltip_move, data);
                            tooltip_options_add("Inventory", &tooltip_move, data);
                        }
                        else
                        {
                            tooltip_options_add("Attack", &tooltip_attack, data);
                        }
                    }

                    if (TCOD_list_peek(tile->items))
                    {
                        tooltip_options_add("Take Item", &tooltip_take_item, data);
                    }

                    if (TCOD_list_size(tile->items) > 1)
                    {
                        tooltip_options_add("Take All", &tooltip_take_items, data);
                    }
                }
            }
            else if (panel_is_inside(mouse_x, mouse_y))
            {
                switch (content)
                {
                case CONTENT_INVENTORY:
                {
                    item_t *selected = NULL;

                    int y = 1;
                    for (void **i = TCOD_list_begin(player->items); i != TCOD_list_end(player->items); i++)
                    {
                        item_t *item = *i;

                        if (mouse_y == y + panel_y - content_scroll[content])
                        {
                            selected = item;

                            break;
                        }
                        else
                        {
                            y++;
                        }
                    }

                    if (selected != NULL)
                    {
                        input = GAME_INPUT_DRAW;

                        tooltip_show(panel_x + 1, y - content_scroll[content]);

                        tooltip_data_t data = {
                            .item = selected};

                        tooltip_options_add("Drop", &tooltip_drop_item, data);
                    }

                    break;
                }
                }
            }
        }
        else if (mouse.wheel_down)
        {
            if (msg_is_inside(mouse_x, mouse_y))
            {
                // scroll message log down
            }
            else if (panel_is_inside(mouse_x, mouse_y))
            {
                input = GAME_INPUT_DRAW;

                panel_content_scroll_down();
            }
        }
        else if (mouse.wheel_up)
        {
            if (msg_is_inside(mouse_x, mouse_y))
            {
                // scroll message log down
            }
            else if (panel_is_inside(mouse_x, mouse_y))
            {
                input = GAME_INPUT_DRAW;

                panel_content_scroll_up();
            }
        }

        break;
    }
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            input = GAME_INPUT_QUIT;

            break;
        }
        case TCODK_CHAR:
        {
            switch (key.c)
            {
            case ',':
            {
                if (key.shift)
                {
                    tile_t *tile = &player->map->tiles[player->x][player->y];

                    if (tile->type == TILE_TYPE_STAIR_UP)
                    {
                        if (current_map_index > 0)
                        {
                            input = GAME_INPUT_TURN;

                            current_map_index--;

                            map_t *new_map = TCOD_list_get(maps, current_map_index);

                            TCOD_list_remove(player->map->actors, player);
                            TCOD_list_push(new_map->actors, player);

                            player->map->tiles[player->x][player->y].actor = NULL;
                            new_map->tiles[new_map->stair_down_x][new_map->stair_down_y].actor = player;

                            player->map = new_map;
                            player->x = new_map->stair_down_x;
                            player->y = new_map->stair_down_y;
                        }
                        else
                        {
                            input = GAME_INPUT_QUIT;
                        }
                    }
                }

                break;
            }
            case '.':
            {
                if (key.shift)
                {
                    tile_t *tile = &player->map->tiles[player->x][player->y];

                    if (tile->type == TILE_TYPE_STAIR_DOWN)
                    {
                        input = GAME_INPUT_TURN;

                        current_map_index++;

                        map_t *new_map;

                        if (TCOD_list_size(maps) == current_map_index)
                        {
                            new_map = map_create();

                            TCOD_list_push(maps, new_map);
                        }
                        else
                        {
                            new_map = TCOD_list_get(maps, current_map_index);
                        }

                        TCOD_list_remove(player->map->actors, player);
                        TCOD_list_push(new_map->actors, player);

                        player->map->tiles[player->x][player->y].actor = NULL;
                        new_map->tiles[new_map->stair_up_x][new_map->stair_up_y].actor = player;

                        player->map = new_map;
                        player->x = new_map->stair_up_x;
                        player->y = new_map->stair_up_y;
                    }
                }

                break;
            }
            case 'c':
            {
                input = GAME_INPUT_DRAW;

                panel_toggle(CONTENT_CHARACTER);

                break;
            }
            case 'i':
            {
                input = GAME_INPUT_DRAW;

                panel_toggle(CONTENT_INVENTORY);

                break;
            }
            case 'l':
            {
                if (key.lctrl)
                {
                    input = GAME_INPUT_DRAW;

                    game_uninit();
                    game_init();
                    game_load();
                }
                else
                {
                    input = GAME_INPUT_DRAW;

                    msg_visible = !msg_visible;
                }

                break;
            }
            case 'r':
            {
                input = GAME_INPUT_DRAW;

                game_uninit();
                game_init();
                game_new();

                break;
            }
            case 's':
            {
                if (key.lctrl)
                {
                    input = GAME_INPUT_DRAW;

                    game_save();
                }

                break;
            }
            case 't':
            {
                input = GAME_INPUT_DRAW;

                if (player->torch)
                {
                    player->light = true;
                    player->torch = false;
                    player->fov_radius = 5;
                }
                else
                {
                    player->light = false;
                    player->torch = true;
                    player->fov_radius = 10;
                }

                actor_calc_fov(player);

                break;
            }
            case 'y':
            {
                input = GAME_INPUT_DRAW;

                player->torch_color = TCOD_color_RGB(TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255));

                break;
            }
            }

            break;
        }
        case TCODK_KP1:
        {
            input = GAME_INPUT_TURN;

            actor_move(player, player->x - 1, player->y + 1, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP2:
        case TCODK_DOWN:
        {
            input = GAME_INPUT_TURN;

            actor_move(player, player->x, player->y + 1, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP3:
        {
            input = GAME_INPUT_TURN;

            actor_move(player, player->x + 1, player->y + 1, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP4:
        case TCODK_LEFT:
        {
            input = GAME_INPUT_TURN;

            actor_move(player, player->x - 1, player->y, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP5:
        {
            input = GAME_INPUT_TURN;

            tooltip_hide();

            break;
        }
        case TCODK_KP6:
        case TCODK_RIGHT:
        {
            input = GAME_INPUT_TURN;

            actor_move(player, player->x + 1, player->y, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP7:
        {
            input = GAME_INPUT_TURN;

            actor_move(player, player->x - 1, player->y - 1, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP8:
        case TCODK_UP:
        {
            input = GAME_INPUT_TURN;

            actor_move(player, player->x, player->y - 1, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP9:
        {
            input = GAME_INPUT_TURN;

            actor_move(player, player->x + 1, player->y - 1, default_interactions);

            tooltip_hide();

            break;
        }
        }

        break;
    }
    }

    if (player->target)
    {
        input = GAME_INPUT_TURN;

        actor_target_process(player);
    }

    return input;
}

void input_uninit(void)
{
}