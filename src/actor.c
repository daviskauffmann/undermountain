#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "game.h"

#define LIT_ROOMS 0

actor_t *actor_create(map_t *map, int x, int y, char *name, unsigned char glyph, TCOD_color_t color, void (*ai)(actor_t *actor))
{
    actor_t *actor = (actor_t *)malloc(sizeof(actor_t));

    actor->map = map;
    actor->x = x;
    actor->y = y;
    actor->name = name;
    actor->glyph = glyph;
    actor->color = color;
    actor->items = TCOD_list_new();
    actor->spells = TCOD_list_new();
    actor->spell_ready = NULL;
    actor->light = ACTOR_LIGHT_DEFAULT;
    actor->fov_map = NULL;
    actor->mark_for_delete = false;
    actor->target = false;
    actor->energy = 0;
    actor->energy_per_turn = TCOD_random_get_int(NULL, 5, 15);
    actor->energy_to_act = 20;
    actor->ai = ai;

    actor_calc_fov(actor);

    return actor;
}

void actor_turn(actor_t *actor)
{
    actor_calc_fov(actor);

    if (actor->ai != NULL)
    {
        actor->energy += actor->energy_per_turn;

        while (actor->energy >= actor->energy_to_act)
        {
            actor->energy -= actor->energy_to_act;

            actor->ai(actor);
        }
    }

    tile_t *tile = &actor->map->tiles[actor->x][actor->y];

    if (TCOD_list_peek(tile->items) != NULL)
    {
        // // msg_log(actor->map, actor->x, actor->y, "actor sees items");
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
    TCOD_map_compute_fov(actor->fov_map, actor->x, actor->y, actor_light_radius[actor->light], true, FOV_DIAMOND);

    TCOD_map_t los_map = map_to_TCOD_map(actor->map);
    TCOD_map_compute_fov(los_map, actor->x, actor->y, 0, true, FOV_DIAMOND);

#if LIT_ROOMS
    for (void **i = TCOD_list_begin(actor->map->rooms); i != TCOD_list_end(actor->map->rooms); i++)
    {
        room_t *room = *i;

        if (room_is_inside(room, x, y))
        {
            for (int x = room->x - 1; x <= room->x + room->w; x++)
            {
                for (int y = room->y - 1; y <= room->y + room->h; y++)
                {
                    TCOD_map_set_in_fov(actor->fov_map, x, y, true);
                    TCOD_map_set_in_fov(los_map, x, y, true);
                }
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

                    if ((other->light == ACTOR_LIGHT_DEFAULT || other->light == ACTOR_LIGHT_TORCH) && TCOD_map_is_in_fov(other->fov_map, x, y))
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
        interactions_t interactions = actor->target_data.interactions;

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
                actor->target = false;
            }

            bool move_to_next = true;

            if (next_x == x && next_y == y)
            {
                actor->target = false;

                tile_t *tile = &actor->map->tiles[x][y];

                if (!tile_walkable[tile->type])
                {
                    move_to_next = false;
                }

                if (tile->actor != NULL && tile->actor != actor)
                {
                    move_to_next = false;

                    if (interactions.attack && tile->actor != player)
                    {
                        msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s attacks %s", actor->name, tile->actor->name);

                        tile->actor->mark_for_delete = true;
                    }
                }

                if (interactions.take_items && TCOD_list_size(tile->items) > 0)
                {
                    msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s takes items", actor->name);

                    move_to_next = false;

                    for (void **i = TCOD_list_begin(tile->items); i != TCOD_list_end(tile->items); i++)
                    {
                        item_t *item = *i;

                        item->x = actor->x;
                        item->y = actor->y;

                        TCOD_list_push(actor->items, item);

                        i = TCOD_list_remove_iterator(tile->items, i);
                    }
                }

                if (interactions.take_item && TCOD_list_peek(tile->items) != NULL)
                {
                    item_t *item = TCOD_list_pop(tile->items);

                    msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s takes %s", actor->name, item->name);

                    move_to_next = false;

                    item->x = actor->x;
                    item->y = actor->y;

                    TCOD_list_push(actor->items, item);
                }

                if (tile->light != NULL)
                {
                    if (interactions.light_off && tile->light->on)
                    {
                        // msg_log("actor turns light off", actor->map, actor->x, actor->y);

                        move_to_next = false;

                        tile->light->on = false;
                    }
                    else if (interactions.light_on && !tile->light->on)
                    {
                        // msg_log("actor turns light on", actor->map, actor->x, actor->y);

                        move_to_next = false;

                        tile->light->on = true;
                    }
                }

                if (interactions.descend && tile->type == TILE_TYPE_STAIR_DOWN)
                {
                    // msg_log("actor descends", actor->map, actor->x, actor->y);

                    move_to_next = false;

                    map_t *new_map;

                    if (TCOD_list_size(maps) == actor->map->level + 1)
                    {
                        new_map = map_create(actor->map->level + 1);

                        TCOD_list_push(maps, new_map);
                    }
                    else
                    {
                        new_map = TCOD_list_get(maps, actor->map->level + 1);
                    }

                    TCOD_list_remove(actor->map->actors, actor);
                    TCOD_list_push(new_map->actors, actor);

                    actor->map->tiles[actor->x][actor->y].actor = NULL;
                    new_map->tiles[new_map->stair_up_x][new_map->stair_up_y].actor = actor;

                    actor->map = new_map;
                    actor->x = new_map->stair_up_x;
                    actor->y = new_map->stair_up_y;
                }

                if (interactions.ascend && tile->type == TILE_TYPE_STAIR_UP)
                {
                    // msg_log("actor ascends", actor->map, actor->x, actor->y);

                    move_to_next = false;

                    if (actor->map->level > 0)
                    {
                        map_t *new_map = TCOD_list_get(maps, actor->map->level - 1);

                        TCOD_list_remove(actor->map->actors, actor);
                        TCOD_list_push(new_map->actors, actor);

                        actor->map->tiles[actor->x][actor->y].actor = NULL;
                        new_map->tiles[new_map->stair_down_x][new_map->stair_down_y].actor = actor;

                        actor->map = new_map;
                        actor->x = new_map->stair_down_x;
                        actor->y = new_map->stair_down_y;
                    }
                }
            }

            if (move_to_next)
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
        else
        {
            actor->target = false;
        }

        if (actor->target_data.actor != NULL && actor->target_data.actor->mark_for_delete)
        {
            actor->target = false;
        }
    }
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

    TCOD_list_delete(actor->spells);

    if (actor->fov_map != NULL)
    {
        TCOD_map_delete(actor->fov_map);
    }

    free(actor);
}