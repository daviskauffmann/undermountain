#include <libtcod.h>

#include "tile.h"
#include "item.h"

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
    for (item_t **iterator = (item_t **)TCOD_list_begin(tile->items);
         iterator != (item_t **)TCOD_list_end(tile->items);
         iterator++)
    {
        item_t *item = *iterator;

        item_destroy(item);
    }

    TCOD_list_clear_and_delete(tile->items);
}