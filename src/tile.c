#include <libtcod/libtcod.h>

#include "tile.h"

void tile_init(Tile *tile, TileType type, bool seen)
{
    tile->type = type;
    tile->seen = seen;
    tile->objects = TCOD_list_new();
    tile->actors = TCOD_list_new();
    tile->items = TCOD_list_new();
}

void tile_reset(Tile *tile)
{
    TCOD_list_delete(tile->objects);
    TCOD_list_delete(tile->actors);
    TCOD_list_delete(tile->items);
}
