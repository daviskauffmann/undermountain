#include <stdio.h>
#include <SDL.h>
#include <time.h>
#include <libtcod.h>

#include "config.h"
#include "console.h"
#include "world.h"
#include "game.h"
#include "input.h"

int main(int argc, char *argv[])
{
    config_initialize();

    // TODO: main menu
    while (!TCOD_console_is_window_closed())
    {
        TCOD_key_t key;
        TCOD_mouse_t mouse;
        TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

        if (key.vk == TCODK_SPACE)
        {
            break;
        }
    }

    tiles_initialize();

    console_initialize();
    world_initialize();
    game_initialize();

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
        console_turn_draw();

    tick:
        world_tick();
        console_tick_draw();
    }

quit:
    SDL_Quit();

    return 0;
}