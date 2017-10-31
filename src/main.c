#include <stdio.h>
#include <SDL.h>
#include <time.h>
#include <libtcod.h>

#include "config.h"
#include "console.h"
#include "world.h"
#include "game.h"
#include "input.h"

int main(int argc, char *argv[])
{
    config_init();
    console_init();

    world_init();
    game_init();

    goto draw;

    while (!TCOD_console_is_window_closed())
    {
        switch (input_handle())
        {
        case INPUT_TICK:
            goto tick;

        case INPUT_TURN:
            goto turn;

        case INPUT_DRAW:
            goto draw;

        case INPUT_QUIT:
            goto quit;
        }

    turn:
        world_turn();

        switch (TCOD_random_get_int(NULL, 0, 2))
        {
        case 0:
            console_log("Hello, World!");
            break;

        case 1:
            console_log("Greetings, World!");
            break;

        case 2:
            console_log("Saluations, World!");
            break;
        }

    draw:
        console_turn_draw();

    tick:
        world_tick();
        console_tick_draw();
    }

quit:
    SDL_Quit();

    return 0;
}