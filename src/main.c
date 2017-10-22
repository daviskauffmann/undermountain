#include <libtcod.h>
#include <SDL.h>
#include <stdio.h>

#include "map.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 50
#define WINDOW_TITLE "Roguelike"

int main(int argc, char *argv[])
{
    TCOD_console_init_root(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE, false, TCOD_RENDERER_SDL);
    TCOD_console_set_default_background(NULL, TCOD_black);
    TCOD_console_set_default_foreground(NULL, TCOD_white);

    TCOD_random_t random = TCOD_random_get_instance();

    tileinfo_init();

    map_t *map = map_create();

    actor_t *player = actor_create(map, true, MAP_WIDTH / 2, MAP_HEIGHT / 2, '@', TCOD_white, 10);

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
                actor_move(map, player, player->x, player->y - 1);
                map_update(map);
                map_draw(map, player);
                break;
            case TCODK_DOWN:
                actor_move(map, player, player->x, player->y + 1);
                map_update(map);
                map_draw(map, player);
                break;
            case TCODK_LEFT:
                actor_move(map, player, player->x - 1, player->y);
                map_update(map);
                map_draw(map, player);
                break;

            case TCODK_RIGHT:
                actor_move(map, player, player->x + 1, player->y);
                map_update(map);
                map_draw(map, player);
                break;
            }
        }
    }
quit:

    map_destroy(map);

    TCOD_console_delete(NULL);

    SDL_Quit();

    return 0;
}