#ifndef UI_H
#define UI_H

#include <libtcod/libtcod.h>

struct game;
struct input;
struct item;

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
    int current_index;
    int max_index;
};

struct tooltip_data
{
    int x;
    int y;
    struct item *item;
};

struct tooltip_option
{
    char *text;
    bool (*fn)(struct game *game, struct input *input, struct tooltip_data data);
    struct tooltip_data data;
};

struct ui
{
    enum menu_state menu_state;
    struct menu_option_info menu_option_info[NUM_MENU_OPTIONS];
    int menu_index;
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
    bool should_restart;
};

struct ui *ui_create(void);
void ui_update(struct ui *ui, struct engine *engine, struct game *game);
bool ui_message_log_is_inside(struct ui *ui, int x, int y);
bool ui_panel_is_inside(struct ui *ui, int x, int y);
void ui_panel_toggle(struct ui *ui, enum panel panel);
bool ui_tooltip_is_inside(struct ui *ui, int x, int y);
void ui_tooltip_show(struct ui *ui);
void ui_tooltip_hide(struct ui *ui);
void ui_tooltip_options_add(struct ui *ui, char *text, bool (*fn)(struct game *game, struct input *input, struct tooltip_data data), struct tooltip_data data);
void ui_tooltip_options_clear(struct ui *ui);
struct tooltip_option *ui_tooltip_get_selected(struct ui *ui);
bool ui_view_is_inside(struct ui *ui, int x, int y);
void ui_destroy(struct ui *ui);

#endif
