#include <stdint.h>
#include <libtcod.h>

#include "map.h"

tileinfo_t tileinfo[NB_TILETYPES];

void tileinfo_init(void)
{
    tileinfo[TILETYPE_EMPTY].glyph = ' ';
    tileinfo[TILETYPE_EMPTY].color = TCOD_white;
    tileinfo[TILETYPE_EMPTY].is_transparent = true;
    tileinfo[TILETYPE_EMPTY].is_walkable = true;

    tileinfo[TILETYPE_FLOOR].glyph = '.';
    tileinfo[TILETYPE_FLOOR].color = TCOD_white;
    tileinfo[TILETYPE_FLOOR].is_transparent = true;
    tileinfo[TILETYPE_FLOOR].is_walkable = true;

    tileinfo[TILETYPE_WALL].glyph = '#';
    tileinfo[TILETYPE_WALL].color = TCOD_white;
    tileinfo[TILETYPE_WALL].is_transparent = false;
    tileinfo[TILETYPE_WALL].is_walkable = false;
}

void map_init(map_t *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];
            tile->type = TILETYPE_EMPTY;
            tile->seen = false;
        }
    }

    for (int i = 0; i < MAP_MAX_ROOMS; i++)
    {
        room_t *room = &map->rooms[i];
        room->is_created = false;
    }

    for (int i = 0; i < MAP_MAX_ACTORS; i++)
    {
        actor_t *actor = &map->actors[i];
        actor->is_active = false;
        actor->is_player = false;
    }
}

void map_generate(map_t *map)
{
    // TCOD_bsp_t *bsp = TCOD_bsp_new_with_size(0, 0, MAP_WIDTH, MAP_HEIGHT);
    // TCOD_bsp_split_recursive(bsp, NULL, 4, 5, 5, 1.5f, 1.5f);
    // TCOD_bsp_delete(bsp);

    map_room_create(map, 20, 10, 40, 20);

    for (int i = 0; i < 10; i++)
    {
        map_actor_create(map, false, rand() % MAP_WIDTH, rand() % MAP_HEIGHT, '@', TCOD_yellow);
    }
}

room_t *map_room_create(map_t *map, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    for (int i = 0; i < MAP_MAX_ROOMS; i++)
    {
        room_t *room = &map->rooms[i];

        if (room->is_created)
        {
            continue;
        }

        room->x1 = x;
        room->y1 = y;
        room->x2 = x + w;
        room->y2 = y + h;
        room->is_created = true;

        for (int x = room->x1; x < room->x2; x++)
        {
            for (int y = room->y1; y < room->y2; y++)
            {
                tile_t *tile = &map->tiles[x][y];
                tile->type = TILETYPE_FLOOR;
            }
        }
    }

    return NULL;
}

actor_t *map_actor_create(map_t *map, bool is_player, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color)
{
    for (int i = 0; i < MAP_MAX_ACTORS; i++)
    {
        actor_t *actor = &map->actors[i];

        if (actor->is_active)
        {
            continue;
        }

        actor->is_active = true;
        actor->is_player = is_player;
        actor->x = x;
        actor->y = y;
        actor->glyph = glyph;
        actor->color = color;

        return actor;
    }

    return NULL;
}

void map_update(map_t *map)
{
    for (int i = 0; i < MAP_MAX_ACTORS; i++)
    {
        actor_t *actor = &map->actors[i];

        if (!actor->is_active || actor->is_player)
        {
            continue;
        }

        int dir = rand() % 8;
        switch (dir)
        {
        case 0:
            map_actor_move(map, actor, 0, -1);
            break;
        case 1:
            map_actor_move(map, actor, 0, 1);
            break;
        case 2:
            map_actor_move(map, actor, -1, 0);
            break;
        case 3:
            map_actor_move(map, actor, 1, 0);
            break;
        }
    }
}

void map_actor_move(map_t *map, actor_t *actor, int dx, int dy)
{
    int x = actor->x + dx;
    int y = actor->y + dy;

    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
    {
        return;
    }

    tile_t *tile = &map->tiles[x][y];
    if (!tileinfo[tile->type].is_walkable)
    {
        return;
    }

    for (int i = 0; i < MAP_MAX_ACTORS; i++)
    {
        actor_t *other = &map->actors[i];

        if (!other->is_active)
        {
            continue;
        }

        if (other->x != x || other->y != y)
        {
            continue;
        }

        // TODO: damage and health
        // TODO: player death
        // TODO: dealing with corpses, is_dead flag or separate object alltogether?
        // if corpses can be resurrected, they will need to store information about the actor
        // if corpses can be picked up, they will need to act like items
        other->is_active = other->is_player ? true : false;

        return;
    }

    actor->x = x;
    actor->y = y;
}

void map_actor_change_map(map_t *mapFrom, map_t *mapTo, actor_t *actor)
{
    for (int i = 0; i < MAP_MAX_ACTORS; i++)
    {
        actor_t *current = &mapFrom->actors[i];

        if (current == actor)
        {
            current->is_active = false;

            break;
        }
    }

    for (int i = 0; i < MAP_MAX_ACTORS; i++)
    {
        actor_t *current = &mapTo->actors[i];

        if (!current->is_active)
        {
            // figure out how to copy the actor to it's place in the new map
            // also, if the moving actor is the player,
            // we need to inform the rest of the game the player's new address
        }
    }
}

TCOD_map_t map_actor_calc_fov(map_t *map, actor_t *actor)
{
    TCOD_map_t fov_map = TCOD_map_new(MAP_WIDTH, MAP_HEIGHT);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            TCOD_map_set_properties(fov_map, x, y, tileinfo[tile->type].is_transparent, tileinfo[tile->type].is_walkable);
        }
    }

    // TODO: actor->sight_radius
    TCOD_map_compute_fov(fov_map, actor->x, actor->y, 10, true, FOV_DIAMOND);

    return fov_map;
}

void map_draw(map_t *map, actor_t *player)
{
    TCOD_map_t fov_map = map_actor_calc_fov(map, player);

    TCOD_console_clear(NULL);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            TCOD_color_t color;
            if (TCOD_map_is_in_fov(fov_map, x, y))
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

    for (int i = 0; i < MAP_MAX_ACTORS; i++)
    {
        actor_t *actor = &map->actors[i];

        if (!actor->is_active)
        {
            continue;
        }

        if (!TCOD_map_is_in_fov(fov_map, actor->x, actor->y))
        {
            continue;
        }

        TCOD_console_set_char_foreground(NULL, actor->x, actor->y, actor->color);
        TCOD_console_set_char(NULL, actor->x, actor->y, actor->glyph);
    }

    TCOD_console_flush();
}