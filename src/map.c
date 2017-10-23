#include <stdint.h>
#include <stdio.h>
#include <libtcod.h>

#include "map.h"
#include "world.h"

TCOD_map_t map_to_TCOD_map(map_t *map);
void map_calc_fov(TCOD_map_t TCOD_map, int x, int y, int radius);
TCOD_path_t map_calc_path(TCOD_map_t TCOD_map, int ox, int oy, int dx, int dy);

map_t *map_create()
{
    map_t *map = (map_t *)malloc(sizeof(map_t));

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];
            tile->type = TILETYPE_EMPTY;
            tile->seen = false;
        }
    }

    map->stair_down_x = TCOD_random_get_int(NULL, 0, MAP_WIDTH - 1);
    map->stair_down_y = TCOD_random_get_int(NULL, 0, MAP_HEIGHT - 1);
    tile_t *stair_down_tile = &map->tiles[map->stair_down_x][map->stair_down_y];
    stair_down_tile->type = TILETYPE_STAIR_DOWN;

    map->stair_up_x = TCOD_random_get_int(NULL, 0, MAP_WIDTH - 1);
    map->stair_up_y = TCOD_random_get_int(NULL, 0, MAP_HEIGHT - 1);
    tile_t *stair_up_tile = &map->tiles[map->stair_up_x][map->stair_up_y];
    stair_up_tile->type = TILETYPE_STAIR_UP;

    map->actors = TCOD_list_new();

    // TODO: BSP map generation
    for (int x = 20; x < 40; x++)
    {
        for (int y = 20; y < 40; y++)
        {
            tile_t *tile = &map->tiles[x][y];
            tile->type = TILETYPE_FLOOR;
        }
    }

    for (int i = 0; i < 20; i++)
    {
        actor_create(map, TCOD_random_get_int(NULL, 0, MAP_WIDTH - 1), TCOD_random_get_int(NULL, 0, MAP_HEIGHT - 1), '@', TCOD_yellow, 10);
    }

    return map;
}

void map_update(map_t *map)
{
    for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
         iterator != (actor_t **)TCOD_list_end(map->actors);
         iterator++)
    {
        actor_t *actor = *iterator;

        if (actor == player)
        {
            continue;
        }

        if (TCOD_random_get_int(NULL, 0, 1) == 0)
        {
            TCOD_map_t TCOD_map = map_to_TCOD_map(map);

            map_calc_fov(TCOD_map, actor->x, actor->y, actor->sight_radius);

            for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
                 iterator != (actor_t **)TCOD_list_end(map->actors);
                 iterator++)
            {
                actor_t *other = *iterator;

                if (other == actor)
                {
                    continue;
                }

                if (TCOD_map_is_in_fov(TCOD_map, other->x, other->y))
                {
                    // TODO: maybe store the path on the actor somehow so it can be reused
                    TCOD_path_t path = map_calc_path(TCOD_map, actor->x, actor->y, other->x, other->y);

                    if (TCOD_path_is_empty(path))
                    {
                        goto end;
                    }

                    int x, y;
                    if (!TCOD_path_walk(path, &x, &y, false))
                    {
                        goto end;
                    }

                    actor_move(map, actor, x, y);

                end:
                    TCOD_path_delete(path);

                    break;
                }
            }

            TCOD_map_delete(TCOD_map);
        }
        else
        {
            int dir = TCOD_random_get_int(NULL, 0, 8);
            switch (dir)
            {
            case 0:
                actor_move(map, actor, actor->x, actor->y - 1);
                break;
            case 1:
                actor_move(map, actor, actor->x, actor->y + 1);
                break;
            case 2:
                actor_move(map, actor, actor->x - 1, actor->y);
                break;
            case 3:
                actor_move(map, actor, actor->x + 1, actor->y);
                break;
            }
        }
    }
}

void map_draw(map_t *map)
{
    TCOD_map_t TCOD_map = map_to_TCOD_map(map);

    map_calc_fov(TCOD_map, player->x, player->y, player->sight_radius);

    TCOD_console_clear(NULL);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            TCOD_color_t color;
            if (TCOD_map_is_in_fov(TCOD_map, x, y))
            {
                tile->seen = true;

                color = tileinfo[tile->type].color;
            }
            else
            {
                if (tile->seen)
                {
                    color = TCOD_gray;
                }
                else
                {
                    continue;
                }
            }

            TCOD_console_set_char_foreground(NULL, x, y, color);
            TCOD_console_set_char(NULL, x, y, tileinfo[tile->type].glyph);
        }
    }

    for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
         iterator != (actor_t **)TCOD_list_end(map->actors);
         iterator++)
    {
        actor_t *actor = *iterator;

        if (!TCOD_map_is_in_fov(TCOD_map, actor->x, actor->y))
        {
            continue;
        }

        TCOD_console_set_char_foreground(NULL, actor->x, actor->y, actor->color);
        TCOD_console_set_char(NULL, actor->x, actor->y, actor->glyph);
    }

    TCOD_console_flush();

    TCOD_map_delete(TCOD_map);
}

TCOD_map_t map_to_TCOD_map(map_t *map)
{
    TCOD_map_t TCOD_map = TCOD_map_new(MAP_WIDTH, MAP_HEIGHT);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            TCOD_map_set_properties(TCOD_map, x, y, tileinfo[tile->type].is_transparent, tileinfo[tile->type].is_walkable);
        }
    }

    for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
         iterator != (actor_t **)TCOD_list_end(map->actors);
         iterator++)
    {
        actor_t *actor = *iterator;

        TCOD_map_set_properties(TCOD_map, actor->x, actor->y, TCOD_map_is_transparent(TCOD_map, actor->x, actor->y), false);
    }

    return TCOD_map;
}

void map_calc_fov(TCOD_map_t TCOD_map, int x, int y, int radius)
{
    TCOD_map_compute_fov(TCOD_map, x, y, radius, true, FOV_DIAMOND);
}

TCOD_path_t map_calc_path(TCOD_map_t TCOD_map, int ox, int oy, int dx, int dy)
{
    TCOD_map_set_properties(TCOD_map, dx, dy, TCOD_map_is_transparent(TCOD_map, dx, dy), true);

    TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 0.0f);
    TCOD_path_compute(path, ox, oy, dx, dy);

    return path;
}

actor_t *actor_create(map_t *map, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color, uint8_t sight_radius)
{
    actor_t *actor = (actor_t *)malloc(sizeof(actor_t));

    actor->x = x;
    actor->y = y;
    actor->glyph = glyph;
    actor->color = color;
    actor->sight_radius = sight_radius;

    TCOD_list_push(map->actors, actor);

    return actor;
}

void actor_destroy(map_t *map, actor_t *actor)
{
    TCOD_list_remove(map->actors, actor);

    free(actor);
}

void actor_move(map_t *map, actor_t *actor, uint8_t x, uint8_t y)
{
    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
    {
        return;
    }

    tile_t *tile = &map->tiles[x][y];
    if (!tileinfo[tile->type].is_walkable)
    {
        return;
    }

    for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
         iterator != (actor_t **)TCOD_list_end(map->actors);
         iterator++)
    {
        actor_t *other = *iterator;

        if (other->x != x || other->y != y)
        {
            continue;
        }

        // TODO: damage and health
        // TODO: player death
        // TODO: dealing with corpses, is_dead flag or separate object alltogether?
        // if corpses can be resurrected, they will need to store information about the actor
        // if corpses can be picked up, they will need to act like items
        if (other != player)
        {
            actor_destroy(map, other);
        }

        return;
    }

    actor->x = x;
    actor->y = y;
}