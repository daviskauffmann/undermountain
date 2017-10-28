#include <stdio.h>
#include <SDL.h>
#include <time.h>
#include <libtcod.h>

#include "config.h"
#include "world.h"
#include "game.h"
#include "view.h"
#include "input.h"

int main(int argc, char *argv[])
{
    config_init();
    world_init();
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
            goto update;

        case INPUT_REDRAW:
            goto redraw;

        case INPUT_QUIT:
            goto quit;
        }

    update:
        world_update();

    redraw:
        view_update();
        view_render();
    }

quit:
    world_destroy();

    SDL_Quit();

    return 0;
}