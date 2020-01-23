#include "config.h"

#include <stdio.h>

#include "sys.h"

int console_width;
int console_height;
bool fullscreen;
TCOD_renderer_t console_renderer;

const char *font_file;
TCOD_font_flags_t font_flags;
int font_layout_width;
int font_layout_height;

void config_load(void)
{
    const char *config_file = NULL;
    if (file_exists("data/config.local.txt"))
    {
        config_file = "data/config.local.txt";
    }
    else if (file_exists("data/config.txt"))
    {
        config_file = "data/config.txt";
    }
    if (config_file)
    {
        printf("Parsing %s.\n", config_file);

        TCOD_parser_t parser = TCOD_parser_new();
        TCOD_parser_struct_t config = TCOD_parser_new_struct(parser, "config");
        TCOD_struct_add_property(config, "console_width", TCOD_TYPE_INT, true);
        TCOD_struct_add_property(config, "console_height", TCOD_TYPE_INT, true);
        TCOD_struct_add_property(config, "font_file", TCOD_TYPE_STRING, true);
        TCOD_struct_add_property(config, "font_layout_row", TCOD_TYPE_BOOL, true);
        TCOD_struct_add_property(config, "font_layout_width", TCOD_TYPE_INT, true);
        TCOD_struct_add_property(config, "font_layout_height", TCOD_TYPE_INT, true);
        TCOD_parser_run(parser, config_file, NULL);

        console_width = TCOD_parser_get_int_property(parser, "config.console_width");
        console_height = TCOD_parser_get_int_property(parser, "config.console_height");
        fullscreen = false;
        console_renderer = TCOD_RENDERER_SDL2;

        font_file = TCOD_parser_get_string_property(parser, "config.font_file");
        font_flags = 0;
        if (TCOD_parser_get_bool_property(parser, "config.font_layout_row"))
        {
            font_flags |= TCOD_FONT_LAYOUT_ASCII_INROW;
        }
        else
        {
            font_flags |= TCOD_FONT_LAYOUT_ASCII_INCOL;
        }
        font_layout_width = TCOD_parser_get_int_property(parser, "config.font_layout_width");
        font_layout_height = TCOD_parser_get_int_property(parser, "config.font_layout_height");

        TCOD_parser_delete(parser);
    }
    else
    {
        printf("No config file found, using defaults.\n");

        console_width = 120;
        console_height = 60;
        fullscreen = false;
        console_renderer = TCOD_RENDERER_SDL2;

        font_file = "data/terminal.png";
        font_flags = TCOD_FONT_LAYOUT_ASCII_INCOL;
        font_layout_width = 16;
        font_layout_height = 16;
    }
}
