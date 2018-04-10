#include <libtcod/libtcod.h>
#include <stdio.h>

#include "config.h"
#include "game.h"

int main(int argc, char *argv[])
{
    // suppress warning C4100
    (void)argc;
    (void)argv;

    config_init();

    TCOD_sys_set_fps(FPS);

    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(console_width, console_height, WINDOW_TITLE, fullscreen, renderer);

    struct game *game = game_create();

    if (TCOD_sys_file_exists(SAVE_PATH))
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
