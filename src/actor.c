#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "game.h"

#define LIT_ROOMS 0

actor_t *actor_create(map_t *map, int x, int y, unsigned char glyph, TCOD_color_t color, int fov_radius)
{
    actor_t *actor = (actor_t *)malloc(sizeof(actor_t));

    actor->map = map;
    actor->x = x;
    actor->y = y;
    actor->glyph = glyph;
    actor->color = color;
    actor->items = TCOD_list_new();
    actor->light = true;
    actor->light_color = TCOD_white;
    actor->torch = false;
    actor->torch_color = TCOD_light_amber;
    actor->fov_radius = fov_radius;
    actor->fov_map = NULL;
    actor->mark_for_delete = false;

    if (TCOD_random_get_int(NULL, 0, 1) == 0)
    {
        actor->torch = true;
        actor->fov_radius *= 2;
    }

    actor_calc_fov(actor);

    return actor;
}

void actor_turn(actor_t *actor)
{
    actor_calc_fov(actor);

    for (void **i = TCOD_list_begin(actor->items); i != TCOD_list_end(actor->items); i++)
    {
        item_t *item = *i;

        item_turn(item);
    }

    if (actor == player)
    {
        return;
    }

    if (TCOD_random_get_int(NULL, 0, 1) == 0)
    {
        for (void **i = TCOD_list_begin(actor->map->actors); i != TCOD_list_end(actor->map->actors); i++)
        {
            actor_t *other = *i;

            if (other == actor)
            {
                continue;
            }

            if (TCOD_map_is_in_fov(actor->fov_map, other->x, other->y))
            {
                msg_log("{name} spots {name}", actor->map, actor->x, actor->y);

                move_actions_t actions = {
                    .light_on = false,
                    .light_off = false,
                    .attack = true,
                    .take_item = false,
                    .take_items = false};

                actor_move(actor, other->x, other->y, actions);
            }
        }
    }
    else
    {
        move_actions_t actions = {
            .light_on = true,
            .light_off = true,
            .attack = true,
            .take_item = true,
            .take_items = true};

        switch (TCOD_random_get_int(NULL, 0, 8))
        {
        case 0:
            actor_move(actor, actor->x, actor->y - 1, actions);

            break;

        case 1:
            actor_move(actor, actor->x, actor->y + 1, actions);

            break;

        case 2:
            actor_move(actor, actor->x - 1, actor->y, actions);

            break;

        case 3:
            actor_move(actor, actor->x + 1, actor->y, actions);

            break;
        }
    }
}

void actor_tick(actor_t *actor)
{
    for (void **i = TCOD_list_begin(actor->items); i != TCOD_list_end(actor->items); i++)
    {
        item_t *item = *i;

        item_tick(item);
    }
}

void actor_calc_fov(actor_t *actor)
{
    if (actor->fov_map != NULL)
    {
        TCOD_map_delete(actor->fov_map);
    }

    actor->fov_map = map_to_TCOD_map(actor->map);
    TCOD_map_compute_fov(actor->fov_map, actor->x, actor->y, actor->fov_radius, true, FOV_DIAMOND);

    TCOD_map_t los_map = map_to_TCOD_map(actor->map);
    TCOD_map_compute_fov(los_map, actor->x, actor->y, 0, true, FOV_DIAMOND);

#if LIT_ROOMS
    for (void **i = TCOD_list_begin(actor->map->rooms); i != TCOD_list_end(actor->map->rooms); i++)
    {
        room_t *room = *i;

        if (!room_is_inside(room, x, y))
        {
            continue;
        }

        for (int x = room->x - 1; x <= room->x + room->w; x++)
        {
            for (int y = room->y - 1; y <= room->y + room->h; y++)
            {
                TCOD_map_set_in_fov(actor->fov_map, x, y, true);
                TCOD_map_set_in_fov(los_map, x, y, true);
            }
        }
    }
#endif

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &actor->map->tiles[x][y];

            for (void **i = TCOD_list_begin(actor->map->lights); i != TCOD_list_end(actor->map->lights); i++)
            {
                light_t *light = *i;

                if (TCOD_map_is_in_fov(light->fov_map, x, y) && TCOD_map_is_in_fov(los_map, x, y))
                {
                    TCOD_map_set_in_fov(actor->fov_map, x, y, true);
                }
            }

            for (void **i = TCOD_list_begin(actor->map->actors); i != TCOD_list_end(actor->map->actors); i++)
            {
                actor_t *other = *i;

                if ((other->torch || other->light) && TCOD_map_is_in_fov(other->fov_map, x, y) && TCOD_map_is_in_fov(los_map, x, y))
                {
                    TCOD_map_set_in_fov(actor->fov_map, x, y, true);
                }
            }

            if (tile->actor != NULL)
            {
                if ((tile->actor->light || tile->actor->torch) && TCOD_map_is_in_fov(los_map, x, y))
                {
                    TCOD_map_set_in_fov(actor->fov_map, x, y, true);
                }
            }
        }
    }

    TCOD_map_delete(los_map);
}

bool actor_move(actor_t *actor, int x, int y, move_actions_t actions)
{
    bool success = false;

    if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT)
    {
        TCOD_map_set_properties(actor->fov_map, x, y, TCOD_map_is_transparent(actor->fov_map, x, y), true);

        TCOD_path_t path = TCOD_path_new_using_map(actor->fov_map, 1.0f);
        TCOD_path_compute(path, actor->x, actor->y, x, y);

        if (!TCOD_path_is_empty(path))
        {
            // TODO: all all this stuff on the tile the actor is on as well
            int next_x, next_y;
            while (TCOD_path_walk(path, &next_x, &next_y, false))
            {
                tile_t *tile = &actor->map->tiles[next_x][next_y];

                if (!tile_walkable[tile->type])
                {
                    break;
                }

                if (tile->light != NULL)
                {
                    if (next_x == x && next_y == y)
                    {
                        if (tile->light->on && actions.light_off)
                        {
                            tile->light->on = false;
                        }
                        else if (!tile->light->on && actions.light_on)
                        {
                            tile->light->on = true;
                        }
                    }
                }

                if (tile->actor != NULL && tile->actor != actor)
                {
                    if (next_x == x && next_y == y && actions.attack && tile->actor != player)
                    {
                        msg_log("{name} hits {name} for {damage}", actor->map, actor->x, actor->y);

                        tile->actor->mark_for_delete = true;

                        success = true;
                    }

                    break;
                }

                if (next_x == x && next_y == y && actions.take_item && TCOD_list_peek(tile->items) != NULL)
                {
                    TCOD_list_push(actor->items, TCOD_list_pop(tile->items));
                }

                if (next_x == x && next_y == y && actions.take_items && TCOD_list_size(tile->items) > 0)
                {
                    for (void **i = TCOD_list_begin(tile->items); i != TCOD_list_end(tile->items); i++)
                    {
                        item_t *item = *i;

                        i = TCOD_list_remove_iterator(tile->items, i);

                        TCOD_list_push(actor->items, item);
                    }
                }

                actor->map->tiles[actor->x][actor->y].actor = NULL;
                actor->map->tiles[next_x][next_y].actor = actor;

                actor->x = next_x;
                actor->y = next_y;

                success = true;

                break;
            }
        }

        TCOD_path_delete(path);
    }

    return success;
}

void actor_draw_turn(actor_t *actor)
{
    if (TCOD_map_is_in_fov(player->fov_map, actor->x, actor->y))
    {
        TCOD_console_set_char_foreground(NULL, actor->x - view_x, actor->y - view_y, actor->color);
        TCOD_console_set_char(NULL, actor->x - view_x, actor->y - view_y, actor->glyph);
    }
}

void actor_draw_tick(actor_t *actor)
{
}

void actor_destroy(actor_t *actor)
{
    for (void **i = TCOD_list_begin(actor->items); i != TCOD_list_end(actor->items); i++)
    {
        item_t *item = *i;

        item_destroy(item);
    }

    TCOD_list_delete(actor->items);

    if (actor->fov_map != NULL)
    {
        TCOD_map_delete(actor->fov_map);
    }

    free(actor);
}