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

    Map maps[1];

    Map *map = &maps[0];
    map_init(map);
    map_generate(map);

    Entity *player = &map->entities[MAX_ENTITIES - 1];
    entity_init(player, 1, 1, '@', TCOD_white);

    Entity *npc = &map->entities[0];
    entity_init(npc, 5, 5, '@', TCOD_yellow);

    map_draw(map);

    while (!TCOD_console_is_window_closed())
    {
        bool quit = false;

        switch (input_handle(map, player))
        {
        case INPUT_QUIT:
            quit = true;
            break;
        case INPUT_TRUE:
            map_draw(map);
            break;
        }

        if (quit)
        {
            break;
        }
    }

    SDL_Quit();

    return 0;
}