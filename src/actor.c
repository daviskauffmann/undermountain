#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "game.h"

#define LIT_ROOMS 0

actor_t *actor_create(map_t *map, int x, int y, unsigned char glyph, TCOD_color_t color)
{
    actor_t *actor = (actor_t *)malloc(sizeof(actor_t));

    actor->map = map;
    actor->x = x;
    actor->y = y;
    actor->glyph = glyph;
    actor->color = color;
    actor->items = TCOD_list_new();
    actor->light = false;
    actor->light_color = TCOD_white;
    actor->torch = false;
    actor->torch_color = TCOD_light_amber;
    actor->fov_radius = 1;
    actor->fov_map = NULL;
    actor->mark_for_delete = false;
    actor->target = false;
    actor->speed = 2;
    actor->turns_waited = 0;

    switch (TCOD_random_get_int(NULL, 0, 2))
    {
    case 0:
    {
        actor->light = false;
        actor->torch = false;
        actor->fov_radius = 1;

        break;
    }
    case 1:
    {
        actor->light = true;
        actor->torch = false;
        actor->fov_radius = 5;

        break;
    }
    case 2:
    {
        actor->light = false;
        actor->torch = true;
        actor->fov_radius = 10;

        break;
    }
    }

    actor_calc_fov(actor);

    return actor;
}

void actor_turn(actor_t *actor)
{
    actor_calc_fov(actor);

    if (actor != player)
    {
        actor->turns_waited++;
        if (actor->turns_waited == actor->speed)
        {
            actor->turns_waited = 0;
        }

        if (actor->turns_waited == 0)
        {
            bool chasing = false;

            for (void **i = TCOD_list_begin(actor->map->actors); i != TCOD_list_end(actor->map->actors); i++)
            {
                actor_t *other = *i;

                if (other != actor)
                {
                    if (TCOD_map_is_in_fov(actor->fov_map, other->x, other->y))
                    {
                        msg_log("{name} spots {name}", actor->map, actor->x, actor->y);

                        interactions_t interactions = {
                            .light_on = false,
                            .light_off = false,
                            .attack = true,
                            .take_item = false,
                            .take_items = false};

                        actor_target_set(actor, other->x, other->y, interactions);

                        chasing = true;

                        break;
                    }
                }
            }

            if (!chasing)
            {
                interactions_t interactions = {
                    .light_on = true,
                    .light_off = true,
                    .attack = true,
                    .take_item = true,
                    .take_items = true};

                switch (TCOD_random_get_int(NULL, 0, 8))
                {
                case 0:
                    actor_move(actor, actor->x, actor->y - 1, interactions);

                    break;

                case 1:
                    actor_move(actor, actor->x, actor->y + 1, interactions);

                    break;

                case 2:
                    actor_move(actor, actor->x - 1, actor->y, interactions);

                    break;

                case 3:
                    actor_move(actor, actor->x + 1, actor->y, interactions);

                    break;
                }
            }

            actor_target_process(actor);
        }
    }
}

void actor_tick(actor_t *actor)
{
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
            if (TCOD_map_is_in_fov(los_map, x, y))
            {
                tile_t *tile = &actor->map->tiles[x][y];

                for (void **i = TCOD_list_begin(actor->map->lights); i != TCOD_list_end(actor->map->lights); i++)
                {
                    light_t *light = *i;

                    if (light->on && TCOD_map_is_in_fov(light->fov_map, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov_map, x, y, true);
                    }
                }

                for (void **i = TCOD_list_begin(actor->map->actors); i != TCOD_list_end(actor->map->actors); i++)
                {
                    actor_t *other = *i;

                    if ((other->light || other->torch) && TCOD_map_is_in_fov(other->fov_map, x, y))
                    {
                        TCOD_map_set_in_fov(actor->fov_map, x, y, true);
                    }
                }
            }
        }
    }

    TCOD_map_delete(los_map);
}

void actor_target_set(actor_t *actor, int x, int y, interactions_t interactions)
{
    tile_t *tile = &actor->map->tiles[x][y];
    actor_t *other = tile->actor;

    actor->target = true;
    actor->target_data.x = x;
    actor->target_data.y = y;
    actor->target_data.actor = other;
    actor->target_data.interactions = interactions;
}

void actor_target_process(actor_t *actor)
{
    if (actor->target)
    {
        int x = actor->target_data.actor == NULL
                    ? actor->target_data.x
                    : actor->target_data.actor->x;
        int y = actor->target_data.actor == NULL
                    ? actor->target_data.y
                    : actor->target_data.actor->y;

        bool arrived = actor_move(actor, x, y, actor->target_data.interactions);

        if (actor->target_data.actor != NULL && actor->target_data.actor->mark_for_delete)
        {
            actor->target = false;
        }

        if (arrived)
        {
            actor->target = false;
        }
    }
}

bool actor_move(actor_t *actor, int x, int y, interactions_t interactions)
{
    bool arrived = false;

    if (map_is_inside(x, y))
    {
        TCOD_map_set_properties(actor->fov_map, x, y, TCOD_map_is_transparent(actor->fov_map, x, y), true);

        TCOD_path_t path = TCOD_path_new_using_map(actor->fov_map, 1.0f);
        TCOD_path_compute(path, actor->x, actor->y, x, y);

        int next_x = actor->x;
        int next_y = actor->y;
        bool valid_path = TCOD_path_walk(path, &next_x, &next_y, false);

        if (!valid_path)
        {
            arrived = true;
        }

        bool interacted = false;

        if (next_x == x && next_y == y)
        {
            interacted = actor_interact(actor, x, y, interactions);

            arrived = true;
        }

        if (!interacted)
        {
            actor->map->tiles[actor->x][actor->y].actor = NULL;
            actor->map->tiles[next_x][next_y].actor = actor;

            actor->x = next_x;
            actor->y = next_y;

            for (void **i = TCOD_list_begin(actor->items); i != TCOD_list_end(actor->items); i++)
            {
                item_t *item = *i;

                item->x = actor->x;
                item->y = actor->y;
            }
        }

        TCOD_path_delete(path);
    }

    return arrived;
}

bool actor_interact(actor_t *actor, int x, int y, interactions_t interactions)
{
    tile_t *tile = &actor->map->tiles[x][y];

    if (!tile_walkable[tile->type])
    {
        return true;
    }

    if (tile->actor != NULL)
    {
        if (interactions.attack)
        {
            if (tile->actor == player)
            {
                msg_log("{name} cannot kill player", actor->map, actor->x, actor->y);
            }
            else
            {
                tile->actor->mark_for_delete = true;
            }
        }

        return true;
    }

    if (interactions.take_items && TCOD_list_size(tile->items) > 0)
    {
        for (void **i = TCOD_list_begin(tile->items); i != TCOD_list_end(tile->items); i++)
        {
            item_t *item = *i;

            item->x = actor->x;
            item->y = actor->y;

            TCOD_list_push(actor->items, item);

            i = TCOD_list_remove_iterator(tile->items, i);
        }

        return true;
    }

    if (interactions.take_item && TCOD_list_peek(tile->items) != NULL)
    {
        item_t *item = TCOD_list_pop(tile->items);

        item->x = actor->x;
        item->y = actor->y;

        TCOD_list_push(actor->items, item);

        return true;
    }

    if (tile->light != NULL)
    {
        if (interactions.light_off && tile->light->on)
        {
            tile->light->on = false;

            return true;
        }
        else if (interactions.light_on && !tile->light->on)
        {
            tile->light->on = true;

            return true;
        }
    }

    return false;
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
    TCOD_list_delete(actor->items);

    if (actor->fov_map != NULL)
    {
        TCOD_map_delete(actor->fov_map);
    }

    free(actor);
}