#include "tile.h"

#include "list.h"

const struct tile_data tile_database[] = {
    [TILE_TYPE_EMPTY] = {
        .name = "Empty",
        .glyph = ' ',
        .is_walkable = true,
        .is_transparent = true,
    },
    [TILE_TYPE_FLOOR] = {
        .name = "Floor",
        .glyph = '.',
        .is_walkable = true,
        .is_transparent = true,
    },
    [TILE_TYPE_GRASS] = {
        .name = "Grass",
        .glyph = '.',
        .is_walkable = true,
        .is_transparent = true,
    },
    [TILE_TYPE_WALL] = {
        .name = "Wall",
        .glyph = '#',
        .is_walkable = false,
        .is_transparent = false,
    },
};

void tile_init(
    struct tile *const tile,
    const enum tile_type type,
    const bool seen)
{
    tile->type = type;
    tile->explored = seen;
    tile->object = NULL;
    tile->actor = NULL;
    tile->corpse = NULL;
    tile->item = NULL;
    tile->surface = NULL;
}

void tile_uninit(struct tile *const tile)
{
    tile;
}
