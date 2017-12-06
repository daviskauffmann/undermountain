#include <libtcod.h>

#include "CMemLeak.h"
#include "config.h"

void config_init(void)
{
    screen_width = 1366;
    screen_height = 768;
    console_width = screen_width / 16;
    console_height = screen_height / 16;
    fullscreen = false;
    renderer = TCOD_RENDERER_SDL;

    font_file = "Anikki_square_16x16.png";
    font_flags = TCOD_FONT_LAYOUT_ASCII_INROW;
    font_char_horiz = 16;
    font_char_vertic = 16;

    TCOD_sys_set_fps(FPS);

    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(console_width, console_height, WINDOW_TITLE, fullscreen, renderer);
}