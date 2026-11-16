#include "menu.h"

#include "config.h"
#include "create.h"
#include "game.h"
#include "game/actor.h"
#include "game/event.h"
#include "game/world.h"
#include "scene.h"
#include "ui.h"
#include <ctype.h>
#include <curses.h>
#include <errno.h>
#include <string.h>

enum menu_option
{
    MENU_OPTION_START,
    MENU_OPTION_CONTINUE,
    MENU_OPTION_QUIT,

    MENU_NUM_OPTIONS
};

struct menu_item
{
    const char *text;
    char hotkey;
};

static const struct menu_item menu_items[MENU_NUM_OPTIONS] = {
    [MENU_OPTION_START] = {.text = "Start", .hotkey = 's'},
    [MENU_OPTION_CONTINUE] = {.text = "Continue", .hotkey = 'c'},
    [MENU_OPTION_QUIT] = {.text = "Quit", .hotkey = 'q'},
};

static int selected_option = 0;
static bool has_save;
static char menu_error[128];

static void draw_menu_item(const int y, const struct menu_item *const item, const bool selected)
{
    char buffer[64];

    snprintf(buffer, sizeof(buffer), "[%c] %s", item->hotkey, item->text);
    ui_centered(y, buffer, selected ? A_REVERSE | A_BOLD : 0);
}

static void init(const struct scene *const previous_scene)
{
    (void)previous_scene;

    selected_option = 0;
    menu_error[0] = '\0';
    FILE *const file = fopen(SAVE_PATH, "rb");
    has_save = file != NULL;
    if (file)
    {
        fclose(file);
    }
    else if (errno != ENOENT)
    {
        snprintf(menu_error, sizeof(menu_error), "Could not open save: %s", strerror(errno));
    }
}

static void uninit(void)
{
}

static void select_option(struct scene **const next_scene)
{
    switch (selected_option)
    {
    case MENU_OPTION_START:
        *next_scene = &create_scene;
        break;
    case MENU_OPTION_CONTINUE: {
        FILE *const file = fopen(SAVE_PATH, "rb");
        if (!file)
        {
            snprintf(menu_error, sizeof(menu_error), "Could not open save: %s", strerror(errno));
            return;
        }
        world_init();
        bool failed = !world_load(file);
        if (fclose(file) != 0)
        {
            failed = true;
        }
        if (failed)
        {
            snprintf(menu_error, sizeof(menu_error), "Could not read saved game (incompatible or damaged save).");
            world_uninit();
            return;
        }
        event_emit(&(struct game_event){
            .type = EVENT_GAME_RESUMED,
            .floor = -1,
            .x = 0,
            .y = 0,
            .actor = world->hero->name,
        });
        *next_scene = &game_scene;
        break;
    }
    case MENU_OPTION_QUIT:
        *next_scene = NULL;
        break;
    default:
        break;
    }
}

static void handle_input(const int ch, struct scene **const next_scene)
{
    switch (ch)
    {
    case KEY_UP:
    case 'k':
    case 'K': {
        selected_option = (selected_option - 1 + MENU_NUM_OPTIONS) % MENU_NUM_OPTIONS;
        if (!has_save && selected_option == MENU_OPTION_CONTINUE)
        {
            selected_option = MENU_OPTION_START;
        }
    }
    break;
    case KEY_DOWN:
    case 'j':
    case 'J': {
        selected_option = (selected_option + 1) % MENU_NUM_OPTIONS;
        if (!has_save && selected_option == MENU_OPTION_CONTINUE)
        {
            selected_option = MENU_OPTION_QUIT;
        }
    }
    break;
    case '\n':
    case '\r':
    case KEY_ENTER: {
        select_option(next_scene);
    }
    break;
    case 27: {
        *next_scene = NULL;
    }
    break;
    default: {
        for (int i = 0; i < MENU_NUM_OPTIONS; i++)
        {
            if (ch == menu_items[i].hotkey || ch == toupper(menu_items[i].hotkey))
            {
                if (i == MENU_OPTION_CONTINUE && !has_save)
                {
                    continue;
                }
                selected_option = i;

                select_option(next_scene);

                break;
            }
        }
    }
    break;
    }
}

static void render(void)
{
    if (!ui_require_size(24, 10))
    {
        return;
    }

    const int max_y = getmaxy(stdscr);

    ui_screen_frame(TITLE);

    int start_y = max_y / 2 - 1;

    for (int i = 0; i < MENU_NUM_OPTIONS; i++)
    {
        if (i == MENU_OPTION_CONTINUE && !has_save)
        {
            continue;
        }
        draw_menu_item(start_y++, &menu_items[i], i == selected_option);
    }
    if (menu_error[0])
    {
        ui_centered(max_y - 3, menu_error, A_BOLD);
    }

    ui_footer_hint("[↑↓/jk] select  [Enter] confirm  [ESC] quit");

    refresh();
}

static struct scene *update(const float delta_time)
{
    (void)delta_time;

    struct scene *next_scene = &menu_scene;
    const int ch = ui_read_key();

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
    .init = init,
    .uninit = uninit,
    .update = update,
};
