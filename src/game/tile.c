#include "tile.h"

#include "color.h"
#include "list.h"

const struct tile_metadata tile_metadata = {
    .ambient_light_color = {64, 64, 64},
    .ambient_light_intensity = 0.05f,
};

const struct tile_data tile_database[] = {
    [TILE_TYPE_EMPTY] = {
        .name = "Empty",
        .glyph = ' ',
        .color = {COLOR_WHITE},
        .is_walkable = true,
        .is_transparent = true,
    },
    [TILE_TYPE_FLOOR] = {
        .name = "Floor",
        .glyph = '.',
        .color = {COLOR_WHITE},
        .is_walkable = true,
        .is_transparent = true,
    },
    [TILE_TYPE_GRASS] = {
        .name = "Grass",
        .glyph = '.',
        .color = {COLOR_LIGHT_GREEN},
        .is_walkable = true,
        .is_transparent = true,
    },
    [TILE_TYPE_WALL] = {
        .name = "Wall",
        .glyph = '#',
        .color = {COLOR_WHITE},
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
    tile->corpses = list_new();
    tile->items = list_new();
    tile->surface = NULL;
}

void tile_uninit(struct tile *const tile)
{
    list_delete(tile->items);
    list_delete(tile->corpses);
}
