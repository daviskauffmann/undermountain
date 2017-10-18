#include <libtcod.h>
#include <SDL.h>
#include <stdio.h>

#include "entity.h"
#include "input.h"
#include "map.h"

#define WIDTH 80
#define HEIGHT 50
#define TITLE "Roguelike"

int main(int argc, char *argv[])
{
    time_t t;
    srand((unsigned)time(&t));

    TCOD_console_init_root(WIDTH, HEIGHT, TITLE, false, TCOD_RENDERER_SDL);

    Map *map = malloc(sizeof(Map));
    map_init(map);
    map_generate(map);

    Entity *player = &map->entities[0];
    entity_init(player, ID_PLAYER, 1, 1, '@', TCOD_white);

    Entity *npc = map_createEntity(map, 5, 5, '@', TCOD_yellow);

    map_draw(map);

    while (!TCOD_console_is_window_closed())
    {
        InputType input = input_handle(player);
        if (input == INPUT_TRUE)
        {
            map_update(map);
            map_draw(map);
        }
        else if (input == INPUT_QUIT)
        {
            break;
        }
    }

    SDL_Quit();

    return 0;
}