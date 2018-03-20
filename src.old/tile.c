#include <libtcod/libtcod.h>

#include "tile.h"

void tile_init(tile_t *tile, tile_type_t type, bool seen)
{
    tile->type = type;
    tile->seen = seen;
    tile->entities = TCOD_list_new();
}

void tile_reset(tile_t *tile)
{
    TCOD_list_delete(tile->entities);
}
