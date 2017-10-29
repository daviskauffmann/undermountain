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

    goto draw;

    while (!TCOD_console_is_window_closed())
    {
        switch (input_handle())
        {
        case INPUT_TICK:
            goto tick;

        case INPUT_DRAW:
            goto draw;

        case INPUT_TURN:
            goto turn;

        case INPUT_QUIT:
            goto quit;
        }

    turn:
        world_turn();

    draw:
        TCOD_console_clear(NULL);
        view_update();
        world_draw_turn();

    tick:
        world_tick();
        world_draw_tick();
        TCOD_console_flush();
    }

quit:
    world_destroy();

    SDL_Quit();

    return 0;
}