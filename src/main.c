#include "config.h"
#include "game/assets.h"
#include "scene.h"
#include "scenes/menu/menu_scene.h"
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
            printf("  -c, --columns\tSet the width of the console in characters\n");
            printf("  -r, --rows\tSet the height of the console in characters\n");
            printf("  -h, --help\tPrint this message\n");
            printf("  -v, --version\tPrint current version\n");
            return 0;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            printf("%s\n", VERSION);
            return 0;
        }
        if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--columns") == 0)
        {
            console_width = atoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--rows") == 0)
        {
            console_height = atoi(argv[i + 1]);
        }
    }

    TCOD_Console *const console = TCOD_console_new(console_width, console_height);
    if (!console)
    {
        printf("Error: Couldn't initialize console: \n%s\n", TCOD_get_error());
        return 1;
    }

    TCOD_Tileset *tileset = TCOD_tileset_load(
        tileset_filename,
        tileset_columns,
        tileset_rows,
        tileset_columns * tileset_rows,
        tileset_charmap);
    if (!tileset)
    {
        printf("Error: Couldn't load tileset: \n%s\n", TCOD_get_error());
        return 1;
    }

    const TCOD_ContextParams params = {
        .tcod_version = TCOD_COMPILEDVERSION,
        .console = console,
        .window_title = TITLE,
        .sdl_window_flags = SDL_WINDOW_RESIZABLE,
        .renderer_type = TCOD_RENDERER_SDL2,
        .tileset = tileset,
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

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            TCOD_context_convert_event_coordinates(context, &event);

            switch (event.type)
            {
            case SDL_QUIT:
            {
                running = false;
            }
            break;
            }

            if (scene)
            {
                scene = scene->handle_event(&event);
                if (!scene)
                {
                    running = false;
                    break;
                }
            }
        }

        TCOD_console_clear(console);

        if (scene)
        {
            scene = scene->update(console, delta_time);
            if (!scene)
            {
                running = false;
                break;
            }
        }

        TCOD_context_present(context, console, NULL);
    }

    if (scene)
    {
        scene->uninit();
    }

    TCOD_tileset_delete(tileset);
    TCOD_console_delete(console);
    TCOD_quit();

    return 0;
}
