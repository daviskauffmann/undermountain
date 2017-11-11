#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

static void tooltip_option_move(tooltip_data_t data);
static void tooltip_option_stair_descend(tooltip_data_t data);
static void tooltip_option_stair_ascend(tooltip_data_t data);
static void tooltip_option_light_on(tooltip_data_t data);
static void tooltip_option_light_off(tooltip_data_t data);
static void tooltip_option_item_take(tooltip_data_t data);
static void tooltip_option_item_take_all(tooltip_data_t data);
static void tooltip_option_item_drop(tooltip_data_t data);
static void tooltip_option_actor_attack(tooltip_data_t data);

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
        .descend = true,
        .ascend = true,
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
                    tooltip_option_t *selected = tooltip_get_selected();

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
                if (view_is_inside(mouse_x, mouse_y) && map_is_inside(mouse_tile_x, mouse_tile_y))
                {
                    if (TCOD_map_is_in_fov(player->fov_map, mouse_tile_x, mouse_tile_y))
                    {
                        actor_target_set(player, mouse_tile_x, mouse_tile_y, default_interactions);
                    }
                    else
                    {
                        interactions_t interactions = {
                            .descend = false,
                            .ascend = false,
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
            if (view_is_inside(mouse_x, mouse_y) && map_is_inside(mouse_tile_x, mouse_tile_y))
            {
                input = GAME_INPUT_DRAW;

                tooltip_show(mouse_x, mouse_y);

                tooltip_data_t data = {
                    .tile_x = mouse_tile_x,
                    .tile_y = mouse_tile_y};

                tile_t *tile = &player->map->tiles[data.tile_x][data.tile_y];

                tooltip_options_add("Move", &tooltip_option_move, data);

                if (tile->type == TILE_TYPE_STAIR_DOWN)
                {
                    tooltip_options_add("Descend", &tooltip_option_stair_descend, data);
                }

                if (tile->type == TILE_TYPE_STAIR_UP)
                {
                    tooltip_options_add("Ascend", &tooltip_option_stair_ascend, data);
                }

                if (TCOD_map_is_in_fov(player->fov_map, data.tile_x, data.tile_y))
                {
                    if (tile->light != NULL)
                    {
                        if (tile->light->on)
                        {
                            tooltip_options_add("Turn Off", &tooltip_option_light_off, data);
                        }
                        else
                        {
                            tooltip_options_add("Turn On", &tooltip_option_light_on, data);
                        }
                    }

                    if (tile->actor != NULL)
                    {
                        if (tile->actor == player)
                        {
                            tooltip_options_add("Character", &tooltip_option_move, data);
                            tooltip_options_add("Inventory", &tooltip_option_move, data);
                        }
                        else if (tile->actor->ai == ai_monster)
                        {
                            tooltip_options_add("Attack", &tooltip_option_actor_attack, data);
                        }
                    }

                    if (TCOD_list_peek(tile->items))
                    {
                        tooltip_options_add("Take Item", &tooltip_option_item_take, data);
                    }

                    if (TCOD_list_size(tile->items) > 1)
                    {
                        tooltip_options_add("Take All", &tooltip_option_item_take_all, data);
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

                        if (mouse_x > panel_x && mouse_x < panel_x + strlen("{name}") + 1 && mouse_y == y + panel_y - content_scroll[content])
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

                        tooltip_options_add("Drop", &tooltip_option_item_drop, data);
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
                input = GAME_INPUT_TURN;

                if (player->light != ACTOR_LIGHT_TORCH)
                {
                    player->light = ACTOR_LIGHT_TORCH;
                }
                else
                {
                    player->light = ACTOR_LIGHT_DEFAULT;
                }

                actor_calc_fov(player);

                break;
            }
            }

            break;
        }
        case TCODK_KP1:
        {
            input = GAME_INPUT_TURN;

            actor_target_set(player, player->x - 1, player->y + 1, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP2:
        case TCODK_DOWN:
        {
            input = GAME_INPUT_TURN;

            actor_target_set(player, player->x, player->y + 1, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP3:
        {
            input = GAME_INPUT_TURN;

            actor_target_set(player, player->x + 1, player->y + 1, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP4:
        case TCODK_LEFT:
        {
            input = GAME_INPUT_TURN;

            actor_target_set(player, player->x - 1, player->y, default_interactions);

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

            actor_target_set(player, player->x + 1, player->y, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP7:
        {
            input = GAME_INPUT_TURN;

            actor_target_set(player, player->x - 1, player->y - 1, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP8:
        case TCODK_UP:
        {
            input = GAME_INPUT_TURN;

            actor_target_set(player, player->x, player->y - 1, default_interactions);

            tooltip_hide();

            break;
        }
        case TCODK_KP9:
        {
            input = GAME_INPUT_TURN;

            actor_target_set(player, player->x + 1, player->y - 1, default_interactions);

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

static void tooltip_option_move(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

static void tooltip_option_stair_descend(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = true,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

static void tooltip_option_stair_ascend(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = true,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

static void tooltip_option_light_on(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = true,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

static void tooltip_option_light_off(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = true,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

static void tooltip_option_item_take(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = true,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

static void tooltip_option_item_take_all(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = true};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

static void tooltip_option_item_drop(tooltip_data_t data)
{
    tile_t *tile = &player->map->tiles[player->x][player->y];

    TCOD_list_remove(player->items, data.item);
    TCOD_list_push(tile->items, data.item);
}

static void tooltip_option_actor_attack(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = true,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}