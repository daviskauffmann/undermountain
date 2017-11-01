#include <stdio.h>
#include <libtcod.h>

#include "config.h"
#include "world.h"

// TODO: get all this information from external config file
void config_initialize(void)
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

    tile_info[TILE_EMPTY].glyph = ' ';
    tile_info[TILE_EMPTY].light_color = TCOD_white;
    tile_info[TILE_EMPTY].dark_color = TCOD_darkest_gray;
    tile_info[TILE_EMPTY].is_transparent = true;
    tile_info[TILE_EMPTY].is_walkable = true;

    tile_info[TILE_FLOOR].glyph = '.';
    tile_info[TILE_FLOOR].light_color = TCOD_white;
    tile_info[TILE_FLOOR].dark_color = TCOD_darkest_gray;
    tile_info[TILE_FLOOR].is_transparent = true;
    tile_info[TILE_FLOOR].is_walkable = true;

    tile_info[TILE_WALL].glyph = '#';
    tile_info[TILE_WALL].light_color = TCOD_white;
    tile_info[TILE_WALL].dark_color = TCOD_darkest_gray;
    tile_info[TILE_WALL].is_transparent = false;
    tile_info[TILE_WALL].is_walkable = false;

    tile_info[TILE_STAIR_DOWN].glyph = '>';
    tile_info[TILE_STAIR_DOWN].light_color = TCOD_white;
    tile_info[TILE_STAIR_DOWN].dark_color = TCOD_darkest_gray;
    tile_info[TILE_STAIR_DOWN].is_transparent = true;
    tile_info[TILE_STAIR_DOWN].is_walkable = true;

    tile_info[TILE_STAIR_UP].glyph = '<';
    tile_info[TILE_STAIR_UP].light_color = TCOD_white;
    tile_info[TILE_STAIR_UP].dark_color = TCOD_darkest_gray;
    tile_info[TILE_STAIR_UP].is_transparent = true;
    tile_info[TILE_STAIR_UP].is_walkable = true;

    actor_info[ACTOR_PLAYER].glyph = '@';
    actor_info[ACTOR_PLAYER].color = TCOD_white;
    actor_info[ACTOR_PLAYER].sight_radius = 5;

    actor_info[ACTOR_MONSTER].glyph = '@';
    actor_info[ACTOR_MONSTER].color = TCOD_red;
    actor_info[ACTOR_MONSTER].sight_radius = 5;

    TCOD_sys_set_fps(FPS);

    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(screen_width, screen_height, WINDOW_TITLE, fullscreen, renderer);
}