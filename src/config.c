#include "config.h"

#include <stdio.h>
#include <string.h>

int console_width;
int console_height;

const char *tileset_filename;
int tileset_columns;
int tileset_rows;
const int *tileset_charmap;

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

    const TCOD_parser_struct_t tileset_struct = TCOD_parser_new_struct(parser, "tileset");
    TCOD_struct_add_property(tileset_struct, "filename", TCOD_TYPE_STRING, true);
    TCOD_struct_add_property(tileset_struct, "columns", TCOD_TYPE_INT, true);
    TCOD_struct_add_property(tileset_struct, "rows", TCOD_TYPE_INT, true);
    TCOD_struct_add_property(tileset_struct, "charmap", TCOD_TYPE_STRING, true);

    TCOD_parser_run(parser, config_file, NULL);

    console_width = TCOD_parser_get_int_property(parser, "console.columns");
    console_height = TCOD_parser_get_int_property(parser, "console.rows");

    tileset_filename = TCOD_parser_get_string_property(parser, "tileset.filename");
    tileset_columns = TCOD_parser_get_int_property(parser, "tileset.columns");
    tileset_rows = TCOD_parser_get_int_property(parser, "tileset.rows");
    const char *tileset_charmap_str = TCOD_parser_get_string_property(parser, "tileset.charmap");
    if (strcmp(tileset_charmap_str, "cp437") == 0)
    {
        tileset_charmap = TCOD_CHARMAP_CP437;
    }
    else if (strcmp(tileset_charmap_str, "tcod") == 0)
    {
        tileset_charmap = TCOD_CHARMAP_TCOD;
    }

    TCOD_parser_delete(parser);
}
