#include <stdint.h>
#include <stdio.h>
#include <SDL.h>
#include <libtcod.h>

#include "config.h"
#include "world.h"
#include "save.h"

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 50
#define WINDOW_TITLE "Roguelike v0.1"

int main(int argc, char *argv[])
{
    TCOD_console_init_root(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE, false, TCOD_RENDERER_SDL);
    TCOD_console_set_default_background(NULL, TCOD_black);
    TCOD_console_set_default_foreground(NULL, TCOD_white);

    config_init();

    world_t *world = world_create();

    map_draw(world->current_map, world->player);

    while (!TCOD_console_is_window_closed())
    {
        TCOD_key_t key;
        TCOD_mouse_t mouse;
        TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

        if (ev == TCOD_EVENT_MOUSE_PRESS)
        {
            int x = world->player->x;
            int y = world->player->y;
            int xTo = mouse.cx;
            int yTo = mouse.cy;
            TCOD_line_init(x, y, xTo, yTo);
            do
            {
                tile_t *tile = &world->current_map->tiles[x][y];
                tile->type = TILETYPE_WALL;
            } while (!TCOD_line_step(&x, &y));

            map_draw(world->current_map, world->player);
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
                case 'r':
                    world_destroy(world);
                    world = world_create();

                    map_draw(world->current_map, world->player);

                    break;

                case ',':
                    if (key.shift)
                    {
                        tile_t *tile = &world->current_map->tiles[world->player->x][world->player->y];
                        if (tile->type != TILETYPE_STAIR_UP)
                        {
                            break;
                        }

                        if (world->current_map_index <= 0)
                        {
                            // TODO: exit game?
                            break;
                        }

                        world->current_map_index--;

                        map_t *new_map = TCOD_list_get(world->maps, world->current_map_index);

                        TCOD_list_remove(world->current_map->actors, world->player);
                        TCOD_list_push(new_map->actors, world->player);

                        world->current_map = new_map;

                        world->player->x = world->current_map->stair_down_x;
                        world->player->y = world->current_map->stair_down_y;

                        map_update(world->current_map, world->player);
                        map_draw(world->current_map, world->player);
                    }

                    break;

                case '.':
                    if (key.shift)
                    {
                        tile_t *tile = &world->current_map->tiles[world->player->x][world->player->y];
                        if (tile->type != TILETYPE_STAIR_DOWN)
                        {
                            break;
                        }

                        world->current_map_index++;

                        map_t *new_map = TCOD_list_size(world->maps) == world->current_map_index
                                             ? map_create(world)
                                             : TCOD_list_get(world->maps, world->current_map_index);

                        TCOD_list_remove(world->current_map->actors, world->player);
                        TCOD_list_push(new_map->actors, world->player);

                        world->current_map = new_map;

                        world->player->x = world->current_map->stair_up_x;
                        world->player->y = world->current_map->stair_up_y;

                        map_update(world->current_map, world->player);
                        map_draw(world->current_map, world->player);
                    }

                    break;

                case 's':
                    if (key.lctrl)
                    {
                        world_save(world);
                    }

                    break;

                case 'l':
                    if (key.lctrl)
                    {
                        world_destroy(world);
                        world = world_load();

                        map_draw(world->current_map, world->player);
                    }

                    break;
                }

                break;

            // case TCODK_KP1:
            //     actor_move(world->current_map, world->player, world->player->x - 1, world->player->y + 1);

            //     map_update(world->current_map, world->player);
            //     map_draw(world->current_map, world->player);

            //     break;
            // case TCODK_KP3:
            //     actor_move(world->current_map, world->player, world->player->x + 1, world->player->y + 1);

            //     map_update(world->current_map, world->player);
            //     map_draw(world->current_map, world->player);

            //     break;
            // case TCODK_KP7:
            //     actor_move(world->current_map, world->player, world->player->x - 1, world->player->y - 1);

            //     map_update(world->current_map, world->player);
            //     map_draw(world->current_map, world->player);

            //     break;
            // case TCODK_KP9:
            //     actor_move(world->current_map, world->player, world->player->x + 1, world->player->y - 1);

            //     map_update(world->current_map, world->player);
            //     map_draw(world->current_map, world->player);

            //     break;

            case TCODK_KP8:
            case TCODK_UP:
                actor_move(world->current_map, world->player, world->player->x, world->player->y - 1);

                map_update(world->current_map, world->player);
                map_draw(world->current_map, world->player);

                break;

            case TCODK_KP2:
            case TCODK_DOWN:
                actor_move(world->current_map, world->player, world->player->x, world->player->y + 1);

                map_update(world->current_map, world->player);
                map_draw(world->current_map, world->player);

                break;

            case TCODK_KP4:
            case TCODK_LEFT:
                actor_move(world->current_map, world->player, world->player->x - 1, world->player->y);

                map_update(world->current_map, world->player);
                map_draw(world->current_map, world->player);

                break;

            case TCODK_KP6:
            case TCODK_RIGHT:
                actor_move(world->current_map, world->player, world->player->x + 1, world->player->y);

                map_update(world->current_map, world->player);
                map_draw(world->current_map, world->player);

                break;
            }
        }
    }
quit:

    world_destroy(world);

    SDL_Quit();

    return 0;
}