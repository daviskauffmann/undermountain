#include <libtcod/libtcod.h>
#include <stdio.h>

#include "game.h"
#include "window.h"

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    window_init();

    struct game *game = game_create();

    if (TCOD_sys_file_exists("../saves/save.gz"))
    {
        game_load(game);
    }
    else
    {
        game_new(game);
    }

    while (!TCOD_console_is_window_closed())
    {
        game_input(game);
        game_update(game);
        game_render(game);

        if (game->should_restart)
        {
            game_destroy(game);
            game = game_create();
            game_new(game);
        }

        if (game->should_quit)
        {
            break;
        }
    }

    game_destroy(game);

    return 0;
}
