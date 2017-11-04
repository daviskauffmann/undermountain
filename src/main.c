#include <stdio.h>
#include <SDL.h>
#include <time.h>
#include <libtcod.h>

#include "system.h"
#include "menu.h"
#include "game.h"

int main(int argc, char *argv[])
{
    system_init();

    // while (!TCOD_console_is_window_closed())
    // {
    //     switch (menu_input())
    //     {
    //     case MENU_INPUT_START:
    //         game_initialize();

    //         goto draw;

    //     case MENU_INPUT_LOAD:
    //         game_load();

    //         goto draw;

    //     case MENU_INPUT_QUIT:
    //         goto quit;
    //     }
    // }

    game_initialize();

    goto draw;

    while (!TCOD_console_is_window_closed())
    {
        switch (game_input())
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
        game_turn();

    draw:
        game_draw_turn();

    tick:
        game_tick();
        game_draw_tick();
    }

quit:
    SDL_Quit();

    return 0;
}