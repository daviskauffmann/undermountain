#include <stdint.h>
#include <stdio.h>
#include <SDL.h>
#include <libtcod.h>

#include "config.h"
#include "game.h"
#include "view.h"
#include "input.h"
#include "world.h"

int main(int argc, char *argv[])
{
    config_init();
    game_init();

    view_update();
    view_render();

    while (!TCOD_console_is_window_closed())
    {
        input_t input = input_handle();

        if (input == INPUT_NONE)
        {
            continue;
        }

        view_update();

        switch (input)
        {
        case INPUT_UPDATE:
            world_update();
            view_render();

            break;

        case INPUT_RESTART:
            world_destroy();
            game_init();

            view_update();
            view_render();

            break;

        case INPUT_LOAD:
            world_destroy();
            game_load();

            view_update();
            view_render();

            break;

        case INPUT_QUIT:
            goto quit;
        }
    }
quit:

    world_destroy();

    SDL_Quit();

    return 0;
}