#include <libtcod.h>

#include "CMemLeak.h"
#include "system.h"

// TODO: get all this information from external config file
void system_init(void)
{
    screen_width = 40;
    screen_height = 25;
    fullscreen = false;
    renderer = TCOD_RENDERER_SDL;

    font_file = "Msgothic.png";
    font_flags = TCOD_FONT_LAYOUT_ASCII_INROW;
    font_char_horiz = 16;
    font_char_vertic = 16;

    TCOD_sys_set_fps(FPS);

    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(screen_width, screen_height, WINDOW_TITLE, fullscreen, renderer);
}