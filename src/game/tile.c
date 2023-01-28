#include "tile.h"

void tile_init(
    struct tile *const tile,
    const enum tile_type type,
    const bool seen)
{
    tile->type = type;
    tile->explored = seen;
    tile->object = NULL;
    tile->actor = NULL;
    tile->corpses = list_new();
    tile->items = list_new();
}

void tile_uninit(struct tile *const tile)
{
    list_delete(tile->items);
    list_delete(tile->corpses);
}
