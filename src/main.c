#include <SDL.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "config.h"
#include "game.h"

int main(int argc, char *argv[])
{
    config_init();

    game_init();
    game_new();

    while (!TCOD_console_is_window_closed())
    {
        game_input();
        game_update();
        game_render();

        if (should_quit)
        {
            break;
        }
    }

    game_reset();

    SDL_Quit();

    return 0;
}