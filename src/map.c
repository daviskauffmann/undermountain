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
        }
    }

    for (int i = 0; i < MAP_MAX_ENTITIES; i++)
    {
        entity_t *entity = &map->entities[i];
        entity->id = ENTITY_ID_UNUSED;
    }
}

void map_generate(map_t *map)
{
    // TODO: libtcod BSP generation
    room_t *room = &map->rooms[0];
    room_init(room, 20, 15, 10, 15);
    map_room_carve(map, room);

    entity_t *player = &map->entities[ENTITY_ID_PLAYER];
    entity_init(player, ENTITY_ID_PLAYER, 1, 1, '@', TCOD_white);

    for (int i = 0; i < 10; i++)
    {
        entity_t *npc = &map->entities[i];
        entity_init(npc, i, rand() % MAP_WIDTH, rand() % MAP_HEIGHT, '@', TCOD_yellow);
    }
}

void map_room_carve(map_t *map, room_t *room)
{
    for (int x = room->x1; x < room->x2; x++)
    {
        for (int y = room->y1; y < room->y2; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile->type = TILETYPE_FLOOR;
        }
    }
}

void map_update(map_t *map)
{
    for (int i = 0; i < MAP_MAX_ENTITIES; i++)
    {
        entity_t *entity = &map->entities[i];

        if (entity->id == ENTITY_ID_UNUSED)
        {
            continue;
        }

        if (entity->id != ENTITY_ID_PLAYER)
        {
            map_entity_update(map, entity);
        }
    }
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
    if (tileinfo[tile->type].solid)
    {
        return;
    }

    for (int i = 0; i < MAP_MAX_ENTITIES; i++)
    {
        entity_t *other = &map->entities[i];

        if (other->id == entity->id || other->id == ENTITY_ID_UNUSED || other->id == ENTITY_ID_PLAYER)
        {
            continue;
        }

        if (other->x == x && other->y == y)
        {
            entity_destroy(other);

            return;
        }
    }

    entity->x = x;
    entity->y = y;
}

void map_draw(map_t *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            // TODO: check if visible
            tile_draw(tile, x, y /*, visible: bool*/);
        }
    }

    for (int i = 0; i < MAP_MAX_ENTITIES; i++)
    {
        entity_t *entity = &map->entities[i];

        if (entity->id == ENTITY_ID_UNUSED)
        {
            continue;
        }

        // TODO: check if visible
        entity_draw(entity);
    }

    TCOD_console_flush();
}