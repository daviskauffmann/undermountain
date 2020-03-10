#include <libtcod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "scene.h"
#include "game/assets.h"
#include "scenes/scene_menu.h"

int main(int argc, char *argv[])
{
    config_load();
    assets_load();

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printf("Options:\n");
            printf("  -h, --help\tPrint this message\n");
            printf("  -v, --version\tPrint current version\n");
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            printf("%s %s\n", TITLE, VERSION);
        }
        if (strcmp(argv[i], "--width") == 0)
        {
            console_width = atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "--height") == 0)
        {
            console_height = atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fullscreen") == 0)
        {
            fullscreen = true;
        }
        if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--windowed") == 0)
        {
            fullscreen = false;
        }
    }

    TCOD_sys_set_fps(FPS);
    TCOD_console_set_custom_font(font_file, font_flags, font_layout_width, font_layout_height);
    if (TCOD_console_init_root(console_width, console_height, TITLE, fullscreen, console_renderer) != 0)
    {
        printf("Error: Couldn't initialize root console.\n%s\n", TCOD_get_error());
        return 1;
    }

    struct scene *scene = &menu_scene;
    scene->init(NULL);

    while (!TCOD_console_is_window_closed())
    {
        TCOD_key_t key;
        TCOD_mouse_t mouse;
        TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);
        switch (ev)
        {
        case TCOD_EVENT_KEY_PRESS:
        {
            switch (key.vk)
            {
            case TCODK_ENTER:
            {
                if (key.lalt)
                {
                    TCOD_console_set_fullscreen((fullscreen = !fullscreen));
                }
            }
            break;
            default:
                break;
            }
        }
        break;
        default:
            break;
        }

        if (!(scene = scene->handle_event(ev, key, mouse)))
        {
            break;
        }

        if (!(scene = scene->update(TCOD_sys_get_last_frame_length())))
        {
            break;
        }

        TCOD_console_clear(NULL);
        scene->render(NULL);
        TCOD_console_flush();
    }

    if (scene)
    {
        scene->quit();
    }

    TCOD_console_delete(NULL);
    TCOD_quit();

    return 0;
}
