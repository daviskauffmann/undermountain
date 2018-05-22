#include <libtcod/libtcod.h>
#include <malloc.h>

#include "game.h"
#include "ui.h"

struct ui *ui_create(void)
{
    struct ui *ui = malloc(sizeof(struct ui));

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
    ui->panel_status[PANEL_INVENTORY].max_index = TCOD_list_size(game->player->items) - 1;
}

void ui_panel_toggle(struct ui *ui, enum panel_type panel_type)
{
    if (ui->panel_visible)
    {
        if (ui->current_panel == panel_type)
        {
            ui->panel_visible = false;
        }
        else
        {
            ui->current_panel = panel_type;
        }
    }
    else
    {
        ui->current_panel = panel_type;
        ui->panel_visible = true;
    }
}

void ui_destroy(struct ui *ui)
{
    free(ui);
}
