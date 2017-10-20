#include <libtcod.h>

#include "tile.h"

tileinfo_t tileinfo[TILETYPE_COUNT];

void tileinfo_init(void)
{
    tileinfo[TILETYPE_EMPTY].glyph = ' ';
    tileinfo[TILETYPE_EMPTY].color = TCOD_white;
    tileinfo[TILETYPE_EMPTY].opaque = false;
    tileinfo[TILETYPE_EMPTY].solid = false;

    tileinfo[TILETYPE_FLOOR].glyph = '.';
    tileinfo[TILETYPE_FLOOR].color = TCOD_white;
    tileinfo[TILETYPE_FLOOR].opaque = false;
    tileinfo[TILETYPE_FLOOR].solid = false;

    tileinfo[TILETYPE_WALL].glyph = '#';
    tileinfo[TILETYPE_WALL].color = TCOD_white;
    tileinfo[TILETYPE_WALL].opaque = true;
    tileinfo[TILETYPE_WALL].solid = true;
}

void tile_draw(tile_t *tile, int x, int y)
{
    // TODO: use visibility to determine color
    // if visible && seen
    //     white
    // if !visible && seen
    //     grey
    // if !visible && !seen
    //     don't draw
    TCOD_console_set_default_foreground(NULL, tileinfo[tile->type].color);
    TCOD_console_put_char(NULL, x, y, tileinfo[tile->type].glyph, TCOD_BKGND_NONE);
}