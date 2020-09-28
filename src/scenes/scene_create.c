#include "scene_create.h"

#include <libtcod.h>

#include "scene_menu.h"
#include "scene_game.h"
#include "../config.h"
#include "../scene.h"
#include "../game/world.h"

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
        case TCODK_ENTER:
        {
            world_setup();
            world_create();

            create_scene.quit();
            game_scene.init(&create_scene);
            return &game_scene;
        }
        break;
        case TCODK_ESCAPE:
        {
            create_scene.quit();
            menu_scene.init(&create_scene);
            return &menu_scene;
        }
        break;
        default:
        {
        }
        break;
        }
    }
    break;
    case TCOD_EVENT_MOUSE_PRESS:
    {
        if (mouse.lbutton)
        {
            world_setup();
            world_create();

            create_scene.quit();
            game_scene.init(&create_scene);
            return &game_scene;
        }
        else if (mouse.rbutton)
        {
            create_scene.quit();
            menu_scene.init(&create_scene);
            return &menu_scene;
        }
    }
    break;
    default:
    {
    }
    break;
    }

    return &create_scene;
}

static struct scene *update(float delta_time)
{
    return &create_scene;
}

static void render(TCOD_console_t console)
{
    int y = 1;
    TCOD_console_printf_rect(
        NULL,
        1,
        y++,
        console_width - 2,
        console_height - 2,
        "TODO: character creation.");
    TCOD_console_printf_rect(
        NULL,
        1,
        y++,
        console_width - 2,
        console_height - 2,
        "Press ENTER or L-Mouse to start.");
    TCOD_console_printf_rect(
        NULL,
        1,
        y++,
        console_width - 2,
        console_height - 2,
        "Press ESC or R-Mouse to return.");

    TCOD_console_set_default_foreground(NULL, TCOD_white);
    TCOD_console_printf_frame(
        NULL,
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

struct scene create_scene =
    {&init,
     &handle_event,
     &update,
     &render,
     &quit};
