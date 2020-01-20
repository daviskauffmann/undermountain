#include "config.h"

const char *font_file;
TCOD_font_flags_t font_flags;
int font_char_horiz;
int font_char_vertic;

int console_width;
int console_height;
bool fullscreen;
TCOD_renderer_t console_renderer;

void config_load(void)
{
    // TODO: load from external file
    font_file = "assets/Anikki_square_16x16.png";
    font_flags = TCOD_FONT_LAYOUT_ASCII_INROW;
    font_char_horiz = 16;
    font_char_vertic = 16;

    console_width = 80;
    console_height = 50;
    fullscreen = false;
    console_renderer = TCOD_RENDERER_SDL2;
}

void config_save(void)
{
    // TODO: save any variables that changed to the file
}
