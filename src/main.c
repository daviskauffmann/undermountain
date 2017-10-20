#include <libtcod.h>
#include <SDL.h>

#include "tile.h"
#include "entity.h"
#include "map.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 50
#define WINDOW_TITLE "Roguelike"

int main(int argc, char *argv[])
{
    time_t t;
    srand((unsigned)time(&t));

    TCOD_console_init_root(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE, false, TCOD_RENDERER_SDL);

    tileinfo_init();

    map_t *map = malloc(sizeof(map_t));
    map_init(map);
    map_generate(map);

    map_draw(map);

    entity_t *player = &map->entities[ENTITY_ID_PLAYER];

    while (!TCOD_console_is_window_closed())
    {
        TCOD_key_t key;
        TCOD_mouse_t mouse;
        TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

        if (ev == TCOD_EVENT_KEY_PRESS)
        {
            switch (key.vk)
            {
            case TCODK_ESCAPE:
                goto quit;
            case TCODK_UP:
                map_entity_move(map, player, 0, -1);
                map_update(map);
                map_draw(map);
                break;
            case TCODK_LEFT:
                map_entity_move(map, player, -1, 0);
                map_update(map);
                map_draw(map);
                break;
            case TCODK_DOWN:
                map_entity_move(map, player, 0, 1);
                map_update(map);
                map_draw(map);
                break;
            case TCODK_RIGHT:
                map_entity_move(map, player, 1, 0);
                map_update(map);
                map_draw(map);
                break;
            }
        }
    }
quit:

    SDL_Quit();

    return 0;
}