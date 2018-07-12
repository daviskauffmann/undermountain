#ifndef UI_H
#define UI_H

#include <libtcod/libtcod.h>

struct game;

enum panel
{
    PANEL_CHARACTER,
    PANEL_INVENTORY,
    PANEL_SPELLBOOK,

    NUM_PANELS
};

struct panel_status
{
    int scroll;
    int current_index;
    int max_index;
};

struct ui
{
    enum panel_type current_panel;
    struct panel_status panel_status[NUM_PANELS];
    bool message_log_visible;
    bool panel_visible;
};

struct ui *ui_create(void);
void ui_update(struct ui *ui, struct game *game);
void ui_panel_toggle(struct ui *ui, enum panel panel);
void ui_destroy(struct ui *ui);

#endif
