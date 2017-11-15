#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "game.h"
#include "utils.h"

#define LIT_ROOMS 0

actor_t *actor_create(actor_type_t type, map_t *map, int x, int y, void (*ai)(actor_t *actor), char *unique_name)
{
    actor_t *actor = (actor_t *)malloc(sizeof(actor_t));

    actor->type = type;
    actor->map = map;
    actor->x = x;
    actor->y = y;
    actor->ai = ai;
    actor->unique_name = unique_name;
    actor->items = TCOD_list_new();
    actor->spells = TCOD_list_new();
    actor->spell_ready = NULL;
    actor->light = ACTOR_LIGHT_TYPE_DEFAULT;
    actor->fov_map = NULL;
    actor->energy = 1.0f;
    actor->mark_for_delete = false;

    actor_calc_fov(actor);

    return actor;
}

void actor_update(actor_t *actor)
{
    actor_calc_fov(actor);

    if (actor->ai != NULL)
    {
        actor->energy += actor_info[actor->type].energy_per_turn;

        while (actor->energy >= 1.0f)
        {
            actor->energy -= 1.0f;

            actor->ai(actor);
        }
    }
}

char *actor_get_name(actor_t *actor)
{
    return actor->unique_name == NULL ? actor_info[actor->type].name : actor->unique_name;
}

void actor_calc_fov(actor_t *actor)
{
    if (actor->fov_map != NULL)
    {
        TCOD_map_delete(actor->fov_map);
    }

    actor->fov_map = map_to_fov_map(actor->map, actor->x, actor->y, actor_light_info[actor->light].radius);

    TCOD_map_t los_map = map_to_fov_map(actor->map, actor->x, actor->y, 0);

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

                    if ((other->light == ACTOR_LIGHT_TYPE_DEFAULT || other->light == ACTOR_LIGHT_TYPE_TORCH) && TCOD_map_is_in_fov(other->fov_map, x, y))
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
    tile_t *tile = &actor->map->tiles[x][y];

    if (tile->actor != NULL)
    {
        // should determine if actor is hostile
        if (tile->actor->ai == &ai_monster)
        {
            actor_attack(actor, tile->actor);
        }
        else
        {
            actor_swap(actor, tile->actor);
        }
    }
    else
    {
        actor_move(actor, x, y);
    }
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

        actor_look(actor, x, y);
    }
}

void actor_look(actor_t *actor, int x, int y)
{
    tile_t *tile = &actor->map->tiles[x][y];

    item_t *item = TCOD_list_peek(tile->items);
    if (item != NULL)
    {
        msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s sees %s", actor_get_name(actor), item_info[item->type].name);
    }

    if (tile->light != NULL)
    {
        msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s sees a light", actor_get_name(actor));
    }
}

void actor_swap(actor_t *actor, actor_t *other)
{
    int tx = actor->x;
    int ty = actor->y;

    actor->x = other->x;
    actor->y = other->y;

    other->x = tx;
    other->y = ty;

    actor->map->tiles[actor->x][actor->y].actor = actor;
    other->map->tiles[other->x][other->y].actor = other;

    msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s swaps with %s", actor_get_name(actor), actor_get_name(other));
}

void actor_path_towards(actor_t *actor, int x, int y)
{
    TCOD_map_t TCOD_map = map_to_TCOD_map(actor->map);
    TCOD_map_set_properties(TCOD_map, x, y, TCOD_map_is_transparent(actor->fov_map, x, y), true);

    TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1.0f);
    TCOD_path_compute(path, actor->x, actor->y, x, y);

    int next_x, next_y;
    if (!TCOD_path_is_empty(path) && TCOD_path_walk(path, &next_x, &next_y, false))
    {
        actor_move(actor, next_x, next_y);
    }
    else
    {
        actor_move_towards(actor, x, y);
    }

    TCOD_path_delete(path);
    TCOD_map_delete(TCOD_map);
}

void actor_move_towards(actor_t *actor, int x, int y)
{
    int dx = x - actor->x;
    int dy = y - actor->y;
    float distance = distance(actor->x, actor->y, x, y);

    dx = round(dx / distance);
    dy = round(dy / distance);

    actor_move(actor, actor->x + dx, actor->y + dy);
}

void actor_light_toggle(actor_t *actor, light_t *light)
{
    light->on = !light->on;

    if (light->on)
    {
        msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s turns on the light", actor_get_name(actor));
    }
    else
    {
        msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s turns off the light", actor_get_name(actor));
    }
}

void actor_item_take(actor_t *actor, TCOD_list_t items)
{
    item_t *item = TCOD_list_pop(items);

    item->x = actor->x;
    item->y = actor->y;

    TCOD_list_push(actor->items, item);

    msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s takes %s", actor_get_name(actor), item_info[item->type].name);
}

void actor_descend(actor_t *actor)
{
    tile_t *tile = &actor->map->tiles[actor->x][actor->y];

    if (tile->type == TILE_TYPE_STAIR_DOWN)
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

        msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s descends to level %d", actor_get_name(actor), actor->map->level);
    }
}

void actor_ascend(actor_t *actor)
{
    tile_t *tile = &actor->map->tiles[actor->x][actor->y];

    if (tile->type == TILE_TYPE_STAIR_UP && actor->map->level > 0)
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

        msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s ascends to level %d", actor_get_name(actor), actor->map->level);
    }
}

void actor_swing(actor_t *actor, int x, int y)
{
    tile_t *tile = &actor->map->tiles[x][y];

    // TODO: check if there is something destructible here
    if (tile->actor)
    {
        actor_attack(actor, tile->actor);
    }
    else
    {
        msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s swings at the air", actor_get_name(actor));
    }
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

void actor_attack(actor_t *actor, actor_t *other)
{
    msg_log(actor->map, actor->x, actor->y, TCOD_white, "%s attacks %s", actor_get_name(actor), actor_get_name(other));

    if (other != player)
    {
        other->mark_for_delete = true;
    }
}

void actor_inventory_add(actor_t *actor, item_t *item)
{
}

void actor_draw(actor_t *actor)
{
    if (TCOD_map_is_in_fov(player->fov_map, actor->x, actor->y))
    {
        TCOD_console_set_char_foreground(NULL, actor->x - view_x, actor->y - view_y, actor_info[actor->type].color);
        TCOD_console_set_char(NULL, actor->x - view_x, actor->y - view_y, actor_info[actor->type].glyph);
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