#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

#define AUTOMOVE_DELAY 0.05f

static bool automove;
static int automove_x;
static int automove_y;
static actor_t *automove_actor;
static move_actions_t automove_actions;

static void automove_set(int x, int y, move_actions_t actions);
static void automove_clear();

void input_init(void)
{
    automove_clear();
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

    move_actions_t default_actions = {
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
                if (mouse_x >= tooltip_x && mouse_x < tooltip_x + tooltip_width && mouse_y >= tooltip_y && mouse_y < tooltip_y + tooltip_height)
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
                        switch (selected->type)
                        {
                        case TOOLTIP_OPTION_TYPE_MOVE:
                        {
                            move_actions_t actions = {
                                .light_on = false,
                                .light_off = false,
                                .attack = false,
                                .take_item = false,
                                .take_items = false};

                            automove_set(tooltip_tile_x, tooltip_tile_y, actions);

                            break;
                        }
                        case TOOLTIP_OPTION_TYPE_ATTACK:
                        {
                            move_actions_t actions = {
                                .light_on = false,
                                .light_off = false,
                                .attack = true,
                                .take_item = false,
                                .take_items = false};

                            automove_set(tooltip_tile_x, tooltip_tile_y, actions);

                            break;
                        }
                        case TOOLTIP_OPTION_TYPE_TAKE_ITEM:
                        {
                            move_actions_t actions = {
                                .light_on = false,
                                .light_off = false,
                                .attack = false,
                                .take_item = true,
                                .take_items = false};

                            automove_set(tooltip_tile_x, tooltip_tile_y, actions);

                            break;
                        }
                        case TOOLTIP_OPTION_TYPE_TAKE_ITEMS:
                        {
                            move_actions_t actions = {
                                .light_on = false,
                                .light_off = false,
                                .attack = false,
                                .take_item = false,
                                .take_items = true};

                            automove_set(tooltip_tile_x, tooltip_tile_y, actions);

                            break;
                        }
                        case TOOLTIP_OPTION_TYPE_DROP_ITEM:
                        {
                            tile_t *tile = &player->map->tiles[player->x][player->y];

                            TCOD_list_remove(player->items, tooltip_item);
                            TCOD_list_push(tile->items, tooltip_item);

                            break;
                        }
                        case TOOLTIP_OPTION_TYPE_LIGHT_OFF:
                        {
                            move_actions_t actions = {
                                .light_on = false,
                                .light_off = true,
                                .attack = false,
                                .take_item = false,
                                .take_items = true};

                            automove_set(tooltip_tile_x, tooltip_tile_y, actions);

                            break;
                        }
                        case TOOLTIP_OPTION_TYPE_LIGHT_ON:
                        {
                            move_actions_t actions = {
                                .light_on = true,
                                .light_off = false,
                                .attack = false,
                                .take_item = false,
                                .take_items = true};

                            automove_set(tooltip_tile_x, tooltip_tile_y, actions);

                            break;
                        }
                        }

                        tooltip_hide();
                    }
                }
                else
                {
                    tooltip_hide();
                }
            }
            else
            {
                if (mouse_tile_x >= view_x && mouse_tile_x < view_x + view_width && mouse_tile_y >= view_y && mouse_tile_y < view_y + view_height)
                {
                    if (TCOD_map_is_in_fov(player->fov_map, mouse_tile_x, mouse_tile_y))
                    {
                        automove_set(mouse_tile_x, mouse_tile_y, default_actions);
                    }
                    else
                    {
                        move_actions_t actions = {
                            .light_on = false,
                            .light_off = false,
                            .attack = false,
                            .take_item = false,
                            .take_items = false};

                        automove_set(mouse_tile_x, mouse_tile_y, actions);
                    }
                }
                else if (mouse_x >= panel_x && mouse_x < panel_x + panel_width && mouse_y >= panel_y && mouse_y < panel_y + panel_height)
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

                            if (mouse_y == y + panel_y)
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
                            tooltip_show(mouse_tile_x, mouse_tile_y);

                            tooltip_item = selected;

                            tooltip_options_add("Drop", TOOLTIP_OPTION_TYPE_DROP_ITEM);
                        }

                        break;
                    }
                    }
                }
            }

            input = GAME_INPUT_DRAW;
        }
        else if (mouse.rbutton)
        {
            if (mouse_tile_x >= view_x && mouse_tile_x < view_x + view_width && mouse_tile_y >= view_y && mouse_tile_y < view_y + view_height)
            {
                if (tooltip_visible)
                {
                    tooltip_hide();
                }
                else
                {
                    tooltip_show(mouse_tile_x, mouse_tile_y);

                    tile_t *tile = &player->map->tiles[tooltip_tile_x][tooltip_tile_y];

                    tooltip_options_add("Move", TOOLTIP_OPTION_TYPE_MOVE);

                    if (TCOD_map_is_in_fov(player->fov_map, tooltip_tile_x, tooltip_tile_y))
                    {
                        if (tile->light != NULL)
                        {
                            if (tile->light->on)
                            {
                                tooltip_options_add("Turn Off", TOOLTIP_OPTION_TYPE_LIGHT_OFF);
                            }
                            else
                            {
                                tooltip_options_add("Turn On", TOOLTIP_OPTION_TYPE_LIGHT_ON);
                            }
                        }

                        if (tile->actor != NULL)
                        {
                            if (tile->actor == player)
                            {
                                tooltip_options_add("Character", TOOLTIP_OPTION_TYPE_MOVE);
                                tooltip_options_add("Inventory", TOOLTIP_OPTION_TYPE_MOVE);
                            }
                            else
                            {
                                tooltip_options_add("Attack", TOOLTIP_OPTION_TYPE_ATTACK);
                            }
                        }

                        if (TCOD_list_peek(tile->items))
                        {
                            tooltip_options_add("Take Item", TOOLTIP_OPTION_TYPE_TAKE_ITEM);
                        }

                        if (TCOD_list_size(tile->items) > 1)
                        {
                            tooltip_options_add("Take All", TOOLTIP_OPTION_TYPE_TAKE_ITEMS);
                        }
                    }
                }

                input = GAME_INPUT_DRAW;
            }
        }
        else if (mouse.wheel_down)
        {
            panel_content_scroll_down();

            input = GAME_INPUT_DRAW;
        }
        else if (mouse.wheel_up)
        {
            panel_content_scroll_up();

            input = GAME_INPUT_DRAW;
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
        case TCODK_PAGEDOWN:
        {
            panel_content_scroll_down();

            input = GAME_INPUT_DRAW;

            break;
        }
        case TCODK_PAGEUP:
        {
            panel_content_scroll_up();

            input = GAME_INPUT_DRAW;

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
                            current_map_index--;

                            map_t *new_map = TCOD_list_get(maps, current_map_index);

                            TCOD_list_remove(player->map->actors, player);
                            TCOD_list_push(new_map->actors, player);

                            player->map->tiles[player->x][player->y].actor = NULL;
                            new_map->tiles[new_map->stair_down_x][new_map->stair_down_y].actor = player;

                            player->map = new_map;
                            player->x = new_map->stair_down_x;
                            player->y = new_map->stair_down_y;

                            input = GAME_INPUT_TURN;
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

                        input = GAME_INPUT_TURN;
                    }
                }

                break;
            }
            case 'a':
            {
                sfx = !sfx;

                input = GAME_INPUT_DRAW;

                break;
            }
            case 'b':
            {
                input = GAME_INPUT_TICK;

                break;
            }
            case 'c':
            {
                panel_toggle(CONTENT_CHARACTER);

                input = GAME_INPUT_DRAW;

                break;
            }
            case 'i':
            {
                panel_toggle(CONTENT_INVENTORY);

                input = GAME_INPUT_DRAW;

                break;
            }
            case 'l':
            {
                if (key.lctrl)
                {
                    game_uninit();
                    game_init();
                    game_load();

                    input = GAME_INPUT_DRAW;
                }
                else
                {
                    msg_visible = !msg_visible;

                    input = GAME_INPUT_DRAW;
                }

                break;
            }
            case 'r':
            {
                game_uninit();
                game_init();
                game_new();

                input = GAME_INPUT_DRAW;

                break;
            }
            case 's':
            {
                if (key.lctrl)
                {
                    game_save();

                    input = GAME_INPUT_DRAW;
                }

                break;
            }
            case 't':
            {
                player->torch = !player->torch;

                if (player->torch)
                {
                    player->fov_radius *= 2;
                }
                else
                {
                    player->fov_radius /= 2;
                }

                actor_calc_fov(player);

                input = GAME_INPUT_DRAW;

                break;
            }
            case 'y':
            {
                torch_color = TCOD_color_RGB(TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255));

                input = GAME_INPUT_DRAW;

                break;
            }
            }

            break;
        }
        case TCODK_KP1:
        {
            automove_clear();

            tooltip_hide();

            input = actor_move(player, player->x - 1, player->y + 1, default_actions)
                        ? GAME_INPUT_TURN
                        : GAME_INPUT_DRAW;

            break;
        }
        case TCODK_KP2:
        case TCODK_DOWN:
        {
            automove_clear();

            tooltip_hide();

            input = actor_move(player, player->x, player->y + 1, default_actions)
                        ? GAME_INPUT_TURN
                        : GAME_INPUT_DRAW;

            break;
        }
        case TCODK_KP3:
        {
            automove_clear();

            tooltip_hide();

            input = actor_move(player, player->x + 1, player->y + 1, default_actions)
                        ? GAME_INPUT_TURN
                        : GAME_INPUT_DRAW;

            break;
        }
        case TCODK_KP4:
        case TCODK_LEFT:
        {
            automove_clear();

            tooltip_hide();

            input = actor_move(player, player->x - 1, player->y, default_actions)
                        ? GAME_INPUT_TURN
                        : GAME_INPUT_DRAW;

            break;
        }
        case TCODK_KP5:
        {
            automove_clear();

            tooltip_hide();

            input = GAME_INPUT_TURN;

            break;
        }
        case TCODK_KP6:
        case TCODK_RIGHT:
        {
            automove_clear();

            tooltip_hide();

            input = actor_move(player, player->x + 1, player->y, default_actions)
                        ? GAME_INPUT_TURN
                        : GAME_INPUT_DRAW;

            break;
        }
        case TCODK_KP7:
        {
            automove_clear();

            tooltip_hide();

            input = actor_move(player, player->x - 1, player->y - 1, default_actions)
                        ? GAME_INPUT_TURN
                        : GAME_INPUT_DRAW;

            break;
        }
        case TCODK_KP8:
        case TCODK_UP:
        {
            automove_clear();

            tooltip_hide();

            input = actor_move(player, player->x, player->y - 1, default_actions)
                        ? GAME_INPUT_TURN
                        : GAME_INPUT_DRAW;

            break;
        }
        case TCODK_KP9:
        {
            automove_clear();

            tooltip_hide();

            input = actor_move(player, player->x + 1, player->y - 1, default_actions)
                        ? GAME_INPUT_TURN
                        : GAME_INPUT_DRAW;

            break;
        }
        }

        break;
    }
    }

    static float automove_timer = 0.0f;
    static bool automove_ready = true;

    if (automove_ready && automove)
    {
        int x = automove_actor == NULL
                    ? automove_x
                    : automove_actor->x;
        int y = automove_actor == NULL
                    ? automove_y
                    : automove_actor->y;

        if (actor_move(player, x, y, automove_actions))
        {
            if (automove_actor != NULL && automove_actor->mark_for_delete)
            {
                automove_clear();
            }

            automove_ready = false;

            input = GAME_INPUT_TURN;
        }
        else
        {
            automove_clear();
        }
    }
    else
    {
        automove_timer += TCOD_sys_get_last_frame_length();

        if (automove_timer >= AUTOMOVE_DELAY)
        {
            automove_ready = true;

            automove_timer = 0.0f;
        }
    }

    return input;
}

static void automove_set(int x, int y, move_actions_t actions)
{
    tile_t *tile = &player->map->tiles[x][y];
    actor_t *actor = tile->actor;

    automove = true;
    automove_actor = actor;
    automove_x = x;
    automove_y = y;
    automove_actions = actions;
}

static void automove_clear()
{
    automove = false;
    automove_actor = NULL;
    automove_x = -1;
    automove_y = -1;
    move_actions_t actions = {
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};
    automove_actions = actions;
}

void input_uninit(void)
{
}