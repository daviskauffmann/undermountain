#include <stdint.h>
#include <stdio.h>
#include <SDL.h>
#include <libtcod.h>

#include "world.h"
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

    world_init();

    map_draw(current_map);

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
                tile_t *tile = &current_map->tiles[x][y];
                tile->type = TILETYPE_WALL;
            } while (!TCOD_line_step(&x, &y));

            map_draw(current_map);
        }

        if (ev == TCOD_EVENT_KEY_PRESS)
        {
            switch (key.vk)
            {
            case TCODK_ESCAPE:
                goto quit;

            case TCODK_CHAR:
                switch (key.c)
                {
                case ',':
                    if (key.shift)
                    {
                        if (current_map_index <= 0)
                        {
                            // TODO: exit game?
                            break;
                        }

                        current_map_index--;

                        map_t *new_map = TCOD_list_get(maps, current_map_index);

                        TCOD_list_remove(current_map->actors, player);
                        TCOD_list_push(new_map->actors, player);

                        current_map = new_map;

                        player->x = current_map->stair_down_x;
                        player->y = current_map->stair_down_y;

                        map_update(current_map);
                        map_draw(current_map);
                    }

                    break;

                case '.':
                    if (key.shift)
                    {
                        // TODO: max maps?
                        current_map_index++;

                        if (TCOD_list_size(maps) == current_map_index)
                        {
                            TCOD_list_push(maps, map_create());
                        }

                        map_t *new_map = TCOD_list_get(maps, current_map_index);

                        TCOD_list_remove(current_map->actors, player);
                        TCOD_list_push(new_map->actors, player);

                        current_map = new_map;

                        player->x = current_map->stair_up_x;
                        player->y = current_map->stair_up_y;

                        map_update(current_map);
                        map_draw(current_map);
                    }

                    break;

                case 's':
                    if (key.lctrl)
                    {
                        world_save();
                    }

                    break;

                case 'l':
                    if (key.lctrl)
                    {
                        world_load();
                    }

                    break;
                }

                break;

            // case TCODK_KP1:
            //     actor_move(current_map, player, player->x - 1, player->y + 1);

            //     map_update(current_map);
            //     map_draw(current_map);

            //     break;
            // case TCODK_KP3:
            //     actor_move(current_map, player, player->x + 1, player->y + 1);

            //     map_update(current_map);
            //     map_draw(current_map);

            //     break;
            // case TCODK_KP7:
            //     actor_move(current_map, player, player->x - 1, player->y - 1);

            //     map_update(current_map);
            //     map_draw(current_map);

            //     break;
            // case TCODK_KP9:
            //     actor_move(current_map, player, player->x + 1, player->y - 1);

            //     map_update(current_map);
            //     map_draw(current_map);

            //     break;

            case TCODK_KP8:
            case TCODK_UP:
                actor_move(current_map, player, player->x, player->y - 1);

                map_update(current_map);
                map_draw(current_map);

                break;

            case TCODK_KP2:
            case TCODK_DOWN:
                actor_move(current_map, player, player->x, player->y + 1);

                map_update(current_map);
                map_draw(current_map);

                break;

            case TCODK_KP4:
            case TCODK_LEFT:
                actor_move(current_map, player, player->x - 1, player->y);

                map_update(current_map);
                map_draw(current_map);

                break;

            case TCODK_KP6:
            case TCODK_RIGHT:
                actor_move(current_map, player, player->x + 1, player->y);

                map_update(current_map);
                map_draw(current_map);

                break;
            }
        }
    }
quit:

    world_destroy();

    TCOD_console_delete(NULL);

    SDL_Quit();

    return 0;
}