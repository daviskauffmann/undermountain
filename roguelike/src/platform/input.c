#include <platform/platform.h>

struct input *input;

static bool do_directional_action(struct actor *player, enum directional_action directional_action, int x, int y);
static void set_took_turn(void *on_hit_params);
static bool tooltip_option_move(struct tooltip_data data);

void input_init(void)
{
    input = malloc(sizeof(struct input));

    input->directional_action = DIRECTIONAL_ACTION_NONE;
    input->inventory_action = INVENTORY_ACTION_NONE;
    input->character_action = CHARACTER_ACTION_NONE;

    input->automoving = false;
    input->automove_x = -1;
    input->automove_y = -1;

    input->took_turn = true;
    input->request_close = false;
}

void input_handle(void)
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
        case TCODK_ENTER:
        {
            if (key.lalt)
            {
                fullscreen = !fullscreen;

                TCOD_console_set_fullscreen(fullscreen);
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
    }

    switch (ui->state)
    {
    case UI_STATE_MENU:
    {
        switch (ev)
        {
        case TCOD_EVENT_KEY_PRESS:
        {
            switch (key.vk)
            {
            case TCODK_ESCAPE:
            {
                switch (ui->menu_state)
                {
                case MENU_STATE_MAIN:
                {
                    input->request_close = true;
                }
                break;
                case MENU_STATE_ABOUT:
                {
                    ui->menu_state = MENU_STATE_MAIN;
                }
                break;
                }
            }
            break;
            case TCODK_CHAR:
            {
                bool handled = false;
                int alpha = key.c - 'a';

                switch (ui->menu_state)
                {
                case MENU_STATE_MAIN:
                {
                    if (alpha >= 0 && alpha < NUM_MAIN_MENU_OPTIONS)
                    {
                        enum main_menu_option main_menu_option = (enum main_menu_option)alpha;

                        switch (main_menu_option)
                        {
                        case MAIN_MENU_OPTION_START:
                        {
                            game_init();

                            if (TCOD_sys_file_exists(SAVE_PATH))
                            {
                                game_load();
                            }
                            else
                            {
                                game_new();
                            }

                            ui->state = UI_STATE_GAME;
                        }
                        break;
                        case MAIN_MENU_OPTION_ABOUT:
                        {
                            ui->menu_state = MENU_STATE_ABOUT;
                        }
                        break;
                        case MAIN_MENU_OPTION_QUIT:
                        {
                            input->request_close = true;
                        }
                        break;
                        }

                        handled = true;
                    }
                }
                break;
                }

                if (handled)
                {
                    break;
                }
            }
            break;
            }
        }
        break;
        case TCOD_EVENT_MOUSE_PRESS:
        {
            if (mouse.lbutton)
            {
                enum main_menu_option main_menu_option = ui_main_menu_get_selected();

                switch (main_menu_option)
                {
                case MAIN_MENU_OPTION_START:
                {
                    game_init();

                    if (TCOD_sys_file_exists(SAVE_PATH))
                    {
                        game_load();
                    }
                    else
                    {
                        game_new();
                    }

                    ui->state = UI_STATE_GAME;
                }
                break;
                case MAIN_MENU_OPTION_ABOUT:
                {
                    ui->menu_state = MENU_STATE_ABOUT;
                }
                break;
                case MAIN_MENU_OPTION_QUIT:
                {
                    input->request_close = true;
                }
                break;
                }
            }
        }
        break;
        }
    }
    break;
    case UI_STATE_GAME:
    {
        switch (ev)
        {
        case TCOD_EVENT_KEY_PRESS:
        {
            input->automoving = false;

            switch (key.vk)
            {
            case TCODK_ESCAPE:
            {
                if (ui->tooltip_visible)
                {
                    ui_tooltip_hide();
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
                    ui->state = UI_STATE_MENU;

                    game_save();
                    game_quit();
                }
            }
            break;
            case TCODK_PAGEDOWN:
            {
                if (ui->panel_visible)
                {
                    struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

                    panel_status->scroll++;
                }
            }
            break;
            case TCODK_PAGEUP:
            {
                if (ui->panel_visible)
                {
                    struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

                    panel_status->scroll--;
                }
            }
            break;
            case TCODK_KP1:
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x--;
                    ui->target_y++;
                }
                else if (game->state == GAME_STATE_PLAY)
                {
                    int x = game->player->x - 1;
                    int y = game->player->y + 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            input->took_turn = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            input->took_turn = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        input->took_turn = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
            break;
            case TCODK_KP2:
            case TCODK_DOWN:
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x;
                    ui->target_y++;
                }
                else if (game->state == GAME_STATE_PLAY)
                {
                    int x = game->player->x;
                    int y = game->player->y + 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            input->took_turn = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            input->took_turn = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        input->took_turn = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
            break;
            case TCODK_KP3:
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x++;
                    ui->target_y++;
                }
                else if (game->state == GAME_STATE_PLAY)
                {
                    int x = game->player->x + 1;
                    int y = game->player->y + 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            input->took_turn = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            input->took_turn = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        input->took_turn = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
            break;
            case TCODK_KP4:
            case TCODK_LEFT:
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x--;
                    ui->target_y;
                }
                else if (game->state == GAME_STATE_PLAY)
                {
                    int x = game->player->x - 1;
                    int y = game->player->y;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            input->took_turn = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            input->took_turn = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        input->took_turn = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
            break;
            case TCODK_KP5:
            {
                if (game->state != GAME_STATE_WAIT)
                {
                    input->took_turn = true;
                }
            }
            break;
            case TCODK_KP6:
            case TCODK_RIGHT:
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x++;
                    ui->target_y;
                }
                else if (game->state == GAME_STATE_PLAY)
                {
                    int x = game->player->x + 1;
                    int y = game->player->y;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            input->took_turn = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            input->took_turn = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        input->took_turn = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
            break;
            case TCODK_KP7:
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x--;
                    ui->target_y--;
                }
                else if (game->state == GAME_STATE_PLAY)
                {
                    int x = game->player->x - 1;
                    int y = game->player->y - 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            input->took_turn = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            input->took_turn = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        input->took_turn = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
            break;
            case TCODK_KP8:
            case TCODK_UP:
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x;
                    ui->target_y--;
                }
                else if (game->state == GAME_STATE_PLAY)
                {
                    int x = game->player->x;
                    int y = game->player->y - 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            input->took_turn = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            input->took_turn = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        input->took_turn = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
            break;
            case TCODK_KP9:
            {
                if (ui->targeting != TARGETING_NONE)
                {
                    ui->target_x++;
                    ui->target_y--;
                }
                else if (game->state == GAME_STATE_PLAY)
                {
                    int x = game->player->x + 1;
                    int y = game->player->y - 1;

                    if (input->directional_action == DIRECTIONAL_ACTION_NONE)
                    {
                        if (key.lctrl)
                        {
                            input->took_turn = actor_swing(game->player, x, y);
                        }
                        else
                        {
                            input->took_turn = actor_move(game->player, x, y);
                        }
                    }
                    else
                    {
                        input->took_turn = do_directional_action(game->player, input->directional_action, x, y);

                        input->directional_action = DIRECTIONAL_ACTION_NONE;
                    }
                }
            }
            break;
            case TCODK_CHAR:
            {
                bool handled = false;
                int alpha = key.c - 'a';

                if (input->inventory_action != INVENTORY_ACTION_NONE && alpha >= 0 && alpha < TCOD_list_size(game->player->items))
                {
                    struct item *item = TCOD_list_get(game->player->items, alpha);

                    switch (input->inventory_action)
                    {
                    case INVENTORY_ACTION_EQUIP:
                    {
                        if (game->state == GAME_STATE_PLAY)
                        {
                            input->took_turn = actor_equip(game->player, item);
                        }
                    }
                    break;
                    case INVENTORY_ACTION_EXAMINE:
                    {
                        // TODO: send examine target to ui

                        ui_panel_show(PANEL_EXAMINE);
                    }
                    break;
                    case INVENTORY_ACTION_DROP:
                    {
                        if (game->state == GAME_STATE_PLAY)
                        {
                            input->took_turn = actor_drop(game->player, item);
                        }
                    }
                    break;
                    case INVENTORY_ACTION_QUAFF:
                    {
                        if (game->state == GAME_STATE_PLAY)
                        {
                            input->took_turn = actor_quaff(game->player, item);
                        }
                    }
                    break;
                    }

                    input->inventory_action = INVENTORY_ACTION_NONE;

                    ui->panel_status[PANEL_INVENTORY].selection_mode = false;

                    handled = true;
                }
                else if (input->character_action != CHARACTER_ACTION_NONE && alpha >= 0 && alpha < NUM_EQUIP_SLOTS - 1)
                {
                    enum equip_slot equip_slot = (enum equip_slot)(alpha + 1);

                    switch (input->character_action)
                    {
                    case CHARACTER_ACTION_EXAMINE:
                    {
                        // TODO: send examine target to ui

                        ui_panel_show(PANEL_EXAMINE);
                    }
                    break;
                    case CHARACTER_ACTION_UNEQUIP:
                    {
                        if (game->state == GAME_STATE_PLAY)
                        {
                            input->took_turn = actor_unequip(game->player, equip_slot);
                        }
                    }
                    break;
                    }

                    input->character_action = CHARACTER_ACTION_NONE;

                    ui->panel_status[PANEL_CHARACTER].selection_mode = false;

                    handled = true;
                }

                for (enum panel panel = 0; panel < NUM_PANELS; panel++)
                {
                    if (ui->panel_status[panel].selection_mode)
                    {
                        handled = true;
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
                    if (game->state == GAME_STATE_PLAY)
                    {
                        input->took_turn = actor_ascend(game->player);
                    }
                }
                break;
                case '>':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        input->took_turn = actor_descend(game->player);
                    }
                }
                break;
                case 'b':
                {
                    ui_panel_toggle(PANEL_SPELLBOOK);
                }
                break;
                case 'C':
                {
                    ui_panel_toggle(PANEL_CHARACTER);
                }
                break;
                case 'c':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        input->directional_action = DIRECTIONAL_ACTION_CLOSE_DOOR;

                        game_log(
                            game->player->floor,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose a direction, ESC to cancel");
                    }
                }
                break;
                case 'D':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        input->directional_action = DIRECTIONAL_ACTION_DRINK;

                        game_log(
                            game->player->floor,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose a direction, ESC to cancel");
                    }
                }
                break;
                case 'd':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        ui_panel_show(PANEL_INVENTORY);

                        input->inventory_action = INVENTORY_ACTION_DROP;
                        ui->panel_status[PANEL_INVENTORY].selection_mode = true;

                        game_log(
                            game->player->floor,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose an item to drop, ESC to cancel");
                    }
                }
                break;
                case 'e':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        ui_panel_show(PANEL_INVENTORY);

                        input->inventory_action = INVENTORY_ACTION_EQUIP;
                        ui->panel_status[PANEL_INVENTORY].selection_mode = true;

                        game_log(
                            game->player->floor,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose an item to equip, ESC to cancel");
                    }
                }
                break;
                case 'f':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        if (ui->targeting == TARGETING_SHOOT)
                        {
                            actor_shoot(game->player, ui->target_x, ui->target_y, &set_took_turn, NULL);

                            ui->targeting = TARGETING_NONE;
                        }
                        else
                        {
                            ui->targeting = TARGETING_SHOOT;

                            bool target_found = false;

                            struct map *map = &game->maps[game->player->floor];

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
                    if (game->state == GAME_STATE_PLAY)
                    {
                        input->took_turn = actor_grab(game->player, game->player->x, game->player->y);
                    }
                }
                break;
                case 'i':
                {
                    ui_panel_toggle(PANEL_INVENTORY);
                }
                break;
                case 'l':
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
                break;
                case 'm':
                {
                    ui->message_log_visible = !ui->message_log_visible;
                }
                break;
                case 'o':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        input->directional_action = DIRECTIONAL_ACTION_OPEN_DOOR;

                        game_log(
                            game->player->floor,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose a direction, ESC to cancel");
                    }
                }
                break;
                case 'p':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        input->directional_action = DIRECTIONAL_ACTION_PRAY;

                        game_log(
                            game->player->floor,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose a direction, ESC to cancel");
                    }
                }
                break;
                case 'q':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        ui_panel_show(PANEL_INVENTORY);

                        input->inventory_action = INVENTORY_ACTION_QUAFF;
                        ui->panel_status[PANEL_INVENTORY].selection_mode = true;

                        game_log(
                            game->player->floor,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose an item to quaff, ESC to cancel");
                    }
                }
                break;
                case 's':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        input->directional_action = DIRECTIONAL_ACTION_SIT;

                        game_log(
                            game->player->floor,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose a direction, ESC to cancel");
                    }
                }
                break;
                case 't':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        game->player->torch = !game->player->torch;

                        input->took_turn = true;
                    }
                }
                break;
                case 'u':
                {
                    if (game->state == GAME_STATE_PLAY)
                    {
                        ui_panel_show(PANEL_CHARACTER);

                        input->character_action = CHARACTER_ACTION_UNEQUIP;
                        ui->panel_status[PANEL_CHARACTER].selection_mode = true;

                        game_log(
                            game->player->floor,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose an item to unequip, ESC to cancel");
                    }
                }
                break;
                case 'X':
                {
                    ui_panel_show(PANEL_INVENTORY);

                    input->inventory_action = INVENTORY_ACTION_EXAMINE;
                    ui->panel_status[PANEL_INVENTORY].selection_mode = true;

                    game_log(
                        game->player->floor,
                        game->player->x,
                        game->player->y,
                        TCOD_white,
                        "Choose an item to examine, ESC to cancel");
                }
                break;
                case 'x':
                {
                    if (key.lctrl)
                    {
                        ui_panel_show(PANEL_CHARACTER);

                        input->character_action = CHARACTER_ACTION_EXAMINE;
                        ui->panel_status[PANEL_CHARACTER].selection_mode = true;

                        game_log(
                            game->player->floor,
                            game->player->x,
                            game->player->y,
                            TCOD_white,
                            "Choose an equipment to examine, ESC to cancel");
                    }
                    else
                    {
                        if (ui->targeting == TARGETING_EXAMINE)
                        {
                            ui->targeting = TARGETING_NONE;

                            // TODO: send examine target to ui

                            ui_panel_show(PANEL_EXAMINE);
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
                    if (game->state == GAME_STATE_PLAY)
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
        case TCOD_EVENT_MOUSE_PRESS:
        {
            if (mouse.lbutton)
            {
                input->automoving = false;

                if (ui->tooltip_visible)
                {
                    if (ui_tooltip_is_inside(ui->mouse_x, ui->mouse_y))
                    {
                        struct tooltip_option *tooltip_option = ui_tooltip_get_selected();

                        if (tooltip_option)
                        {
                            if (tooltip_option->fn)
                            {
                                input->took_turn = tooltip_option->fn(tooltip_option->tooltip_data);
                            }

                            ui_tooltip_hide();
                        }
                    }
                    else
                    {
                        ui_tooltip_hide();
                    }
                }
                else if (ui_view_is_inside(ui->mouse_x, ui->mouse_y))
                {
                    input->automoving = true;
                    input->automove_x = ui->mouse_tile_x;
                    input->automove_y = ui->mouse_tile_y;
                }
                else if (ui_panel_is_inside(ui->mouse_x, ui->mouse_y))
                {
                    if (input->inventory_action != INVENTORY_ACTION_NONE)
                    {
                        struct item *item = ui_panel_inventory_get_selected();

                        if (item)
                        {
                            switch (input->inventory_action)
                            {
                            case INVENTORY_ACTION_EQUIP:
                            {
                                input->took_turn = actor_equip(game->player, item);

                                input->inventory_action = INVENTORY_ACTION_NONE;
                            }
                            break;
                            case INVENTORY_ACTION_DROP:
                            {
                                input->took_turn = actor_drop(game->player, item);

                                input->inventory_action = INVENTORY_ACTION_NONE;
                            }
                            break;
                            }
                        }
                    }
                    else if (input->character_action != CHARACTER_ACTION_NONE)
                    {
                        enum equip_slot equip_slot = ui_panel_character_get_selected();

                        if (equip_slot >= 1 && equip_slot < NUM_EQUIP_SLOTS)
                        {
                            input->took_turn = actor_unequip(game->player, equip_slot);

                            input->character_action = CHARACTER_ACTION_NONE;

                            ui->panel_status[PANEL_CHARACTER].selection_mode = false;
                        }
                    }
                }
            }
            else if (mouse.rbutton)
            {
                if (ui_view_is_inside(ui->mouse_x, ui->mouse_y) && map_is_inside(ui->mouse_tile_x, ui->mouse_tile_y))
                {
                    struct map *map = &game->maps[game->player->floor];
                    struct tile *tile = &map->tiles[ui->mouse_tile_x][ui->mouse_tile_y];

                    ui_tooltip_show();

                    struct tooltip_data tooltip_data;
                    tooltip_data.x = ui->mouse_tile_x;
                    tooltip_data.y = ui->mouse_tile_y;

                    ui_tooltip_options_add("Move", tooltip_data, &tooltip_option_move);

                    if (tile->object)
                    {
                        ui_tooltip_options_add("Examine Object", tooltip_data, NULL);
                        ui_tooltip_options_add("Interact", tooltip_data, NULL);
                        ui_tooltip_options_add("Bash", tooltip_data, NULL);
                    }

                    if (tile->actor)
                    {
                        ui_tooltip_options_add("Examine Actor", tooltip_data, NULL);
                        ui_tooltip_options_add("Talk", tooltip_data, NULL);
                        ui_tooltip_options_add("Swap", tooltip_data, NULL);
                        ui_tooltip_options_add("Attack", tooltip_data, NULL);
                    }

                    if (TCOD_list_peek(tile->items))
                    {
                        ui_tooltip_options_add("Examine Item", tooltip_data, NULL);
                        ui_tooltip_options_add("Take Item", tooltip_data, NULL);
                    }

                    if (TCOD_list_size(tile->items) > 1)
                    {
                        ui_tooltip_options_add("Take All", tooltip_data, NULL);
                    }

                    ui_tooltip_options_add("Cancel", tooltip_data, NULL);
                }
                else if (ui_panel_is_inside(ui->mouse_x, ui->mouse_y))
                {
                    switch (ui->current_panel)
                    {
                    case PANEL_INVENTORY:
                    {
                        struct item *item = ui_panel_inventory_get_selected();

                        if (item)
                        {
                            ui_tooltip_show();

                            struct tooltip_data tooltip_data;
                            tooltip_data.item = item;

                            ui_tooltip_options_add("Drop", tooltip_data, NULL);

                            if (base_item_info[item_info[item->type].base_item].equip_slot != EQUIP_SLOT_NONE)
                            {
                                ui_tooltip_options_add("Equip", tooltip_data, NULL);
                            }

                            if (item_info[item->type].base_item == BASE_ITEM_POTION)
                            {
                                ui_tooltip_options_add("Quaff", tooltip_data, NULL);
                            }

                            ui_tooltip_options_add("Cancel", tooltip_data, NULL);
                        }

                        break;
                    }
                    break;
                    case PANEL_CHARACTER:
                    {
                        enum equip_slot equip_slot = ui_panel_character_get_selected();

                        if (equip_slot >= 1 && equip_slot < NUM_EQUIP_SLOTS)
                        {
                            struct item *equipment = game->player->equipment[equip_slot];

                            if (equipment)
                            {
                                ui_tooltip_show();

                                struct tooltip_data tooltip_data;
                                tooltip_data.equip_slot = equip_slot;

                                ui_tooltip_options_add("Unequip", tooltip_data, NULL);

                                ui_tooltip_options_add("Cancel", tooltip_data, NULL);
                            }
                        }
                    }
                    break;
                    }
                }
            }
            else if (mouse.wheel_down)
            {
                if (ui->panel_visible)
                {
                    struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

                    panel_status->scroll++;
                }
            }
            else if (mouse.wheel_up)
            {
                if (ui->panel_visible)
                {
                    struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

                    panel_status->scroll--;
                }
            }
        }
        break;
        }

        if (input->automoving && game->state == GAME_STATE_PLAY)
        {
            // probably shouldnt use the path function for this
            // we need to implement custom behavior depending on what the player is doing
            // for example, if the player selects the interact option on a tooltip for an object far away,
            //      the player should navigate there but not interact/attack anything along the way
            input->took_turn = actor_path_towards(game->player, input->automove_x, input->automove_y);

            if (!input->took_turn)
            {
                input->automoving = false;
            }
        }
    }
    break;
    }
}

void input_quit(void)
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

static void set_took_turn(void *on_hit_params)
{
    input->took_turn = true;
}

static bool tooltip_option_move(struct tooltip_data data)
{
    input->automoving = true;
    input->automove_x = data.x;
    input->automove_y = data.y;

    return false;
}
