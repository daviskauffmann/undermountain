#include <stdint.h>
#include <libtcod.h>

#include "map.h"
#include "tile.h"
#include "room.h"
#include "entity.h"

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

    for (int i = 0; i < MAP_MAX_ENTITIES; i++)
    {
        entity_t *entity = &map->entities[i];
        entity->is_active = false;
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
        entity_t *entity = map_entity_create(map, rand() % MAP_WIDTH, rand() % MAP_HEIGHT, '@', TCOD_yellow);
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

        room_init(room, x, y, w, h);

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

void map_update(map_t *map)
{
    for (int i = 0; i < MAP_MAX_ENTITIES; i++)
    {
        entity_t *entity = &map->entities[i];

        if (!entity->is_active || entity->is_player)
        {
            continue;
        }

        map_entity_update(map, entity);
    }
}

entity_t *map_entity_create(map_t *map, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color)
{
    for (int i = 0; i < MAP_MAX_ENTITIES; i++)
    {
        entity_t *entity = &map->entities[i];

        if (entity->is_active)
        {
            continue;
        }

        entity_init(entity, x, y, glyph, color);

        return entity;
    }

    return NULL;
}

void map_entity_update(map_t *map, entity_t *entity)
{
    int dir = rand() % 8;
    switch (dir)
    {
    case 0:
        map_entity_move(map, entity, 0, -1);
        break;
    case 1:
        map_entity_move(map, entity, 0, 1);
        break;
    case 2:
        map_entity_move(map, entity, -1, 0);
        break;
    case 3:
        map_entity_move(map, entity, 1, 0);
        break;
    }
}

void map_entity_move(map_t *map, entity_t *entity, int dx, int dy)
{
    int x = entity->x + dx;
    int y = entity->y + dy;

    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
    {
        return;
    }

    tile_t *tile = &map->tiles[x][y];
    if (!tileinfo[tile->type].is_walkable)
    {
        return;
    }

    for (int i = 0; i < MAP_MAX_ENTITIES; i++)
    {
        entity_t *other = &map->entities[i];

        if (!other->is_active || other->is_player)
        {
            continue;
        }

        if (other->x != x || other->y != y)
        {
            continue;
        }

        entity_destroy(other);

        return;
    }

    entity->x = x;
    entity->y = y;
}

TCOD_map_t map_calc_fov(map_t *map, entity_t *entity)
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

    // TODO: entity->sight_radius
    TCOD_map_compute_fov(fov_map, entity->x, entity->y, 10, true, FOV_DIAMOND);

    return fov_map;
}

void map_draw(map_t *map, entity_t *player)
{
    TCOD_map_t fov_map = map_calc_fov(map, player);

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

    for (int i = 0; i < MAP_MAX_ENTITIES; i++)
    {
        entity_t *entity = &map->entities[i];

        if (!entity->is_active)
        {
            continue;
        }

        if (!TCOD_map_is_in_fov(fov_map, entity->x, entity->y))
        {
            continue;
        }

        TCOD_console_set_char_foreground(NULL, entity->x, entity->y, entity->color);
        TCOD_console_set_char(NULL, entity->x, entity->y, entity->glyph);
    }

    TCOD_console_flush();
}