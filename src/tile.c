#include <libtcod.h>
#include "tile.h"

void tile_init(Tile *tile, TileType type)
{
    tile->type = type;
}

void tile_draw(Tile *tile, int x, int y)
{
    TCOD_color_t color = TCOD_white;
    char glyph = ' ';

    switch (tile->type)
    {
    case TILE_FLOOR:
        glyph = '.';
        break;
    case TILE_WALL:
        glyph = '#';
        break;
    }

    TCOD_console_set_default_foreground(NULL, color);
    TCOD_console_put_char(NULL, x, y, glyph, TCOD_BKGND_NONE);
}