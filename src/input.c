#include <libtcod.h>

#include "input.h"
#include "config.h"
#include "world.h"

input_t input_handle(world_t *world)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    if (ev == TCOD_EVENT_MOUSE_PRESS)
    {
        // TODO: offset by vx and vy
        // this means they will need to be global
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

        return INPUT_UPDATE;
    }

    if (ev == TCOD_EVENT_KEY_PRESS)
    {
        // TODO: find a place for this
        tile_t *tile_n = world->player->y - 1 > 0
                             ? &world->current_map->tiles[world->player->x][world->player->y - 1]
                             : NULL;
        bool walkable_n = tile_n == NULL ? false : tileinfo[tile_n->type].is_walkable;
        tile_t *tile_e = world->player->x + 1 < MAP_WIDTH
                             ? &world->current_map->tiles[world->player->x + 1][world->player->y]
                             : NULL;
        bool walkable_e = tile_e == NULL ? false : tileinfo[tile_e->type].is_walkable;
        tile_t *tile_s = world->player->y + 1 < MAP_HEIGHT
                             ? &world->current_map->tiles[world->player->x][world->player->y + 1]
                             : NULL;
        bool walkable_s = tile_s == NULL ? false : tileinfo[tile_s->type].is_walkable;
        tile_t *tile_w = world->player->x - 1 > 0
                             ? &world->current_map->tiles[world->player->x - 1][world->player->y]
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
                return INPUT_RESTART;

            case ',':
                if (key.shift)
                {
                    tile_t *tile = &world->current_map->tiles[world->player->x][world->player->y];
                    if (tile->type != TILETYPE_STAIR_UP)
                    {
                        return INPUT_NONE;
                    }

                    if (world->current_map_index <= 0)
                    {
                        return INPUT_QUIT;
                    }

                    world->current_map_index--;

                    map_t *new_map = TCOD_list_get(world->maps, world->current_map_index);

                    TCOD_list_remove(world->current_map->actors, world->player);
                    TCOD_list_push(new_map->actors, world->player);

                    world->current_map = new_map;

                    world->player->x = world->current_map->stair_down_x;
                    world->player->y = world->current_map->stair_down_y;

                    return INPUT_UPDATE;
                }

                return INPUT_NONE;

            case '.':
                if (key.shift)
                {
                    tile_t *tile = &world->current_map->tiles[world->player->x][world->player->y];
                    if (tile->type != TILETYPE_STAIR_DOWN)
                    {
                        return INPUT_NONE;
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

                    return INPUT_UPDATE;
                }

                return INPUT_NONE;

            case 's':
                if (key.lctrl)
                {
                    world_save(world);
                }

                return INPUT_NONE;

            case 'l':
                if (key.lctrl)
                {
                    return INPUT_LOAD;
                }

                return INPUT_NONE;
            }

            return INPUT_NONE;

        case TCODK_KP1:
            if (walkable_s || walkable_w)
            {
                actor_move(world->current_map, world->player, world->player->x - 1, world->player->y + 1);
            }

            return INPUT_UPDATE;

        case TCODK_KP2:
        case TCODK_DOWN:
            actor_move(world->current_map, world->player, world->player->x, world->player->y + 1);

            return INPUT_UPDATE;

        case TCODK_KP3:
            if (walkable_e || walkable_s)
            {
                actor_move(world->current_map, world->player, world->player->x + 1, world->player->y + 1);
            }

            return INPUT_UPDATE;

        case TCODK_KP4:
        case TCODK_LEFT:
            actor_move(world->current_map, world->player, world->player->x - 1, world->player->y);

            return INPUT_UPDATE;

        case TCODK_KP5:
            return INPUT_UPDATE;

        case TCODK_KP6:
        case TCODK_RIGHT:
            actor_move(world->current_map, world->player, world->player->x + 1, world->player->y);

            return INPUT_UPDATE;

        case TCODK_KP7:
            if (walkable_n || walkable_w)
            {
                actor_move(world->current_map, world->player, world->player->x - 1, world->player->y - 1);
            }

            return INPUT_UPDATE;

        case TCODK_KP8:
        case TCODK_UP:
            actor_move(world->current_map, world->player, world->player->x, world->player->y - 1);

            return INPUT_UPDATE;

        case TCODK_KP9:
            if (walkable_n || walkable_e)
            {
                actor_move(world->current_map, world->player, world->player->x + 1, world->player->y - 1);
            }

            return INPUT_UPDATE;
        }
    }

    return INPUT_NONE;
}