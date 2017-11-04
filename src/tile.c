#include <libtcod.h>

#include "tile.h"
#include "item.h"

void tiles_initialize(void)
{
    tile_glyph[TILE_EMPTY] = ' ';
    tile_glyph[TILE_FLOOR] = '.';
    tile_glyph[TILE_WALL] = '#';
    tile_glyph[TILE_STAIR_DOWN] = '>';
    tile_glyph[TILE_STAIR_UP] = '<';

    tile_transparent[TILE_EMPTY] = true;
    tile_transparent[TILE_FLOOR] = true;
    tile_transparent[TILE_WALL] = false;
    tile_transparent[TILE_STAIR_DOWN] = true;
    tile_transparent[TILE_STAIR_UP] = true;

    tile_walkable[TILE_EMPTY] = true;
    tile_walkable[TILE_FLOOR] = true;
    tile_walkable[TILE_WALL] = false;
    tile_walkable[TILE_STAIR_DOWN] = true;
    tile_walkable[TILE_STAIR_UP] = true;
}

void tile_initialize(tile_t *tile, tile_type_t type)
{
    tile->type = TILE_WALL;
    tile->seen = false;
    tile->light = NULL;
    tile->actor = NULL;
    tile->items = TCOD_list_new();
}

void tile_finalize(tile_t *tile)
{
    for (void **i = TCOD_list_begin(tile->items); i != TCOD_list_end(tile->items); i++)
    {
        item_t *item = *i;

        item_destroy(item);
    }

    TCOD_list_clear_and_delete(tile->items);
}