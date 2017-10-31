#include <stdio.h>
#include <libtcod.h>

#include "config.h"
#include "world.h"

// TODO: get all this information from external config file
void config_init(void)
{
    screen_width = 40;
    screen_height = 25;
    fullscreen = false;
    renderer = TCOD_RENDERER_SDL;

    font_file = "Anikki_square_16x16.png";
    font_flags = TCOD_FONT_LAYOUT_ASCII_INROW;
    font_char_horiz = 16;
    font_char_vertic = 16;

    sfx = true;
    default_background_color = TCOD_black;
    default_foreground_color = TCOD_white;
    torch_color = TCOD_light_amber;

    tileinfo[TILETYPE_EMPTY].glyph = ' ';
    tileinfo[TILETYPE_EMPTY].light_color = TCOD_white;
    tileinfo[TILETYPE_EMPTY].dark_color = TCOD_darkest_gray;
    tileinfo[TILETYPE_EMPTY].is_transparent = true;
    tileinfo[TILETYPE_EMPTY].is_walkable = true;

    tileinfo[TILETYPE_FLOOR].glyph = '.';
    tileinfo[TILETYPE_FLOOR].light_color = TCOD_white;
    tileinfo[TILETYPE_FLOOR].dark_color = TCOD_darkest_gray;
    tileinfo[TILETYPE_FLOOR].is_transparent = true;
    tileinfo[TILETYPE_FLOOR].is_walkable = true;

    tileinfo[TILETYPE_WALL].glyph = '#';
    tileinfo[TILETYPE_WALL].light_color = TCOD_white;
    tileinfo[TILETYPE_WALL].dark_color = TCOD_darkest_gray;
    tileinfo[TILETYPE_WALL].is_transparent = false;
    tileinfo[TILETYPE_WALL].is_walkable = false;

    tileinfo[TILETYPE_STAIR_DOWN].glyph = '>';
    tileinfo[TILETYPE_STAIR_DOWN].light_color = TCOD_white;
    tileinfo[TILETYPE_STAIR_DOWN].dark_color = TCOD_darkest_gray;
    tileinfo[TILETYPE_STAIR_DOWN].is_transparent = true;
    tileinfo[TILETYPE_STAIR_DOWN].is_walkable = true;

    tileinfo[TILETYPE_STAIR_UP].glyph = '<';
    tileinfo[TILETYPE_STAIR_UP].light_color = TCOD_white;
    tileinfo[TILETYPE_STAIR_UP].dark_color = TCOD_darkest_gray;
    tileinfo[TILETYPE_STAIR_UP].is_transparent = true;
    tileinfo[TILETYPE_STAIR_UP].is_walkable = true;

    actorinfo[ACTORTYPE_PLAYER].glyph = '@';
    actorinfo[ACTORTYPE_PLAYER].color = TCOD_white;
    actorinfo[ACTORTYPE_PLAYER].sight_radius = 5;

    actorinfo[ACTORTYPE_MONSTER].glyph = '@';
    actorinfo[ACTORTYPE_MONSTER].color = TCOD_red;
    actorinfo[ACTORTYPE_MONSTER].sight_radius = 5;
}