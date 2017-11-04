#include <libtcod.h>

#include "config.h"
#include "console.h"
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

    TCOD_sys_set_fps(FPS);

    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(screen_width, screen_height, WINDOW_TITLE, fullscreen, renderer);
}