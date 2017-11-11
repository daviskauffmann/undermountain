#include <stdio.h>
#include <SDL.h>
#include <time.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

int main(int argc, char *argv[])
{
    system_init();

    game_init();
    game_new();

    goto draw;

    while (!TCOD_console_is_window_closed())
    {
        switch (input_handle())
        {
        case GAME_INPUT_TICK:
            goto tick;

        case GAME_INPUT_TURN:
            goto turn;

        case GAME_INPUT_DRAW:
            goto draw;

        case GAME_INPUT_QUIT:
            goto quit;
        }

    turn:
        game_turn();

    draw:
        game_draw_turn();

    tick:
        game_tick();
        game_draw_tick();
    }

quit:
    game_uninit();

    SDL_Quit();

    return 0;
}