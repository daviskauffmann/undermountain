#ifndef UI_H
#define UI_H

#include <libtcod/libtcod.h>

struct game;

enum menu_state
{
    MENU_STATE_MAIN,
    MENU_STATE_ABOUT
};

enum menu_option
{
    MENU_OPTION_START,
    MENU_OPTION_ABOUT,
    MENU_OPTION_QUIT,

    NUM_MENU_OPTIONS
};

struct menu_option_info
{
    const char *text;
};

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
    enum menu_state menu_state;
    struct menu_option_info menu_option_info[NUM_MENU_OPTIONS];
    int menu_index;
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
