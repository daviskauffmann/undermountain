#include <SDL.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "config.h"
#include "ECS.h"
#include "world.h"
#include "game.h"

int main(int argc, char *argv[])
{
    config_init();

    ECS_init();
    world_init();
    game_init();

    game_new();

    while (!TCOD_console_is_window_closed())
    {
        input_system();
        lighting_system();
        fov_system();

        if (game_status == STATUS_UPDATE)
        {

            ai_system();
            movement_system();

            game_status = STATUS_WAITING;
            turn++;
        }

        render_system();

        if (game_status == STATUS_QUIT)
        {
            break;
        }
    }

    game_reset();
    world_reset();
    ECS_reset();

    SDL_Quit();

    return 0;
}