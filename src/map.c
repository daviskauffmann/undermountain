#include <libtcod.h>

#include "entity.h"
#include "map.h"
#include "tile.h"

void map_init(Map *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_init(&map->tiles[x][y], TILE_EMPTY);
        }
    }

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_init(&map->entities[i], ID_UNUSED, 0, 0, ' ', TCOD_white);
    }
}

Entity *map_createEntity(Map *map, int x, int y, char glyph, TCOD_color_t color)
{
    for (int i = ID_PLAYER + 1; i < MAX_ENTITIES; i++)
    {
        Entity *entity = &map->entities[i];

        if (entity->id == ID_UNUSED)
        {
            entity_init(entity, i, x, y, glyph, color);
            return entity;
        }
    }

    return NULL;
}

void map_generate(Map *map)
{
    // TODO: libtcod BSP generation
}

void map_update(Map *map)
{
    for (int i = ID_PLAYER + 1; i < MAX_ENTITIES; i++)
    {
        Entity *entity = &map->entities[i];

        if (entity->id == ID_UNUSED)
        {
            break;
        }

        entity_think(entity);
    }
}

void map_draw(Map *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            Tile *tile = &map->tiles[x][y];

            // TODO: check if visible
            tile_draw(tile, x, y /*, visible: bool*/);
        }
    }

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        Entity *entity = &map->entities[i];

        if (entity->id == ID_UNUSED)
        {
            break;
        }

        // TODO: check if visible
        entity_draw(entity);
    }

    TCOD_console_flush();
}