#include <libtcod/libtcod.h>
#include <malloc.h>

#include "actor.h"
#include "config.h"
#include "engine.h"
#include "game.h"
#include "input.h"
#include "ui.h"
#include "util.h"

#include "CMemleak.h"

static bool do_directional_action(struct actor *player, enum directional_action directional_action, int x, int y);
static void cb_should_update(struct game *game);
static bool tooltip_option_move(struct game *game, struct input *input, struct tooltip_data data);

struct input *input_create(void)
{
    struct input *input = calloc(1, sizeof(struct input));

    input->directional_action = DIRECTIONAL_ACTION_NONE;
    input->inventory_action = INVENTORY_ACTION_NONE;
    input->character_action = CHARACTER_ACTION_NONE;
    input->automoving = false;
    input->automove_x = -1;
    input->automove_y = -1;

    return input;
}

void input_handle(struct input *input, struct engine *engine, struct game *game, struct ui *ui)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        if (engine->state == ENGINE_STATE_PLAYING)
        {
            input->automoving = false;
        }

        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            if (engine->state == ENGINE_STATE_MENU)
            {
                if (ui->menu_state == MENU_STATE_MAIN)
                {
                    engine->should_quit = true;
                }
                else if (ui->menu_state == MENU_STATE_ABOUT)
                {
                    ui->menu_state = MENU_STATE_MAIN;
                }
            }
            else if (engine->state == ENGINE_STATE_PLAYING)
            {
                if (ui->tooltip_visible)
                {
                    ui_tooltip_hide(ui);
                }
                else if (input->directional_action != DIRECTIONAL_ACTION_NONE ||
                         input->inventory_action != INVENTORY_ACTION_NONE ||
                         input->character_action != CHARACTER_ACTION_NONE)
                {
                    input->directional_action = DIRECTIONAL_ACTION_NONE;
                    input->inventory_action = INVENTORY_ACTION_NONE;
                    input->character_action = CHARACTER_ACTION_NONE;

                    for (enum panel panel = 0; panel < NUM_PANELS; panel++)
                    {
                        ui->panel_status[panel].selection_mode = false;
                    }
                }
                else if (ui->panel_visible)
                {
                    ui->panel_visible = false;
                }
                else if (ui->targeting)
                {
                    ui->targeting = false;
                }
                else
                {
                    engine->state = ENGINE_STATE_MENU;

                    game->should_restart = true;
                }
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
        }
        break;
        case TCODK_PAGEDOWN:
        {
            if (engine->state == ENGINE_STATE_PLAYING && ui->panel_visible)
            {
                struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

                panel_status->scroll++;
            }
        }
        break;
        case TCODK_PAGEUP:
        {
            if (engine->state == ENGINE_STATE_PLAYING && ui->panel_visible)
            {
                struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

                panel_status->scroll--;
            }
        }
        break;
        case TCODK_KP1:
        {
            if (engine->state == ENGINE_STATE_PLAYING)
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x--;
                    ui->target_y++;
                }
                else if (game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    int x = game->player->x - 1;
                    int y = game->player->y + 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
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
                        game->should_update = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP2:
        case TCODK_DOWN:
        {
            if (engine->state == ENGINE_STATE_PLAYING)
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x;
                    ui->target_y++;
                }
                else if (game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    int x = game->player->x;
                    int y = game->player->y + 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
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
                        game->should_update = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP3:
        {
            if (engine->state == ENGINE_STATE_PLAYING)
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x++;
                    ui->target_y++;
                }
                else if (game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    int x = game->player->x + 1;
                    int y = game->player->y + 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
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
                        game->should_update = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP4:
        case TCODK_LEFT:
        {
            if (engine->state == ENGINE_STATE_PLAYING)
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x--;
                    ui->target_y;
                }
                else if (game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    int x = game->player->x - 1;
                    int y = game->player->y;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
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
                        game->should_update = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP5:
        {
            if (engine->state == ENGINE_STATE_PLAYING && game->turn_available)
            {
                game->should_update = true;
            }
        }
        break;
        case TCODK_KP6:
        case TCODK_RIGHT:
        {
            if (engine->state == ENGINE_STATE_PLAYING)
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x++;
                    ui->target_y;
                }
                else if (game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    int x = game->player->x + 1;
                    int y = game->player->y;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
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
                        game->should_update = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP7:
        {
            if (engine->state == ENGINE_STATE_PLAYING)
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x--;
                    ui->target_y--;
                }
                else if (game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    int x = game->player->x - 1;
                    int y = game->player->y - 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
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
                        game->should_update = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP8:
        case TCODK_UP:
        {
            if (engine->state == ENGINE_STATE_PLAYING)
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x;
                    ui->target_y--;
                }
                else if (game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    int x = game->player->x;
                    int y = game->player->y - 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
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
                        game->should_update = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_KP9:
        {
            if (engine->state == ENGINE_STATE_PLAYING)
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x++;
                    ui->target_y--;
                }
                else if (game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    int x = game->player->x + 1;
                    int y = game->player->y - 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
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
                        game->should_update = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
        }
        break;
        case TCODK_CHAR:
        {
            bool handled = false;

            if (key.c >= 97 && key.c <= 122)
            {
                if (engine->state == ENGINE_STATE_MENU)
                {
                    enum main_menu_option main_menu_option = key.c - 97;

                    switch (main_menu_option)
                    {
                    case MAIN_MENU_OPTION_START:
                    {
                        engine->state = ENGINE_STATE_PLAYING;

                        if (TCOD_sys_file_exists(SAVE_PATH))
                        {
                            game_load(game);
                        }
                        else
                        {
                            game_new(game);
                        }

                        handled = true;
                    }
                    break;
                    case MAIN_MENU_OPTION_ABOUT:
                    {
                        ui->menu_state = MENU_STATE_ABOUT;

                        handled = true;
                    }
                    break;
                    case MAIN_MENU_OPTION_QUIT:
                    {
                        engine->should_quit = true;

                        handled = true;
                    }
                    break;
                    }
                }
                else if (engine->state == ENGINE_STATE_PLAYING)
                {
                    if (game->state == GAME_STATE_PLAYING && game->turn_available)
                    {
                        if (input->inventory_action != INVENTORY_ACTION_NONE)
                        {
                            struct item *item = TCOD_list_get(game->player->items, key.c - 97);

                            if (item)
                            {
                                switch (input->inventory_action)
                                {
                                case INVENTORY_ACTION_EQUIP:
                                {
                                    game->should_update = actor_equip(game->player, item);

                                    input->inventory_action = INVENTORY_ACTION_NONE;

                                    ui->panel_status[PANEL_INVENTORY].selection_mode = false;

                                    handled = true;
                                }
                                break;
                                case INVENTORY_ACTION_DROP:
                                {
                                    game->should_update = actor_drop(game->player, item);

                                    input->inventory_action = INVENTORY_ACTION_NONE;

                                    ui->panel_status[PANEL_INVENTORY].selection_mode = false;

                                    handled = true;
                                }
                                break;
                                }
                            }
                        }
                        else if (input->character_action != CHARACTER_ACTION_NONE)
                        {
                            enum equip_slot equip_slot = key.c - 97 + 1;

                            if (equip_slot >= 1 && equip_slot < NUM_EQUIP_SLOTS)
                            {
                                game->should_update = actor_unequip(game->player, equip_slot);

                                input->character_action = CHARACTER_ACTION_NONE;

                                ui->panel_status[PANEL_CHARACTER].selection_mode = false;

                                handled = true;
                            }
                        }
                    }
                }
            }

            if (handled)
            {
                break;
            }

            switch (key.c)
            {
            case '<':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    game->should_update = actor_ascend(game->player);
                }
            }
            break;
            case '>':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    game->should_update = actor_descend(game->player);
                }
            }
            break;
            case 'b':
            {
                if (engine->state == ENGINE_STATE_PLAYING)
                {
                    ui_panel_toggle(ui, PANEL_SPELLBOOK);
                }
            }
            break;
            case 'C':
            {
                if (engine->state == ENGINE_STATE_PLAYING)
                {
                    ui_panel_toggle(ui, PANEL_CHARACTER);
                }
            }
            break;
            case 'c':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    input->directional_action = DIRECTIONAL_ACTION_CLOSE_DOOR;

                    game_log(
                        game,
                        game->player->level,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'D':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    input->directional_action = DIRECTIONAL_ACTION_DRINK;

                    game_log(
                        game,
                        game->player->level,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'd':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    ui_panel_show(ui, PANEL_INVENTORY);

                    input->inventory_action = INVENTORY_ACTION_DROP;
                    ui->panel_status[PANEL_INVENTORY].selection_mode = true;

                    game_log(
                        game,
                        game->player->level,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose an item to drop, ESC to cancel");
                }
            }
            break;
            case 'e':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    ui_panel_show(ui, PANEL_INVENTORY);

                    input->inventory_action = INVENTORY_ACTION_EQUIP;
                    ui->panel_status[PANEL_INVENTORY].selection_mode = true;

                    game_log(
                        game,
                        game->player->level,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose an item to equip, ESC to cancel");
                }
            }
            break;
            case 'f':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    if (ui->targeting == TARGETING_SHOOT)
                    {
                        actor_shoot(game->player, ui->target_x, ui->target_y, &cb_should_update, game);

                        ui->targeting = TARGETING_NONE;
                    }
                    else
                    {
                        ui->targeting = TARGETING_SHOOT;

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

                                ui->target_x = target->x;
                                ui->target_y = target->y;
                            }
                        }

                        if (!target_found)
                        {
                            ui->target_x = game->player->x;
                            ui->target_y = game->player->y;
                        }
                    }
                }
            }
            break;
            case 'g':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    game->should_update = actor_grab(game->player, game->player->x, game->player->y);
                }
            }
            break;
            case 'i':
            {
                if (engine->state == ENGINE_STATE_PLAYING)
                {
                    ui_panel_toggle(ui, PANEL_INVENTORY);
                }
            }
            break;
            case 'l':
            {
                if (engine->state == ENGINE_STATE_PLAYING)
                {
                    if (ui->targeting == TARGETING_LOOK)
                    {
                        ui->targeting = TARGETING_NONE;
                    }
                    else
                    {
                        ui->targeting = TARGETING_LOOK;

                        ui->target_x = game->player->x;
                        ui->target_y = game->player->y;
                    }
                }
            }
            break;
            case 'm':
            {
                if (engine->state == ENGINE_STATE_PLAYING)
                {
                    ui->message_log_visible = !ui->message_log_visible;
                }
            }
            break;
            case 'o':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    input->directional_action = DIRECTIONAL_ACTION_OPEN_DOOR;

                    game_log(
                        game,
                        game->player->level,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'p':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    input->directional_action = DIRECTIONAL_ACTION_PRAY;

                    game_log(
                        game,
                        game->player->level,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose a direction, ESC to cancel");
                }
            }
            break;
            case 'q':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    // TODO:
                    // open inventory
                    // enter "select item mode (quaff action)"
                    // ESC or Right Click to leave
                    // press a-z key or left click an item to select
                    // from wherever that event gets picked up, use the mode and calculate what item was selected
                    // drink it
                }
            }
            break;
            case 'r':
            {
                if (engine->state == ENGINE_STATE_PLAYING)
                {
                    ui->should_restart = true;
                }
            }
            break;
            case 's':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
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
                        input->directional_action = DIRECTIONAL_ACTION_SIT;

                        game_log(
                            game,
                            game->player->level,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose a direction, ESC to cancel");
                    }
                }
            }
            break;
            case 't':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    game->player->torch = !game->player->torch;

                    game->should_update = true;
                }
            }
            break;
            case 'u':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    ui_panel_show(ui, PANEL_CHARACTER);

                    input->character_action = CHARACTER_ACTION_UNEQUIP;
                    ui->panel_status[PANEL_CHARACTER].selection_mode = true;

                    game_log(
                        game,
                        game->player->level,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose an item to unequip, ESC to cancel");
                }
            }
            break;
            case 'x':
            {
                if (engine->state == ENGINE_STATE_PLAYING)
                {
                    if (ui->targeting == TARGETING_EXAMINE)
                    {
                        ui->targeting = TARGETING_NONE;

                        // TODO: send examine target to ui

                        ui_panel_show(ui, PANEL_EXAMINE);
                    }
                    else
                    {
                        ui->targeting = TARGETING_EXAMINE;

                        ui->target_x = game->player->x;
                        ui->target_y = game->player->y;
                    }
                }
            }
            break;
            case 'z':
            {
                if (engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
                {
                    if (ui->targeting == TARGETING_SPELL)
                    {
                        ui->targeting = TARGETING_NONE;
                    }
                    else
                    {
                        ui->targeting = TARGETING_SPELL;

                        ui->target_x = game->player->x;
                        ui->target_y = game->player->y;
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
    case TCOD_EVENT_MOUSE_MOVE:
    {
        ui->mouse_x = mouse.cx;
        ui->mouse_y = mouse.cy;
        ui->mouse_tile_x = mouse.cx + ui->view_x;
        ui->mouse_tile_y = mouse.cy + ui->view_y;
    }
    break;
    case TCOD_EVENT_MOUSE_PRESS:
    {
        if (mouse.lbutton)
        {
            if (engine->state == ENGINE_STATE_MENU)
            {
                enum main_menu_option main_menu_option = ui_main_menu_get_selected(ui);

                switch (main_menu_option)
                {
                case MAIN_MENU_OPTION_START:
                {
                    engine->state = ENGINE_STATE_PLAYING;

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
                case MAIN_MENU_OPTION_ABOUT:
                {
                    ui->menu_state = MENU_STATE_ABOUT;
                }
                break;
                case MAIN_MENU_OPTION_QUIT:
                {
                    engine->should_quit = true;
                }
                break;
                }
            }
            else if (engine->state == ENGINE_STATE_PLAYING)
            {
                input->automoving = false;

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
                else if (ui_panel_is_inside(ui, ui->mouse_x, ui->mouse_y))
                {
                    if (input->inventory_action != INVENTORY_ACTION_NONE)
                    {
                        struct item *item = ui_panel_inventory_get_selected(ui, game);

                        if (item)
                        {
                            switch (input->inventory_action)
                            {
                            case INVENTORY_ACTION_EQUIP:
                            {
                                game->should_update = actor_equip(game->player, item);

                                input->inventory_action = INVENTORY_ACTION_NONE;
                            }
                            break;
                            case INVENTORY_ACTION_DROP:
                            {
                                game->should_update = actor_drop(game->player, item);

                                input->inventory_action = INVENTORY_ACTION_NONE;
                            }
                            break;
                            }
                        }
                    }
                    else if (input->character_action != CHARACTER_ACTION_NONE)
                    {
                        enum equip_slot equip_slot = ui_panel_character_get_selected(ui, game);

                        if (equip_slot >= 1 && equip_slot < NUM_EQUIP_SLOTS)
                        {
                            game->should_update = actor_unequip(game->player, equip_slot);

                            input->character_action = CHARACTER_ACTION_NONE;

                            ui->panel_status[PANEL_CHARACTER].selection_mode = false;
                        }
                    }
                }
            }
        }
        else if (mouse.rbutton)
        {
            if (engine->state == ENGINE_STATE_PLAYING)
            {
                if (ui_view_is_inside(ui, ui->mouse_x, ui->mouse_y) && map_is_inside(ui->mouse_tile_x, ui->mouse_tile_y))
                {
                    struct map *map = &game->maps[game->player->level];
                    struct tile *tile = &map->tiles[ui->mouse_tile_x][ui->mouse_tile_y];

                    ui_tooltip_show(ui);

                    struct tooltip_data data;
                    data.x = ui->mouse_tile_x;
                    data.y = ui->mouse_tile_y;

                    ui_tooltip_options_add(ui, "Move", &tooltip_option_move, data);

                    if (tile->object)
                    {
                        ui_tooltip_options_add(ui, "Examine Object", NULL, data);
                        ui_tooltip_options_add(ui, "Interact", NULL, data);
                        ui_tooltip_options_add(ui, "Bash", NULL, data);
                    }

                    if (tile->actor)
                    {
                        ui_tooltip_options_add(ui, "Examine Actor", NULL, data);
                        ui_tooltip_options_add(ui, "Talk", NULL, data);
                        ui_tooltip_options_add(ui, "Swap", NULL, data);
                        ui_tooltip_options_add(ui, "Attack", NULL, data);
                    }

                    if (TCOD_list_peek(tile->items))
                    {
                        ui_tooltip_options_add(ui, "Examine Item", NULL, data);
                        ui_tooltip_options_add(ui, "Take Item", NULL, data);
                    }

                    if (TCOD_list_size(tile->items) > 1)
                    {
                        ui_tooltip_options_add(ui, "Take All", NULL, data);
                    }

                    ui_tooltip_options_add(ui, "Cancel", NULL, data);
                }
                else if (ui_panel_is_inside(ui, ui->mouse_x, ui->mouse_y))
                {
                    switch (ui->current_panel)
                    {
                    case PANEL_INVENTORY:
                    {
                        struct item *item = ui_panel_inventory_get_selected(ui, game);

                        if (item)
                        {
                            struct item_info *item_info = &game->item_info[item->type];
                            struct base_item_info *base_item_info = &game->base_item_info[item_info->base_item];

                            ui_tooltip_show(ui);

                            struct tooltip_data data;
                            data.item = item;

                            ui_tooltip_options_add(ui, "Drop", NULL, data);

                            if (base_item_info->equip_slot != EQUIP_SLOT_NONE)
                            {
                                ui_tooltip_options_add(ui, "Equip", NULL, data);
                            }

                            if (item_info->base_item == BASE_ITEM_POTION)
                            {
                                ui_tooltip_options_add(ui, "Quaff", NULL, data);
                            }

                            ui_tooltip_options_add(ui, "Cancel", NULL, data);
                        }

                        break;
                    }
                    break;
                    case PANEL_CHARACTER:
                    {
                        enum equip_slot equip_slot = ui_panel_character_get_selected(ui, game);

                        if (equip_slot >= 1 && equip_slot < NUM_EQUIP_SLOTS)
                        {
                            struct item *equipment = game->player->equipment[equip_slot];

                            if (equipment)
                            {
                                ui_tooltip_show(ui);

                                struct tooltip_data data;
                                // data.equip_slot = equip_slot;

                                ui_tooltip_options_add(ui, "Unequip", NULL, data);

                                ui_tooltip_options_add(ui, "Cancel", NULL, data);
                            }
                        }
                    }
                    break;
                    }
                }
            }
        }
        else if (mouse.wheel_down)
        {
            if (engine->state == ENGINE_STATE_PLAYING && ui->panel_visible)
            {
                struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

                panel_status->scroll++;
            }
        }
        else if (mouse.wheel_up)
        {
            if (engine->state == ENGINE_STATE_PLAYING && ui->panel_visible)
            {
                struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

                panel_status->scroll--;
            }
        }
    }
    break;
    }

    if (input->automoving && engine->state == ENGINE_STATE_PLAYING && game->state == GAME_STATE_PLAYING && game->turn_available)
    {
        // probably shouldnt use the path function for this
        // we need to implement custom behavior depending on what the player is doing
        // for example, if the player selects the interact option on a tooltip for an object far away,
        //      the player should navigate there but not interact/attack anything along the way
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

static bool do_directional_action(struct actor *player, enum directional_action directional_action, int x, int y)
{
    switch (directional_action)
    {
    case DIRECTIONAL_ACTION_CLOSE_DOOR:
        return actor_close_door(player, x, y);
    case DIRECTIONAL_ACTION_DRINK:
        return actor_drink(player, x, y);
    case DIRECTIONAL_ACTION_OPEN_DOOR:
        return actor_open_door(player, x, y);
    case DIRECTIONAL_ACTION_PRAY:
        return actor_pray(player, x, y);
    case DIRECTIONAL_ACTION_SIT:
        return actor_sit(player, x, y);
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
