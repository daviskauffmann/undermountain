#include <libtcod/libtcod.h>
#include <stdio.h>

#include "assets.h"
#include "config.h"
#include "input.h"
#include "game.h"
#include "program.h"
#include "renderer.h"
#include "ui.h"

int main(int argc, char *args[])
{
    (void)argc;
    (void)args;

    config_load();
    assets_load();
    program_init();
    input_init();
    game_init();
    ui_init();
    renderer_init();

    TCOD_sys_set_fps(FPS);

    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(console_width, console_height, WINDOW_TITLE, fullscreen, console_renderer);

    while (!TCOD_console_is_window_closed())
    {
        char title[256];
        sprintf(title, "%s - FPS: %d", WINDOW_TITLE, TCOD_sys_get_fps());
        TCOD_console_set_window_title(title);

        input_handle();
        game_update();
        ui_update();
        renderer_draw();

        if (program->should_quit)
        {
            break;
        }
    }

    TCOD_console_delete(NULL);

    renderer_quit();
    ui_quit();
    game_quit();
    input_quit();
    program_quit();
    config_save();

    return 0;
}
