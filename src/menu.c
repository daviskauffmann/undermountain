#include "menu.h"
#include "config.h"
#include "scene.h"
#include "ui.h"
#include <curses.h>
#include <string.h>

extern struct scene create_scene;

enum menu_option
{
    MENU_OPTION_START,
    MENU_OPTION_QUIT,

    MENU_NUM_OPTIONS
};

struct menu_item
{
    const char *text;
    char hotkey;
};

static const struct menu_item menu_items[MENU_NUM_OPTIONS] = {
    {"Start", 's'},
    {"Quit", 'q'}};

static int selected_option = 0;

static void draw_menu_item(int y, const struct menu_item *item, bool selected)
{
    int max_x = getmaxx(stdscr);
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "[%c] %s", item->hotkey, item->text);

    int x = (max_x - (int)strlen(buffer)) / 2;

    if (selected)
    {
        attron(A_REVERSE | A_BOLD);
    }

    mvprintw(y, x, "%s", buffer);

    if (selected)
    {
        attroff(A_REVERSE | A_BOLD);
    }
}

static void init(const struct scene *const previous_scene)
{
    (void)previous_scene;

    selected_option = 0;
}

static void uninit(void) {}

static void handle_input(int ch, struct scene **next_scene)
{
    switch (ch)
    {
    case KEY_UP:
    case 'k':
    case 'K':
    {
        selected_option = (selected_option - 1 + MENU_NUM_OPTIONS) % MENU_NUM_OPTIONS;
    }
    break;
    case KEY_DOWN:
    case 'j':
    case 'J':
    {
        selected_option = (selected_option + 1) % MENU_NUM_OPTIONS;
    }
    break;
    case '\n':
    case '\r':
    case KEY_ENTER:
    {
        if (selected_option == MENU_OPTION_START)
        {
            *next_scene = &create_scene;
        }
        else if (selected_option == MENU_OPTION_QUIT)
        {
            *next_scene = NULL;
        }
    }
    break;
    case 27:
    {
        *next_scene = NULL;
    }
    break;
    default:
    {
        for (int i = 0; i < MENU_NUM_OPTIONS; i++)
        {
            if (ch == menu_items[i].hotkey || ch == menu_items[i].hotkey - 32)
            {
                selected_option = i;

                if (i == MENU_OPTION_START)
                {
                    *next_scene = &create_scene;
                }
                else if (i == MENU_OPTION_QUIT)
                {
                    *next_scene = NULL;
                }

                break;
            }
        }
    }
    break;
    }
}

static void render(void)
{
    int max_y = getmaxy(stdscr);

    ui_screen_frame(TITLE);

    int start_y = max_y / 2 - 1;

    for (int i = 0; i < MENU_NUM_OPTIONS; i++)
    {
        draw_menu_item(start_y + i, &menu_items[i], i == selected_option);
    }

    ui_footer_hint("[↑↓/jk] select  [Enter] confirm  [ESC] quit");

    refresh();
}

static struct scene *update(const float delta_time)
{
    (void)delta_time;

    struct scene *next_scene = &menu_scene;
    int ch = getch();

    if (ch != ERR)
    {
        handle_input(ch, &next_scene);
    }

    if (next_scene == &menu_scene)
    {
        render();
    }

    return next_scene;
}

struct scene menu_scene = {
    &init,
    &uninit,
    &update};
