#include <platform/platform.h>
#include <roguelike/roguelike.h>

enum option
{
    OPTION_START,
    OPTION_ABOUT,
    OPTION_QUIT,

    NUM_OPTIONS
};

struct option_info
{
    char *text;
};

static struct option_info option_info[NUM_OPTIONS];
static int mouse_x;
static int mouse_y;

static enum option get_selected_option(void)
{
    int y = 1;
    for (enum option option = 0; option < NUM_OPTIONS; option++)
    {
        if (mouse_x > 0 && mouse_x < (int)strlen(option_info[option].text) + 3 + 1 && mouse_y == y)
        {
            return option;
        }

        y++;
    }

    return -1;
}

static bool select_option(enum option option)
{
    switch (option)
    {
    case OPTION_START:
    {
        game_init();

        if (file_exists(SAVE_PATH))
        {
            // TODO: prompt whether the player wants to overwrite the save with a new character
            // if so, go to character creation
            game_load();
        }
        else
        {
            // TODO: go to character creation and pass the result to game_new()
            game_new();
        }

        platform_set_state(&game_state);
    }
    break;
    case OPTION_ABOUT:
    {
        platform_set_state(&about_state);
    }
    break;
    case OPTION_QUIT:
    {
        return true;
    }
    break;
    }

    return false;
}

static void init(void)
{
    option_info[OPTION_START].text = "Start";
    option_info[OPTION_ABOUT].text = "About";
    option_info[OPTION_QUIT].text = "Quit";

    mouse_x = -1;
    mouse_y = -1;
}

static bool handleEvent(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse)
{
    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            return true;
        }
        break;
        case TCODK_TEXT:
        {
            int alpha = key.text[0] - 'a';

            enum option option = (enum option)alpha;

            if (select_option(option))
            {
                return true;
            }
        }
        break;
        }
    }
    break;
    case TCOD_EVENT_MOUSE_PRESS:
    {
        if (mouse.lbutton)
        {
            enum option option = get_selected_option();

            if (select_option(option))
            {
                return true;
            }
        }
    }
    break;
    }

    mouse_x = mouse.cx;
    mouse_y = mouse.cy;

    return false;
}

static void update(void)
{
}

static void render(void)
{
    int y = 1;
    for (enum option option = 0; option < NUM_OPTIONS; option++)
    {
        TCOD_console_set_default_foreground(NULL, option == get_selected_option() ? TCOD_yellow : TCOD_white);
        TCOD_console_printf(NULL, 1, y++, "%c) %s", option + 'a', option_info[option].text);
    }

    TCOD_console_set_default_foreground(NULL, TCOD_white);
    TCOD_console_printf_frame(NULL, 0, 0, console_width, console_height, false, TCOD_BKGND_SET, TITLE);
}

static void quit(void)
{
}

struct state menu_state = {
    &init,
    &handleEvent,
    &update,
    &render,
    &quit
};
