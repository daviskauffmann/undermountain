#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "game.h"

#define LIT_ROOMS 0

actor_t *actor_create(actor_type_t type, map_t *map, int x, int y)
{
    actor_t *actor = (actor_t *)malloc(sizeof(actor_t));

    actor->type = type;
    actor->map = map;
    actor->x = x;
    actor->y = y;
    actor->items = TCOD_list_new();
    actor->spells = TCOD_list_new();
    actor->spell_ready = NULL;
    actor->light = ACTOR_LIGHT_DEFAULT;
    actor->fov_map = NULL;
    actor->energy = 1.0f;
    actor->mark_for_delete = false;

    actor_calc_fov(actor);

    return actor;
}

void actor_update(actor_t *actor)
{
    actor_calc_fov(actor);

    if (actor_ai[actor->type] != NULL)
    {
        actor->energy += actor_energy_per_turn[actor->type];

        while (actor->energy >= 1.0f)
        {
            actor->energy -= 1.0f;

            actor_ai[actor->type](actor);
        }
    }
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

void actor_default_action(actor_t *actor, int x, int y)
{
    // should take a coordinate and figure out what the best action to do there is
}

void actor_move(actor_t *actor, int x, int y)
{
    if (TCOD_map_is_walkable(actor->fov_map, x, y))
    {
        actor->map->tiles[actor->x][actor->y].actor = NULL;
        actor->map->tiles[x][y].actor = actor;

        actor->x = x;
        actor->y = y;

        for (void **i = TCOD_list_begin(actor->items); i != TCOD_list_end(actor->items); i++)
        {
            item_t *item = *i;

            item->x = actor->x;
            item->y = actor->y;
        }
    }
}

void actor_move_towards(actor_t *actor, int x, int y)
{
    // should move in a direction towards the target
}

void actor_path_towards(actor_t *actor, int x, int y)
{
    // should calculate a path and move towards the target
}

void actor_light_toggle(actor_t *actor, light_t *light)
{
    light->on = !light->on;

    if (light->on)
    {
        msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s turns on the light", actor_name[actor->type]);
    }
    else
    {
        msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s turns off the light", actor_name[actor->type]);
    }
}

void actor_item_take(actor_t *actor, TCOD_list_t items)
{
    item_t *item = TCOD_list_pop(items);

    item->x = actor->x;
    item->y = actor->y;

    TCOD_list_push(actor->items, item);

    msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s takes %s", actor_name[actor->type], item->name);
}

void actor_descend(actor_t *actor)
{
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

    actor_calc_fov(actor);

    msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s descends to level %d", actor_name[actor->type], actor->map->level);
}

void actor_ascend(actor_t *actor)
{
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

        actor_calc_fov(actor);

        msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s ascends to level %d", actor_name[actor->type], actor->map->level);
    }
}

void actor_swing(actor_t *actor, int x, int y)
{
    // should perform a melee attack at the target, whether there is something there or not
}

void actor_shoot(actor_t *actor, int x, int y)
{
    // should fire a ranged weapon at the target coordinate
    // possibly spawning a projectile graphic
}

void actor_cast(actor_t *actor, spell_t *spell, int x, int y)
{
    // should cast a spell at the target coordinate
    spell->cast(actor, x, y);
}

void actor_draw(actor_t *actor)
{
    actor_calc_fov(actor);

    if (TCOD_map_is_in_fov(player->fov_map, actor->x, actor->y))
    {
        TCOD_console_set_char_foreground(NULL, actor->x - view_x, actor->y - view_y, actor_color[actor->type]);
        TCOD_console_set_char(NULL, actor->x - view_x, actor->y - view_y, actor_glyph[actor->type]);
    }
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