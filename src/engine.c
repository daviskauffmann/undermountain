#include <libtcod/libtcod.h>

#include "engine.h"

int screen_width;
int screen_height;
int console_width;
int console_height;
bool fullscreen;
int renderer;

const char *font_file;
int font_flags;
int font_char_horiz;
int font_char_vertic;

void engine_init(void)
{
    // TODO: external config file
    screen_width = 1366;
    screen_height = 768;
    console_width = 80;
    console_height = 50;
    fullscreen = false;
    renderer = TCOD_RENDERER_SDL;

    font_file = "terminal.png";
    font_flags = TCOD_FONT_LAYOUT_ASCII_INCOL;
    font_char_horiz = 16;
    font_char_vertic = 16;

    TCOD_sys_set_fps(FPS);

    TCOD_console_set_custom_font(font_file, font_flags, font_char_horiz, font_char_vertic);
    TCOD_console_init_root(console_width, console_height, WINDOW_TITLE, fullscreen, renderer);
}

void engine_quit(void)
{
}
