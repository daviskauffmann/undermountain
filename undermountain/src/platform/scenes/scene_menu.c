#include "scene_menu.h"

#include <string.h>

#include <libtcod.h>

#include "scene_about.h"
#include "scene_game.h"
#include "../config.h"
#include "../scene.h"
#include "../sys.h"

#include "../../game/world.h"

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

static int mouse_x;
static int mouse_y;

static struct option_info option_info[NUM_OPTIONS];

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

static struct scene *select_option(enum option option)
{
    switch (option)
    {
    case OPTION_START:
    {
        world_init();
        if (file_exists(SAVE_PATH))
        {
            // TODO: prompt whether the player wants to overwrite the save with a new character
            // if so, go to character creation
            world_load(SAVE_PATH);
        }
        else
        {
            // TODO: go to character creation and pass the result to world_new()
            world_new();
        }

        menu_scene.quit();
        game_scene.init(&menu_scene);
        return &game_scene;
    }
    break;
    case OPTION_ABOUT:
    {
        menu_scene.quit();
        about_scene.init(&menu_scene);
        return &about_scene;
    }
    break;
    case OPTION_QUIT:
    {
        menu_scene.quit();
        return NULL;
    }
    break;
    }

    return &menu_scene;
}

static void init(struct scene *previous_scene)
{
    option_info[OPTION_START].text = "Start";
    option_info[OPTION_ABOUT].text = "About";
    option_info[OPTION_QUIT].text = "Quit";

    mouse_x = -1;
    mouse_y = -1;
}

static struct scene *handle_event(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse)
{
    mouse_x = mouse.cx;
    mouse_y = mouse.cy;

    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            return NULL;
        }
        break;
        case TCODK_TEXT:
        {
            int alpha = key.text[0] - 'a';
            enum option option = (enum option)alpha;
            return select_option(option);
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
            return select_option(option);
        }
    }
    break;
    }

    return &menu_scene;
}

static struct scene *update(float delta)
{
    return &menu_scene;
}

static void render(TCOD_console_t console)
{
    int y = 1;
    for (enum option option = 0; option < NUM_OPTIONS; option++)
    {
        TCOD_console_set_default_foreground(console, option == get_selected_option() ? TCOD_yellow : TCOD_white);
        TCOD_console_printf(
            console,
            1,
            y++,
            "%c) %s",
            option + 'a',
            option_info[option].text);
    }

    TCOD_console_set_default_foreground(console, TCOD_white);
    TCOD_console_printf_frame(
        console,
        0,
        0,
        console_width,
        console_height,
        false,
        TCOD_BKGND_SET,
        TITLE);
}

static void quit(void)
{
}

struct scene menu_scene =
    {&init,
     &handle_event,
     &update,
     &render,
     &quit};
