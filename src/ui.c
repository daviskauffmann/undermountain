#include <libtcod/libtcod.h>
#include <malloc.h>

#include "game.h"
#include "ui.h"

#include "CMemleak.h"

struct ui *ui_create(void)
{
    struct ui *ui = malloc(sizeof(struct ui));

    ui->menu_state = MENU_STATE_MAIN;
    ui->menu_option_info[MENU_OPTION_START].text = "Start";
    ui->menu_option_info[MENU_OPTION_ABOUT].text = "About";
    ui->menu_option_info[MENU_OPTION_QUIT].text = "Quit";
    ui->menu_index = 0;

    ui->current_panel = PANEL_CHARACTER;

    ui->panel_status[PANEL_CHARACTER].scroll = 0;
    ui->panel_status[PANEL_CHARACTER].current_index = 0;
    ui->panel_status[PANEL_CHARACTER].max_index = 0;

    ui->panel_status[PANEL_INVENTORY].scroll = 0;
    ui->panel_status[PANEL_INVENTORY].current_index = 0;
    ui->panel_status[PANEL_INVENTORY].max_index = 0;

    ui->panel_status[PANEL_SPELLBOOK].scroll = 0;
    ui->panel_status[PANEL_SPELLBOOK].current_index = 0;
    ui->panel_status[PANEL_SPELLBOOK].max_index = 0;

    ui->message_log_visible = true;
    ui->panel_visible = false;

    return ui;
}

void ui_update(struct ui *ui, struct game *game)
{
    switch (game->state)
    {
    case STATE_PLAYING:
    {
        ui->panel_status[PANEL_INVENTORY].max_index = TCOD_list_size(game->player->items) - 1;

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

void ui_destroy(struct ui *ui)
{
    free(ui);
}
