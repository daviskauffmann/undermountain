#include "config.h"
#include "game/assets.h"
#include "scene.h"
#include "scenes/scene_menu.h"
#include <SDL2/SDL.h>
#include <libtcod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    config_load();
    assets_load();

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printf("Options:\n");
            printf("      --console-width\tSet the width of the console in characters\n");
            printf("      --console-height\tSet the height of the console in characters\n");
            printf("  -h, --help\tPrint this message\n");
            printf("  -v, --version\tPrint current version\n");
            return 0;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            printf("%s\n", VERSION);
            return 0;
        }
        if (strcmp(argv[i], "--console-width") == 0)
        {
            console_width = atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "--console-height") == 0)
        {
            console_height = atoi(argv[i + 1]);
        }
    }

    TCOD_Console *console = TCOD_console_new(console_width, console_height);
    if (!console)
    {
        printf("Error: Couldn't initialize console: \n%s\n", TCOD_get_error());
        return 1;
    }

    TCOD_ContextParams params = {
        .tcod_version = TCOD_COMPILEDVERSION,
        .console = console,
        .window_title = TITLE,
        .sdl_window_flags = SDL_WINDOW_RESIZABLE,
        .renderer_type = TCOD_RENDERER_SDL2,
        // .tileset = tileset,
        .vsync = true,
        .argc = argc,
        .argv = argv,
    };
    TCOD_Context *context;
    if (TCOD_context_new(&params, &context) != 0)
    {
        printf("Error: Couldn't initialize context: \n%s\n", TCOD_get_error());
        return 1;
    }

    struct scene *scene = &menu_scene;
    scene->init(NULL);

    uint64_t current_time = 0;
    bool running = true;
    while (running)
    {
        const uint64_t previous_time = current_time;
        current_time = SDL_GetTicks64();
        const float delta_time = (current_time - previous_time) / 1000.0f;

        // SDL_Event event;
        // while (SDL_PollEvent(&event))
        // {
        //     TCOD_context_convert_event_coordinates(context, &event);

        //     switch (event.type)
        //     {
        //     case SDL_QUIT:
        //     {
        //         running = false;
        //     }
        //     break;
        //     }

        //     if (!(scene = scene->handle_event(ev, key, mouse)))
        //     {
        //         break;
        //     }
        // }

        TCOD_key_t key;
        TCOD_mouse_t mouse;
        TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);
        if (!(scene = scene->handle_event(ev, key, mouse)))
        {
            break;
        }

        if (!(scene = scene->update(delta_time)))
        {
            break;
        }

        TCOD_console_clear(console);
        scene->render(console);
        TCOD_context_present(context, console, NULL);
    }

    if (scene)
    {
        scene->quit();
    }

    TCOD_console_delete(console);
    TCOD_quit();

    return 0;
}
