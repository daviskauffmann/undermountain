#include <libtcod/libtcod.h>
#include <stdio.h>

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

    struct program *program = program_create();
    struct input *input = input_create();
    struct game *game = game_create();
    struct ui *ui = ui_create();
    struct renderer *renderer = renderer_create();

    TCOD_sys_set_fps(FPS);

    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(console_width, console_height, WINDOW_TITLE, fullscreen, console_renderer);

    while (!TCOD_console_is_window_closed())
    {
        char title[256];
        sprintf(title, "%s - FPS: %d", WINDOW_TITLE, TCOD_sys_get_fps());
        TCOD_console_set_window_title(title);

        input_handle(input, program, game, ui);
        game_update(game, program);
        ui_update(ui, program, game);
        renderer_draw(renderer, program, game, ui);

        if (game->should_restart)
        {
            game_destroy(game);
            game = game_create();
        }

        if (ui->should_restart)
        {
            ui_destroy(ui);
            ui = ui_create();
        }

        if (program->should_quit)
        {
            break;
        }
    }

    TCOD_console_delete(NULL);

    renderer_destroy(renderer);
    ui_destroy(ui);
    game_destroy(game);
    input_destroy(input);
    program_destroy(program);

    config_save();

    return 0;
}
