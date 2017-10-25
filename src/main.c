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
        switch (input_handle())
        {
        case INPUT_NONE:
            continue;

        case INPUT_UPDATE:
            world_update();

            goto redraw;

        case INPUT_RESTART:
            world_destroy();
            game_init();

            goto redraw;

        case INPUT_LOAD:
            world_destroy();
            game_load();

            goto redraw;

        case INPUT_QUIT:
            goto quit;
        }

    redraw:
        view_update();
        view_render();
    }

quit:

    world_destroy();

    SDL_Quit();

    return 0;
}