#include <libtcod.h>
#include <SDL.h>

#include "map.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 50
#define WINDOW_TITLE "Roguelike"

int main(int argc, char *argv[])
{
    // TODO: use TCOD RNG
    time_t t;
    srand((unsigned)time(&t));

    TCOD_console_init_root(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE, false, TCOD_RENDERER_SDL);
    TCOD_console_set_default_background(NULL, TCOD_black);
    TCOD_console_set_default_foreground(NULL, TCOD_white);

    tileinfo_init();

    map_t *map = malloc(sizeof(map_t));
    map_init(map);
    map_generate(map);

    entity_t *player = map_entity_create(map, true, MAP_WIDTH / 2, MAP_HEIGHT / 2, '@', TCOD_white);

    map_draw(map, player);

    while (!TCOD_console_is_window_closed())
    {
        TCOD_key_t key;
        TCOD_mouse_t mouse;
        TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

        if (ev == TCOD_EVENT_MOUSE_PRESS)
        {
            int x = player->x;
            int y = player->y;
            int xTo = mouse.cx;
            int yTo = mouse.cy;
            TCOD_line_init(x, y, xTo, yTo);
            do
            {
                tile_t *tile = &map->tiles[x][y];
                tile->type = TILETYPE_WALL;
            } while (!TCOD_line_step(&x, &y));

            map_draw(map, player);
        }

        if (ev == TCOD_EVENT_KEY_PRESS)
        {
            switch (key.vk)
            {
            case TCODK_ESCAPE:
                goto quit;
            case TCODK_UP:
                map_entity_move(map, player, 0, -1);
                map_update(map);
                map_draw(map, player);
                break;
            case TCODK_LEFT:
                map_entity_move(map, player, -1, 0);
                map_update(map);
                map_draw(map, player);
                break;
            case TCODK_DOWN:
                map_entity_move(map, player, 0, 1);
                map_update(map);
                map_draw(map, player);
                break;
            case TCODK_RIGHT:
                map_entity_move(map, player, 1, 0);
                map_update(map);
                map_draw(map, player);
                break;
            }
        }
    }
quit:

    SDL_Quit();

    return 0;
}