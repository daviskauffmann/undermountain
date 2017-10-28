#include <libtcod.h>

#include "input.h"
#include "config.h"
#include "game.h"
#include "world.h"
#include "view.h"

input_t input_handle(void)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_MOUSE_PRESS:
        player->path = map_calc_path(map_to_TCOD_map(current_map), player->x, player->y, mouse.cx + view_left, mouse.cy + view_top);

        break;

    case TCOD_EVENT_KEY_PRESS:
        player->path = NULL;

        // TODO: find a place for this
        tile_t *tile_n = player->y - 1 > 0
                             ? &current_map->tiles[player->x][player->y - 1]
                             : NULL;
        bool walkable_n = tile_n == NULL ? false : tileinfo[tile_n->type].is_walkable;
        tile_t *tile_e = player->x + 1 < MAP_WIDTH
                             ? &current_map->tiles[player->x + 1][player->y]
                             : NULL;
        bool walkable_e = tile_e == NULL ? false : tileinfo[tile_e->type].is_walkable;
        tile_t *tile_s = player->y + 1 < MAP_HEIGHT
                             ? &current_map->tiles[player->x][player->y + 1]
                             : NULL;
        bool walkable_s = tile_s == NULL ? false : tileinfo[tile_s->type].is_walkable;
        tile_t *tile_w = player->x - 1 > 0
                             ? &current_map->tiles[player->x - 1][player->y]
                             : NULL;
        bool walkable_w = tile_w == NULL ? false : tileinfo[tile_w->type].is_walkable;

        switch (key.vk)
        {
        case TCODK_ESCAPE:
            return INPUT_QUIT;

        case TCODK_CHAR:
            switch (key.c)
            {
            case 'r':
                world_destroy();
                world_init();
                game_init();

                return INPUT_REDRAW;

            case ',':
                if (key.shift)
                {
                    tile_t *tile = &current_map->tiles[player->x][player->y];
                    if (tile->type != TILETYPE_STAIR_UP)
                    {
                        break;
                    }

                    if (current_map_index <= 0)
                    {
                        return INPUT_QUIT;
                    }

                    current_map_index--;

                    map_t *new_map = TCOD_list_get(maps, current_map_index);

                    TCOD_list_remove(current_map->actors, player);
                    TCOD_list_push(new_map->actors, player);

                    player->x = new_map->stair_down_x;
                    player->y = new_map->stair_down_y;

                    current_map = new_map;

                    return INPUT_UPDATE;
                }

                break;

            case '.':
                if (key.shift)
                {
                    tile_t *tile = &current_map->tiles[player->x][player->y];
                    if (tile->type != TILETYPE_STAIR_DOWN)
                    {
                        break;
                    }

                    current_map_index++;

                    map_t *new_map = TCOD_list_size(maps) == current_map_index
                                         ? map_create()
                                         : TCOD_list_get(maps, current_map_index);

                    TCOD_list_remove(current_map->actors, player);
                    TCOD_list_push(new_map->actors, player);

                    player->x = new_map->stair_up_x;
                    player->y = new_map->stair_up_y;

                    current_map = new_map;

                    return INPUT_UPDATE;
                }

                break;

            case 's':
                if (key.lctrl)
                {
                    game_save();
                }

                break;

            case 'l':
                if (key.lctrl)
                {
                    world_destroy();
                    game_load();

                    return INPUT_REDRAW;
                }

                break;
            }

            break;

        case TCODK_KP1:
            if (walkable_s || walkable_w)
            {
                actor_move(current_map, player, player->x - 1, player->y + 1);
            }

            return INPUT_UPDATE;

        case TCODK_KP2:
        case TCODK_DOWN:
            actor_move(current_map, player, player->x, player->y + 1);

            return INPUT_UPDATE;

        case TCODK_KP3:
            if (walkable_e || walkable_s)
            {
                actor_move(current_map, player, player->x + 1, player->y + 1);
            }

            return INPUT_UPDATE;

        case TCODK_KP4:
        case TCODK_LEFT:
            actor_move(current_map, player, player->x - 1, player->y);

            return INPUT_UPDATE;

        case TCODK_KP5:
            return INPUT_UPDATE;

        case TCODK_KP6:
        case TCODK_RIGHT:
            actor_move(current_map, player, player->x + 1, player->y);

            return INPUT_UPDATE;

        case TCODK_KP7:
            if (walkable_n || walkable_w)
            {
                actor_move(current_map, player, player->x - 1, player->y - 1);
            }

            return INPUT_UPDATE;

        case TCODK_KP8:
        case TCODK_UP:
            actor_move(current_map, player, player->x, player->y - 1);

            return INPUT_UPDATE;

        case TCODK_KP9:
            if (walkable_n || walkable_e)
            {
                actor_move(current_map, player, player->x + 1, player->y - 1);
            }

            return INPUT_UPDATE;
        }

        break;
    }

    if (!TCOD_path_is_empty(player->path))
    {
        int x, y;
        if (TCOD_path_walk(player->path, &x, &y, false))
        {
            actor_move(current_map, player, x, y);

            return INPUT_UPDATE;
        }
    }

    player->path = NULL;

    return INPUT_NONE;
}