#include <SDL.h>
#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "config.h"
#include "game.h"

static game_t game;

int main(int argc, char *argv[])
{
    config_init();

    game_init(&game);
    game_new(&game);

    while (!TCOD_console_is_window_closed())
    {
        game_input(&game);
        game_update(&game);
        game_render(&game);

        if (game.should_restart)
        {
            game_reset(&game);
            game_init(&game);
            game_new(&game);
        }

        if (game.should_quit)
        {
            break;
        }
    }

    game_reset(&game);

    SDL_Quit();

    return 0;
}