#include <libtcod/libtcod.h>

#include "config.h"

const char *font_file;
int font_flags;
int font_char_horiz;
int font_char_vertic;

int console_width;
int console_height;
bool fullscreen;
int console_renderer;

void config_init(void)
{
    // TODO: external config file
    font_file = "terminal16x16.png";
    font_flags = TCOD_FONT_LAYOUT_ASCII_INROW;
    font_char_horiz = 16;
    font_char_vertic = 16;

    console_width = 80;
    console_height = 40;
    fullscreen = false;
    console_renderer = TCOD_RENDERER_SDL;
}
