#include "config.h"

#include <stdio.h>

int console_width;
int console_height;

void config_load(void)
{
    const char *config_file = "data/config.cfg";
    if (TCOD_sys_file_exists("data/config.local.cfg"))
    {
        config_file = "data/config.local.cfg";
    }
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

    TCOD_parser_delete(parser);
}
