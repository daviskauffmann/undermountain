#include "tile.h"

void tile_init(struct tile *tile, enum tile_type type, bool seen)
{
    tile->type = type;
    tile->seen = seen;
    tile->actor = NULL;
    tile->corpses = TCOD_list_new();
    tile->items = TCOD_list_new();
}

void tile_reset(struct tile *tile)
{
    TCOD_list_delete(tile->corpses);
    TCOD_list_delete(tile->items);
}
