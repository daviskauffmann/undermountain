#include <platform/platform.h>

struct ui *ui;

void ui_init(void)
{
    ui = malloc(sizeof(struct ui));

    ui->state = UI_STATE_MENU;
    ui->menu_state = MENU_STATE_MAIN;

    // TODO: platform assets?
    ui->main_menu_option_info[MAIN_MENU_OPTION_START].text = "Start";
    ui->main_menu_option_info[MAIN_MENU_OPTION_ABOUT].text = "About";
    ui->main_menu_option_info[MAIN_MENU_OPTION_QUIT].text = "Quit";

    ui->targeting = TARGETING_NONE;
    ui->target_x = -1;
    ui->target_y = -1;

    ui->current_panel = PANEL_CHARACTER;

    ui->panel_status[PANEL_CHARACTER].scroll = 0;
    ui->panel_status[PANEL_CHARACTER].selection_mode = false;
    ui->panel_status[PANEL_EXAMINE].scroll = 0;
    ui->panel_status[PANEL_EXAMINE].selection_mode = false;
    ui->panel_status[PANEL_INVENTORY].scroll = 0;
    ui->panel_status[PANEL_INVENTORY].selection_mode = false;
    ui->panel_status[PANEL_SPELLBOOK].scroll = 0;
    ui->panel_status[PANEL_SPELLBOOK].selection_mode = false;

    ui->message_log_visible = true;
    ui->message_log_x = 0;
    ui->message_log_height = 0;
    ui->message_log_y = 0;
    ui->message_log_width = 0;

    ui->panel_visible = false;
    ui->panel_width = 0;
    ui->panel_x = 0;
    ui->panel_y = 0;
    ui->panel_height = 0;

    ui->tooltip_visible = false;
    ui->tooltip_x = 0;
    ui->tooltip_y = 0;
    ui->tooltip_width = 0;
    ui->tooltip_height = 0;
    ui->tooltip_options = TCOD_list_new();

    ui->view_width = 0;
    ui->view_height = 0;
    ui->view_x = 0;
    ui->view_y = 0;

    ui->mouse_x = 0;
    ui->mouse_y = 0;
    ui->mouse_tile_x = 0;
    ui->mouse_tile_y = 0;
}

void ui_update(void)
{
    switch (ui->state)
    {
    case UI_STATE_MENU:
    {

    }
    break;
    case UI_STATE_GAME:
    {
        ui->message_log_x = 0;
        ui->message_log_height = console_height / 4;
        ui->message_log_y = console_height - ui->message_log_height;
        ui->message_log_width = console_width;

        ui->panel_width = console_width / 2;
        ui->panel_x = console_width - ui->panel_width;
        ui->panel_y = 0;
        ui->panel_height = console_height - (ui->message_log_visible ? ui->message_log_height : 0);

        ui->tooltip_width = 0;
        for (void **i = TCOD_list_begin(ui->tooltip_options); i != TCOD_list_end(ui->tooltip_options); i++)
        {
            struct tooltip_option *option = *i;

            int len = (int)strlen(option->text) + 2;

            if (len > ui->tooltip_width)
            {
                ui->tooltip_width = len;
            }
        }
        ui->tooltip_height = TCOD_list_size(ui->tooltip_options) + 2;

        ui->view_width = console_width - (ui->panel_visible ? ui->panel_width : 0);
        ui->view_height = console_height - (ui->message_log_visible ? ui->message_log_height : 0);
        ui->view_x = game->player->x - ui->view_width / 2;
        ui->view_y = game->player->y - ui->view_height / 2;

        if (ui->view_x + ui->view_width > MAP_WIDTH)
            ui->view_x = MAP_WIDTH - ui->view_width;
        if (ui->view_x < 0)
            ui->view_x = 0;
        if (ui->view_y + ui->view_height > MAP_HEIGHT)
            ui->view_y = MAP_HEIGHT - ui->view_height;
        if (ui->view_y < 0)
            ui->view_y = 0;
    }
    break;
    }
}

enum main_menu_option ui_main_menu_get_selected(void)
{
    if (ui->state == UI_STATE_MENU && ui->menu_state == MENU_STATE_MAIN)
    {
        int y = 1;
        for (enum main_menu_option main_menu_option = 0; main_menu_option < NUM_MAIN_MENU_OPTIONS; main_menu_option++)
        {
            struct main_menu_option_info *main_menu_option_info = &ui->main_menu_option_info[main_menu_option];

            if (ui->mouse_x > 0 && ui->mouse_x < (int)strlen(main_menu_option_info->text) + 3 + 1 && ui->mouse_y == y)
            {
                return main_menu_option;
            }

            y++;
        }
    }

    return -1;
}

bool ui_message_log_is_inside(int x, int y)
{
    return ui->state == UI_STATE_GAME && ui->message_log_visible && x >= ui->message_log_x && x < ui->message_log_x + ui->message_log_width && y >= ui->message_log_y && y < ui->message_log_y + ui->message_log_height;
}

bool ui_panel_is_inside(int x, int y)
{
    return ui->state == UI_STATE_GAME && ui->panel_visible && x >= ui->panel_x && x < ui->panel_x + ui->panel_width && y >= ui->panel_y && y < ui->panel_y + ui->panel_height;
}

void ui_panel_toggle(enum panel panel)
{
    if (ui->panel_visible)
    {
        if (ui->current_panel == panel)
        {
            ui->panel_visible = false;
        }
        else
        {
            ui->current_panel = panel;
        }
    }
    else
    {
        ui->current_panel = panel;
        ui->panel_visible = true;
    }
}

void ui_panel_show(enum panel panel)
{
    if (!ui->panel_visible || ui->current_panel != panel)
    {
        ui_panel_toggle(panel);
    }
}

enum equip_slot ui_panel_character_get_selected(void)
{
    if (ui->state == UI_STATE_GAME && ui->panel_visible)
    {
        if (ui->panel_visible && ui->current_panel == PANEL_CHARACTER)
        {
            int y = 15;
            for (enum equip_slot equip_slot = 1; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
            {
                if (ui->mouse_x > ui->panel_x && ui->mouse_x < ui->panel_x + (int)strlen(equip_slot_info[equip_slot].label) + 1 + 3 && ui->mouse_y == y + ui->panel_y - ui->panel_status[ui->current_panel].scroll)
                {
                    return equip_slot;
                }

                y++;
            }
        }
    }

    return -1;
}

struct item *ui_panel_inventory_get_selected(void)
{
    if (ui->state == UI_STATE_GAME && ui->panel_visible)
    {
        if (ui->panel_visible && ui->current_panel == PANEL_INVENTORY)
        {
            int y = 1;
            for (void **iterator = TCOD_list_begin(game->player->items); iterator != TCOD_list_end(game->player->items); iterator++)
            {
                struct item *item = *iterator;

                if (ui->mouse_x > ui->panel_x && ui->mouse_x < ui->panel_x + (int)strlen(item_info[item->type].name) + 1 + 3 && ui->mouse_y == y + ui->panel_y - ui->panel_status[ui->current_panel].scroll)
                {
                    return item;
                }

                y++;
            }
        }
    }

    return NULL;
}

bool ui_tooltip_is_inside(int x, int y)
{
    return ui->state == UI_STATE_GAME && ui->tooltip_visible && x >= ui->tooltip_x && x < ui->tooltip_x + ui->tooltip_width && y >= ui->tooltip_y && y < ui->tooltip_y + ui->tooltip_height;
}

void ui_tooltip_show(void)
{
    ui_tooltip_options_clear();

    ui->tooltip_visible = true;
    ui->tooltip_x = ui->mouse_x;
    ui->tooltip_y = ui->mouse_y;
}

void ui_tooltip_hide(void)
{
    ui_tooltip_options_clear();

    ui->tooltip_visible = false;
}

void ui_tooltip_options_add(char *text, struct tooltip_data tooltip_data, bool(*fn)(struct tooltip_data tooltip_data))
{
    struct tooltip_option *tooltip_option = tooltip_option_create(text, tooltip_data, fn);

    TCOD_list_push(ui->tooltip_options, tooltip_option);
}

void ui_tooltip_options_clear(void)
{
    for (void **iterator = TCOD_list_begin(ui->tooltip_options); iterator != TCOD_list_end(ui->tooltip_options); iterator++)
    {
        struct tooltip_option *tooltip_option = *iterator;

        iterator = TCOD_list_remove_iterator(ui->tooltip_options, iterator);

        tooltip_option_destroy(tooltip_option);
    }
}

struct tooltip_option *ui_tooltip_get_selected(void)
{
    if (ui->state == UI_STATE_GAME && ui->tooltip_visible)
    {
        int y = 1;
        for (void **i = TCOD_list_begin(ui->tooltip_options); i != TCOD_list_end(ui->tooltip_options); i++)
        {
            struct tooltip_option *option = *i;

            if (ui->mouse_x > ui->tooltip_x && ui->mouse_x < ui->tooltip_x + (int)strlen(option->text) + 1 && ui->mouse_y == y + ui->tooltip_y)
            {
                return option;
            }

            y++;
        }
    }

    return NULL;
}

bool ui_view_is_inside(int x, int y)
{
    return ui->state == UI_STATE_GAME && x >= 0 && x < ui->view_width && y >= 0 && y < ui->view_height;
}

void ui_quit(void)
{
    for (void **iterator = TCOD_list_begin(ui->tooltip_options); iterator != TCOD_list_end(ui->tooltip_options); iterator++)
    {
        struct tooltip_option *tooltip_option = *iterator;

        tooltip_option_destroy(tooltip_option);
    }

    TCOD_list_delete(ui->tooltip_options);

    free(ui);
}
