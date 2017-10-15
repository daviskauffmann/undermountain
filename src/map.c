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
        entity_init(&map->entities[i], -1, -1, ' ', TCOD_white);
    }
}

void map_generate(Map *map)
{
}

void map_update(Map *map)
{
    for (int i = 0; i < MAX_ENTITIES - 1; i++)
    {
        if (map->entities[i].x == -1 || map->entities[i].y == -1)
        {
            break;
        }

        entity_think(&map->entities[i]);
    }
}

void map_draw(Map *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_draw(&map->tiles[x][y], x, y);
        }
    }

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_draw(&map->entities[i]);
    }

    TCOD_console_flush();
}