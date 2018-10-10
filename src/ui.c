#include <libtcod/libtcod.h>
#include <malloc.h>
#include <string.h>

#include "game.h"
#include "map.h"
#include "ui.h"
#include "window.h"

#include "CMemleak.h"

struct ui *ui_create(void)
{
    struct ui *ui = calloc(1, sizeof(struct ui));

    ui->menu_state = MENU_STATE_MAIN;
    ui->menu_option_info[MENU_OPTION_START].text = "Start";
    ui->menu_option_info[MENU_OPTION_ABOUT].text = "About";
    ui->menu_option_info[MENU_OPTION_QUIT].text = "Quit";
    ui->menu_index = 0;

    ui->current_panel = PANEL_CHARACTER;

    ui->panel_status[PANEL_CHARACTER].scroll = 0;
    ui->panel_status[PANEL_CHARACTER].current_index = 0;
    ui->panel_status[PANEL_CHARACTER].max_index = 0;

    ui->panel_status[PANEL_EXAMINE].scroll = 0;
    ui->panel_status[PANEL_EXAMINE].current_index = 0;
    ui->panel_status[PANEL_EXAMINE].max_index = 0;

    ui->panel_status[PANEL_INVENTORY].scroll = 0;
    ui->panel_status[PANEL_INVENTORY].current_index = 0;
    ui->panel_status[PANEL_INVENTORY].max_index = 0;

    ui->panel_status[PANEL_SPELLBOOK].scroll = 0;
    ui->panel_status[PANEL_SPELLBOOK].current_index = 0;
    ui->panel_status[PANEL_SPELLBOOK].max_index = 0;

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
    ui->tooltip_width = 15;
    ui->tooltip_height = 10;
    ui->tooltip_options = TCOD_list_new();

    ui->view_width = 0;
    ui->view_height = 0;
    ui->view_x = 0;
    ui->view_y = 0;

    ui->mouse_x = 0;
    ui->mouse_y = 0;
    ui->mouse_tile_x = 0;
    ui->mouse_tile_y = 0;

    return ui;
}

void ui_update(struct ui *ui, struct game *game)
{
    switch (game->state)
    {
    case STATE_PLAYING:
    {
        ui->panel_status[PANEL_INVENTORY].max_index = TCOD_list_size(game->player->items) - 1;

        ui->message_log_x = 0;
        ui->message_log_height = console_height / 4;
        ui->message_log_y = console_height - ui->message_log_height;
        ui->message_log_width = console_width;

        ui->panel_width = console_width / 2;
        ui->panel_x = console_width - ui->panel_width;
        ui->panel_y = 0;
        ui->panel_height = console_height - (ui->message_log_visible ? ui->message_log_height : 0);

        ui->tooltip_width = 15;
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

        {
            struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

            if (panel_status->current_index < 0)
            {
                panel_status->current_index = 0;
            }
            else if (panel_status->current_index > panel_status->max_index)
            {
                panel_status->current_index = panel_status->max_index;
            }
        }
    }
    break;
    }
}

bool ui_message_log_is_inside(struct ui *ui, int x, int y)
{
    return ui->message_log_visible && x >= ui->message_log_x && x < ui->message_log_x + ui->message_log_width && y >= ui->message_log_y && y < ui->message_log_y + ui->message_log_height;
}

bool ui_panel_is_inside(struct ui *ui, int x, int y)
{
    return ui->panel_visible && x >= ui->panel_x && x < ui->panel_x + ui->panel_width && y >= ui->panel_y && y < ui->panel_y + ui->panel_height;
}

void ui_panel_toggle(struct ui *ui, enum panel panel)
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

bool ui_tooltip_is_inside(struct ui *ui, int x, int y)
{
    return ui->tooltip_visible && x >= ui->tooltip_x && x < ui->tooltip_x + ui->tooltip_width && y >= ui->tooltip_y && y < ui->tooltip_y + ui->tooltip_height;
}

void ui_tooltip_show(struct ui *ui)
{
    ui_tooltip_options_clear(ui);

    ui->tooltip_visible = true;
    ui->tooltip_x = ui->mouse_x;
    ui->tooltip_y = ui->mouse_y;
}

void ui_tooltip_hide(struct ui *ui)
{
    ui_tooltip_options_clear(ui);

    ui->tooltip_visible = false;
}

void ui_tooltip_options_add(struct ui *ui, char *text, bool (*fn)(struct game *game, struct input *input, struct tooltip_data data), struct tooltip_data data)
{
    struct tooltip_option *option = (struct tooltip_option *)calloc(1, sizeof(struct tooltip_option));

    option->text = text;
    option->fn = fn;
    option->data = data;

    TCOD_list_push(ui->tooltip_options, option);
}

void ui_tooltip_options_clear(struct ui *ui)
{
    for (void **i = TCOD_list_begin(ui->tooltip_options); i != TCOD_list_end(ui->tooltip_options); i++)
    {
        struct tooltip_option *option = *i;

        i = TCOD_list_remove_iterator(ui->tooltip_options, i);

        free(option);
    }
}

struct tooltip_option *ui_tooltip_get_selected(struct ui *ui)
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

    return NULL;
}

bool ui_view_is_inside(struct ui *ui, int x, int y)
{
    return x >= 0 && x < ui->view_width && y >= 0 && y < ui->view_height;
}

void ui_destroy(struct ui *ui)
{
    for (void **i = TCOD_list_begin(ui->tooltip_options); i != TCOD_list_end(ui->tooltip_options); i++)
    {
        struct tooltip_option *option = *i;

        free(option);
    }

    TCOD_list_delete(ui->tooltip_options);

    free(ui);
}
