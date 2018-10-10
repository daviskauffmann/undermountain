#include "config.h"
#include "engine.h"
#include "game.h"
#include "input.h"
#include "renderer.h"
#include "ui.h"

#include "CMemleak.h"

enum engine_state engine_state;

int engine_run(void)
{
    engine_state = ENGINE_STATE_MENU;

    config_load();

    TCOD_sys_set_fps(FPS);

    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(console_width, console_height, WINDOW_TITLE, fullscreen, console_renderer);

    struct input *input = input_create();
    struct game *game = game_create();
    struct ui *ui = ui_create();
    struct renderer *renderer = renderer_create();

    while (!TCOD_console_is_window_closed())
    {
        input_handle(input, game, ui);
        game_update(game);
        ui_update(ui, game);
        renderer_draw(renderer, game, ui);

        if (game->should_restart)
        {
            game_destroy(game);
            game = game_create();
        }

        if (game->should_quit)
        {
            break;
        }
    }

    renderer_destroy(renderer);
    ui_destroy(ui);
    game_destroy(game);
    input_destroy(input);

    config_save();

    return 0;
}
