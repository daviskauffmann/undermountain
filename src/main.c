#include "game.h"
#include "input.h"
#include "renderer.h"
#include "ui.h"
#include "window.h"

#include "CMemleak.h"

int main(void)
{
    window_init();

    struct input *input = input_create();
    struct game *game = game_create();
    struct ui *ui = ui_create();
    struct renderer *renderer = renderer_create();

    while (!window_is_closed())
    {
        input_handle(input, game, ui);
        game_update(game);
        ui_update(ui, game);
        renderer_draw(renderer, game, input, ui);

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

    window_quit();

    return 0;
}
