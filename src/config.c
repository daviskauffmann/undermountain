#include <libtcod.h>

#include "config.h"
#include "world.h"

tileinfo_t tileinfo[NB_TILETYPES];

void config_init(void)
{
    // TODO: read these parameters from a config file
    TCOD_console_set_custom_font("terminal.png", TCOD_FONT_LAYOUT_ASCII_INCOL, 16, 16);
    TCOD_console_init_root(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE, false, TCOD_RENDERER_SDL);
    TCOD_console_set_default_background(NULL, TCOD_black);
    TCOD_console_set_default_foreground(NULL, TCOD_white);

    // TODO: move this to a config file
    tileinfo[TILETYPE_EMPTY].glyph = ' ';
    tileinfo[TILETYPE_EMPTY].color = TCOD_white;
    tileinfo[TILETYPE_EMPTY].is_transparent = true;
    tileinfo[TILETYPE_EMPTY].is_walkable = true;

    tileinfo[TILETYPE_FLOOR].glyph = '.';
    tileinfo[TILETYPE_FLOOR].color = TCOD_white;
    tileinfo[TILETYPE_FLOOR].is_transparent = true;
    tileinfo[TILETYPE_FLOOR].is_walkable = true;

    tileinfo[TILETYPE_WALL].glyph = '#';
    tileinfo[TILETYPE_WALL].color = TCOD_white;
    tileinfo[TILETYPE_WALL].is_transparent = false;
    tileinfo[TILETYPE_WALL].is_walkable = false;

    tileinfo[TILETYPE_STAIR_DOWN].glyph = '>';
    tileinfo[TILETYPE_STAIR_DOWN].color = TCOD_white;
    tileinfo[TILETYPE_STAIR_DOWN].is_transparent = true;
    tileinfo[TILETYPE_STAIR_DOWN].is_walkable = true;

    tileinfo[TILETYPE_STAIR_UP].glyph = '<';
    tileinfo[TILETYPE_STAIR_UP].color = TCOD_white;
    tileinfo[TILETYPE_STAIR_UP].is_transparent = true;
    tileinfo[TILETYPE_STAIR_UP].is_walkable = true;
}