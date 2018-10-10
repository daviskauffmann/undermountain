#include <libtcod/libtcod.h>
#include <malloc.h>

#include "actor.h"
#include "game.h"
#include "input.h"
#include "ui.h"
#include "util.h"
#include "window.h"

#include "CMemleak.h"

static bool interact(struct game *game, struct input *input, int x, int y);
static void cb_should_update(struct game *game);
static bool tooltip_option_move(struct game *game, struct input *input, struct tooltip_data data);

struct input *input_create(void)
{
    struct input *input = calloc(1, sizeof(struct input));

    input->action = ACTION_NONE;
    input->targeting = TARGETING_NONE;
    input->target_x = -1;
    input->target_y = -1;
    input->automoving = false;
    input->automove_x = -1;
    input->automove_y = -1;

    return input;
}

#include <stdio.h>

void input_handle(struct input *input, struct game *game, struct ui *ui)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        input->automoving = false;

        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            if (ui->panel_visible)
            {
                ui->panel_visible = false;
            }
            else if (input->targeting)
            {
                input->targeting = false;
            }
            else
            {
                game->should_quit = true;
            }
        }
        break;
        case TCODK_ENTER:
        {
            if (key.lalt)
            {
                fullscreen = !fullscreen;

                TCOD_console_set_fullscreen(fullscreen);
            }
            else if (game->state == STATE_MENU)
            {
                switch (ui->menu_state)
                {
                case MENU_STATE_MAIN:
                {
                    switch (ui->menu_index)
                    {
                    case 0:
                    {
                        if (TCOD_sys_file_exists(SAVE_PATH))
                        {
                            game_load(game);
                        }
                        else
                        {
                            game_new(game);
                        }
                    }
                    break;
                    case 1:
                    {
                        ui->menu_state = MENU_STATE_ABOUT;
                    }
                    break;
                    case 2:
                    {
                        game->should_quit = true;
                    }
                    break;
                    }
                }
                break;
                case MENU_STATE_ABOUT:
                {
                    ui->menu_state = MENU_STATE_MAIN;
                }
                break;
                }
            }
        }
        break;
        case TCODK_PAGEDOWN:
        {
            if (game->state == STATE_PLAYING && ui->panel_visible)
            {
                struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

                if (panel_status->current_index < panel_status->max_index)
                {
                    panel_status->current_index++;
                }
            }
        }
        break;
        case TCODK_PAGEUP:
        {
            if (game->state == STATE_PLAYING && ui->panel_visible)
            {
                struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

                if (panel_status->current_index > 0)
                {
                    panel_status->current_index--;
                }
            }
        }
        break;
        case TCODK_KP1:
        {
            if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
            {
                if (input->targeting != TARGETING_NONE)
                {
                    input->target_x--;
                    input->target_y++;
                }
                else
                {
                    int x = game->player->x - 1;
                    int y = game->player->y + 1;

                    if (input->action == ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            game->should_update = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            game->should_update = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        game->should_update = interact(game, input, x, y);

                        input->action = ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP2:
        case TCODK_DOWN:
        {
            if (game->state == STATE_MENU && ui->menu_state == MENU_STATE_MAIN)
            {
                if (ui->menu_index < 2)
                {
                    ui->menu_index++;
                }
            }
            else if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
            {
                if (input->targeting != TARGETING_NONE)
                {
                    input->target_x;
                    input->target_y++;
                }
                else
                {
                    int x = game->player->x;
                    int y = game->player->y + 1;

                    if (input->action == ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            game->should_update = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            game->should_update = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        game->should_update = interact(game, input, x, y);

                        input->action = ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP3:
        {
            if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
            {
                if (input->targeting != TARGETING_NONE)
                {
                    input->target_x++;
                    input->target_y++;
                }
                else
                {
                    int x = game->player->x + 1;
                    int y = game->player->y + 1;

                    if (input->action == ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            game->should_update = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            game->should_update = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        game->should_update = interact(game, input, x, y);

                        input->action = ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP4:
        case TCODK_LEFT:
        {
            if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
            {
                if (input->targeting != TARGETING_NONE)
                {
                    input->target_x--;
                    input->target_y;
                }
                else
                {
                    int x = game->player->x - 1;
                    int y = game->player->y;

                    if (input->action == ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            game->should_update = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            game->should_update = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        game->should_update = interact(game, input, x, y);

                        input->action = ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP5:
        {
            if (game->state == STATE_PLAYING && game->turn_available)
            {
                game->should_update = true;
            }
        }
        break;
        case TCODK_KP6:
        case TCODK_RIGHT:
        {
            if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
            {
                if (input->targeting != TARGETING_NONE)
                {
                    input->target_x++;
                    input->target_y;
                }
                else
                {
                    int x = game->player->x + 1;
                    int y = game->player->y;

                    if (input->action == ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            game->should_update = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            game->should_update = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        game->should_update = interact(game, input, x, y);

                        input->action = ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP7:
        {
            if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
            {
                if (input->targeting != TARGETING_NONE)
                {
                    input->target_x--;
                    input->target_y--;
                }
                else
                {
                    int x = game->player->x - 1;
                    int y = game->player->y - 1;

                    if (input->action == ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            game->should_update = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            game->should_update = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        game->should_update = interact(game, input, x, y);

                        input->action = ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP8:
        case TCODK_UP:
        {
            if (game->state == STATE_MENU && ui->menu_state == MENU_STATE_MAIN)
            {
                if (ui->menu_index > 0)
                {
                    ui->menu_index--;
                }
            }
            else if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
            {
                if (input->targeting != TARGETING_NONE)
                {
                    input->target_x;
                    input->target_y--;
                }
                else
                {
                    int x = game->player->x;
                    int y = game->player->y - 1;

                    if (input->action == ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            game->should_update = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            game->should_update = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        game->should_update = interact(game, input, x, y);

                        input->action = ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP9:
        {
            if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
            {
                if (input->targeting != TARGETING_NONE)
                {
                    input->target_x++;
                    input->target_y--;
                }
                else
                {
                    int x = game->player->x + 1;
                    int y = game->player->y - 1;

                    if (input->action == ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            game->should_update = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            game->should_update = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        game->should_update = interact(game, input, x, y);

                        input->action = ACTION_NONE;
                    }
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
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    game->should_update = actor_ascend(game->player);
                }
            }
            break;
            case '>':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    game->should_update = actor_descend(game->player);
                }
            }
            break;
            case 'b':
            {
                ui_panel_toggle(ui, PANEL_SPELLBOOK);
            }
            break;
            case 'C':
            {
                ui_panel_toggle(ui, PANEL_CHARACTER);
            }
            break;
            case 'c':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    input->action = ACTION_CLOSE_DOOR;

                    game_log(
                        game,
                        game->player->level,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction");
                }
            }
            break;
            case 'd':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    if (ui->panel_visible && ui->current_panel == PANEL_INVENTORY)
                    {
                        switch (ui->current_panel)
                        {
                        case PANEL_INVENTORY:
                        {
                            if (TCOD_list_size(game->player->items) > 0)
                            {
                                struct item *item = TCOD_list_get(game->player->items, ui->panel_status[ui->current_panel].current_index);

                                game->should_update = actor_drop(game->player, item);
                            }
                        }
                        break;
                        }
                    }
                    else
                    {
                        input->action = ACTION_DRINK;

                        game_log(
                            game,
                            game->player->level,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose a direction");
                    }
                }
            }
            break;
            case 'e':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    if (ui->panel_visible && ui->current_panel == PANEL_INVENTORY)
                    {
                        switch (ui->current_panel)
                        {
                        case PANEL_INVENTORY:
                        {
                            if (TCOD_list_size(game->player->items) > 0)
                            {
                                struct item *item = TCOD_list_get(game->player->items, ui->panel_status[ui->current_panel].current_index);

                                game->should_update = actor_equip(game->player, item);
                            }
                        }
                        break;
                        }
                    }
                }
            }
            break;
            case 'f':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    if (input->targeting == TARGETING_SHOOT)
                    {
                        actor_shoot(game->player, input->target_x, input->target_y, &cb_should_update, game);

                        input->targeting = TARGETING_NONE;
                    }
                    else
                    {
                        input->targeting = TARGETING_SHOOT;

                        bool target_found = false;

                        struct map *map = &game->maps[game->player->level];

                        {
                            struct actor *target = NULL;
                            float min_distance = 1000.0f;

                            for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
                            {
                                struct actor *actor = *iterator;

                                if (TCOD_map_is_in_fov(game->player->fov, actor->x, actor->y) &&
                                    actor->faction != game->player->faction &&
                                    !actor->dead)
                                {
                                    float dist = distance_sq(game->player->x, game->player->y, actor->x, actor->y);

                                    if (dist < min_distance)
                                    {
                                        target = actor;
                                        min_distance = dist;
                                    }
                                }
                            }

                            if (target)
                            {
                                target_found = true;

                                input->target_x = target->x;
                                input->target_y = target->y;
                            }
                        }

                        if (!target_found)
                        {
                            input->target_x = game->player->x;
                            input->target_y = game->player->y;
                        }
                    }
                }
            }
            break;
            case 'g':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    game->should_update = actor_grab(game->player, game->player->x, game->player->y);
                }
            }
            break;
            case 'i':
            {
                ui_panel_toggle(ui, PANEL_INVENTORY);
            }
            break;
            case 'l':
            {
                if (game->state == STATE_PLAYING)
                {
                    if (input->targeting == TARGETING_LOOK)
                    {
                        input->targeting = TARGETING_NONE;
                    }
                    else
                    {
                        input->targeting = TARGETING_LOOK;

                        input->target_x = game->player->x;
                        input->target_y = game->player->y;
                    }
                }
            }
            break;
            case 'm':
            {
                ui->message_log_visible = !ui->message_log_visible;
            }
            break;
            case 'o':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    input->action = ACTION_OPEN_DOOR;

                    game_log(
                        game,
                        game->player->level,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction");
                }
            }
            break;
            case 'p':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    input->action = ACTION_PRAY;

                    game_log(
                        game,
                        game->player->level,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction");
                }
            }
            break;
            case 'q':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    if (ui->panel_visible && ui->current_panel == PANEL_INVENTORY)
                    {
                        switch (ui->current_panel)
                        {
                        case PANEL_INVENTORY:
                        {
                            if (TCOD_list_size(game->player->items) > 0)
                            {
                                struct item *item = TCOD_list_get(game->player->items, ui->panel_status[ui->current_panel].current_index);

                                game->should_update = actor_quaff(game->player, item);
                            }
                        }
                        break;
                        }
                    }
                }
            }
            break;
            case 'r':
            {
                game->should_restart = true;
            }
            break;
            case 's':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    if (key.lctrl)
                    {
                        game_save(game);

                        game_log(
                            game,
                            game->player->level,
                            game->player->x,
                            game->player->y,
                            TCOD_green,
                            "Game saved!");
                    }
                    else
                    {
                        input->action = ACTION_SIT;

                        game_log(
                            game,
                            game->player->level,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose a direction");
                    }
                }
            }
            break;
            case 't':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    game->player->torch = !game->player->torch;

                    game->should_update = true;
                }
            }
            break;
            case 'x':
            {
                if (game->state == STATE_PLAYING)
                {
                    if (input->targeting == TARGETING_EXAMINE)
                    {
                        input->targeting = TARGETING_NONE;

                        // TODO: send examine target to ui

                        if (!ui->panel_visible || ui->current_panel != PANEL_EXAMINE)
                        {
                            ui_panel_toggle(ui, PANEL_EXAMINE);
                        }
                    }
                    else
                    {
                        input->targeting = TARGETING_EXAMINE;

                        input->target_x = game->player->x;
                        input->target_y = game->player->y;
                    }
                }
            }
            break;
            case 'z':
            {
                if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
                {
                    if (input->targeting == TARGETING_SPELL)
                    {
                        input->targeting = TARGETING_NONE;
                    }
                    else
                    {
                        input->targeting = TARGETING_SPELL;

                        input->target_x = game->player->x;
                        input->target_y = game->player->y;
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
    case TCOD_EVENT_MOUSE_PRESS:
    {
        input->automoving = false;

        if (mouse.lbutton)
        {
            if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
            {
                if (ui->tooltip_visible)
                {
                    if (ui_tooltip_is_inside(ui, ui->mouse_x, ui->mouse_y))
                    {
                        struct tooltip_option *option = ui_tooltip_get_selected(ui);

                        if (option)
                        {
                            if (option->fn)
                            {
                                game->should_update = option->fn(game, input, option->data);
                            }

                            ui_tooltip_hide(ui);
                        }
                    }
                    else
                    {
                        ui_tooltip_hide(ui);
                    }
                }
                else if (ui_view_is_inside(ui, ui->mouse_x, ui->mouse_y))
                {
                    input->automoving = true;
                    input->automove_x = ui->mouse_tile_x;
                    input->automove_y = ui->mouse_tile_y;
                }
            }
        }
        else if (mouse.rbutton)
        {
            if (game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
            {
                // struct map *map = &game->maps[game->player->level];
                // struct tile *tile = &map->tiles[ui->mouse_tile_x][ui->mouse_tile_y];

                ui_tooltip_show(ui);

                struct tooltip_data data;
                data.x = ui->mouse_tile_x;
                data.y = ui->mouse_tile_y;

                ui_tooltip_options_add(ui, "Move", &tooltip_option_move, data);
                ui_tooltip_options_add(ui, "Cancel", NULL, data);
            }
        }
    }
    break;
    case TCOD_EVENT_MOUSE_MOVE:
    {
        ui->mouse_x = mouse.cx;
        ui->mouse_y = mouse.cy;
        ui->mouse_tile_x = mouse.cx + ui->view_x;
        ui->mouse_tile_y = mouse.cy + ui->view_y;
    }
    break;
    }

    if (input->automoving && game->state == STATE_PLAYING && game->play_state == PLAY_STATE_PLAYING && game->turn_available)
    {
        game->should_update = actor_path_towards(game->player, input->automove_x, input->automove_y);

        if (!game->should_update)
        {
            input->automoving = false;
        }
    }
}

void input_destroy(struct input *input)
{
    free(input);
}

static bool interact(struct game *game, struct input *input, int x, int y)
{
    if (!map_is_inside(x, y))
    {
        return false;
    }

    switch (input->action)
    {
    case ACTION_DESCEND:
        return actor_descend(game->player);
    case ACTION_ASCEND:
        return actor_ascend(game->player);
    case ACTION_CLOSE_DOOR:
        return actor_close_door(game->player, x, y);
    case ACTION_OPEN_DOOR:
        return actor_open_door(game->player, x, y);
    case ACTION_PRAY:
        return actor_pray(game->player, x, y);
    case ACTION_DRINK:
        return actor_drink(game->player, x, y);
    case ACTION_SIT:
        return actor_sit(game->player, x, y);
    }

    return false;
}

static void cb_should_update(struct game *game)
{
    game->should_update = true;
}

static bool tooltip_option_move(struct game *game, struct input *input, struct tooltip_data data)
{
    input->automoving = true;
    input->automove_x = data.x;
    input->automove_y = data.y;

    return false;
}
