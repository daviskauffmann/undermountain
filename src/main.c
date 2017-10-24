#include <stdint.h>
#include <stdio.h>
#include <SDL.h>
#include <libtcod.h>

#include "config.h"
#include "world.h"
#include "input.h"
#include "save.h"

int main(int argc, char *argv[])
{
    config_init();

    world_t *world = world_create();

    map_draw(world->current_map, world->player);

    while (!TCOD_console_is_window_closed())
    {
        input_t input = input_handle(world);
        switch (input)
        {
        case INPUT_NONE:
            break;

        case INPUT_UPDATE:
            map_update(world->current_map, world->player);
            map_draw(world->current_map, world->player);

            break;

        case INPUT_RESTART:
            world_destroy(world);
            world = world_create();

            map_draw(world->current_map, world->player);

            break;

        case INPUT_LOAD:
            world_destroy(world);
            world = world_load();

            map_draw(world->current_map, world->player);

            break;

        case INPUT_QUIT:
            goto quit;
        }
    }
quit:

    world_destroy(world);

    SDL_Quit();

    return 0;
}