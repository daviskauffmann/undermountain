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

    const TCOD_parser_t parser = TCOD_parser_new();
    const TCOD_parser_struct_t console_struct = TCOD_parser_new_struct(parser, "console");
    TCOD_struct_add_property(console_struct, "columns", TCOD_TYPE_INT, true);
    TCOD_struct_add_property(console_struct, "rows", TCOD_TYPE_INT, true);
    TCOD_parser_run(parser, config_file, NULL);

    console_width = TCOD_parser_get_int_property(parser, "console.columns");
    console_height = TCOD_parser_get_int_property(parser, "console.rows");

    TCOD_parser_delete(parser);
}
