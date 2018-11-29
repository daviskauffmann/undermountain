#ifndef PLATFORM_UI_H
#define PLATFORM_UI_H

#include "tooltip_option.h"

struct game;
struct input;
struct item;

enum ui_state
{
    UI_STATE_MENU,
    UI_STATE_GAME
};

enum menu_state
{
    MENU_STATE_MAIN,
    MENU_STATE_LOAD,
    MENU_STATE_ABOUT
};

enum main_menu_option
{
    MAIN_MENU_OPTION_NEW,
    MAIN_MENU_OPTION_LOAD,
    MAIN_MENU_OPTION_ABOUT,
    MAIN_MENU_OPTION_QUIT,

    NUM_MAIN_MENU_OPTIONS
};

struct main_menu_option_info
{
    char *text;
};

enum targeting
{
    TARGETING_NONE,
    TARGETING_LOOK,
    TARGETING_EXAMINE,
    TARGETING_SHOOT,
    TARGETING_SPELL
};

enum panel
{
    PANEL_CHARACTER,
    PANEL_EXAMINE,
    PANEL_INVENTORY,
    PANEL_SPELLBOOK,

    NUM_PANELS
};

struct panel_status
{
    int scroll;
    bool selection_mode;
};

struct ui
{
    enum ui_state state;
    enum menu_state menu_state;
    struct main_menu_option_info main_menu_option_info[NUM_MAIN_MENU_OPTIONS];
    enum targeting targeting;
    int target_x;
    int target_y;
    enum panel_type current_panel;
    struct panel_status panel_status[NUM_PANELS];
    bool message_log_visible;
    int message_log_x;
    int message_log_height;
    int message_log_y;
    int message_log_width;
    bool panel_visible;
    int panel_width;
    int panel_x;
    int panel_y;
    int panel_height;
    bool tooltip_visible;
    int tooltip_x;
    int tooltip_y;
    int tooltip_width;
    int tooltip_height;
    TCOD_list_t tooltip_options;
    int view_width;
    int view_height;
    int view_x;
    int view_y;
    int mouse_x;
    int mouse_y;
    int mouse_tile_x;
    int mouse_tile_y;
};

extern struct ui *ui;

void ui_init(void);
void ui_update(void);
enum main_menu_option ui_main_menu_get_selected(void);
bool ui_message_log_is_inside(int x, int y);
bool ui_panel_is_inside(int x, int y);
void ui_panel_toggle(enum panel panel);
void ui_panel_show(enum panel panel);
enum equip_slot ui_panel_character_get_selected(void);
struct item *ui_panel_inventory_get_selected(void);
bool ui_tooltip_is_inside(int x, int y);
void ui_tooltip_show(void);
void ui_tooltip_hide(void);
void ui_tooltip_options_add(char *text, struct tooltip_data tooltip_data, bool(*fn)(struct tooltip_data tooltip_data));
void ui_tooltip_options_clear(void);
struct tooltip_option *ui_tooltip_get_selected(void);
bool ui_view_is_inside(int x, int y);
void ui_quit(void);

#endif
