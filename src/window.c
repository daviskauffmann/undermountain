#include <libtcod/libtcod.h>

#include "window.h"

const char *font_file;
int font_flags;
int font_char_horiz;
int font_char_vertic;

int console_width;
int console_height;
bool fullscreen;
int console_renderer;

void window_init(void)
{
    // TODO: external config file
    font_file = "terminal.png";
    font_flags = TCOD_FONT_LAYOUT_ASCII_INCOL;
    font_char_horiz = 16;
    font_char_vertic = 16;

    console_width = 80;
    console_height = 50;
    fullscreen = false;
    console_renderer = TCOD_RENDERER_SDL;

    TCOD_sys_set_fps(FPS);

    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(console_width, console_height, WINDOW_TITLE, fullscreen, console_renderer);
}

bool window_is_closed(void)
{
    return TCOD_console_is_window_closed();
}
