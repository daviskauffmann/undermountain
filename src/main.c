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

    while (!TCOD_console_is_window_closed() && game_status != GAME_STATUS_QUIT)
    {
        game_update();
        game_draw();
    }

    game_uninit();

    SDL_Quit();

    return 0;
}