#include "scene_about.h"

#include "../config.h"
#include "../scene.h"
#include "scene_game.h"
#include "scene_menu.h"
#include <libtcod.h>

static void init(struct scene *previous_scene)
{
}

static struct scene *handle_event(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse)
{
    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            about_scene.quit();
            menu_scene.init(&about_scene);
            return &menu_scene;
        }
        break;
        default:
            break;
        }
    }
    break;
    case TCOD_EVENT_MOUSE_PRESS:
    {
        if (mouse.rbutton)
        {
            about_scene.quit();
            menu_scene.init(&about_scene);
            return &menu_scene;
        }
    }
    break;
    default:
        break;
    }

    return &about_scene;
}

static struct scene *update(float delta_time)
{
    return &about_scene;
}

static void render(TCOD_console_t console)
{
    int y = 1;
    y += TCOD_console_printf_rect(
        console,
        1,
        y,
        console_width - 2,
        console_height - 2,
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");
    TCOD_console_printf_rect(
        console,
        1,
        y + 1,
        console_width - 2,
        console_height - 2,
        "Press ESC or R-Mouse to return.");

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

struct scene about_scene =
    {&init,
     &handle_event,
     &update,
     &render,
     &quit};
