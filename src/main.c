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

    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(screen_width, screen_height, WINDOW_TITLE, fullscreen, renderer);
    TCOD_sys_set_fps(FPS);
    TCOD_console_set_default_background(NULL, default_background_color);
    TCOD_console_set_default_foreground(NULL, default_foreground_color);

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