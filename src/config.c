#include <stdio.h>
#include <libtcod.h>

#include "config.h"
#include "world.h"

// bool parser_new_struct(TCOD_parser_struct_t str, const char *name)
// {
//     printf("new structure type '%s' with name '%s'\n", TCOD_struct_get_name(str), name ? name : "NULL");

//     return true;
// }

// bool parser_flag(const char *name)
// {
//     printf("found new flag '%s'\n", name);

//     return true;
// }

// bool parser_property(const char *name, TCOD_value_type_t type, TCOD_value_t value)
// {
//     printf("found new property '%s'\n", name);

//     return true;
// }

// bool parser_end_struct(TCOD_parser_struct_t str, const char *name)
// {
//     printf("end of structure type '%s'\n", name);

//     return true;
// }

// void parser_error(const char *msg)
// {
//     fprintf(stderr, msg);

//     exit(1);
// }

// TODO: get all this information from external config file
void config_init(void)
{
    TCOD_console_set_custom_font("tiles.png", TCOD_FONT_LAYOUT_ASCII_INROW, 16, 16);
    TCOD_console_init_root(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE, false, TCOD_RENDERER_SDL);
    TCOD_sys_set_fps(60);
    TCOD_console_set_default_background(NULL, TCOD_black);
    TCOD_console_set_default_foreground(NULL, TCOD_white);

    // TCOD_parser_t parser = TCOD_parser_new();

    // TCOD_parser_struct_t tile = TCOD_parser_new_struct(parser, "tile");
    // TCOD_struct_add_property(tile, "glyph", TCOD_TYPE_CHAR, true);
    // TCOD_struct_add_property(tile, "col", TCOD_TYPE_COLOR, true);
    // TCOD_struct_add_property(tile, "is_transparent", TCOD_TYPE_BOOL, true);
    // TCOD_struct_add_property(tile, "is_walkable", TCOD_TYPE_BOOL, true);

    // TCOD_parser_struct_t actor = TCOD_parser_new_struct(parser, "actor");
    // TCOD_struct_add_property(actor, "glyph", TCOD_TYPE_CHAR, true);
    // TCOD_struct_add_property(actor, "col", TCOD_TYPE_COLOR, true);
    // TCOD_struct_add_property(actor, "sight_radius", TCOD_TYPE_INT, true);

    // TCOD_parser_listener_t listener = {
    //     parser_new_struct,
    //     parser_flag,
    //     parser_property,
    //     parser_end_struct,
    //     parser_error};

    // TCOD_parser_run(parser, "config.txt", &listener);
    // TCOD_parser_delete(parser);

    tile_info[TILETYPE_EMPTY].glyph = ' ';
    tile_info[TILETYPE_EMPTY].light_color = TCOD_white;
    tile_info[TILETYPE_EMPTY].dark_color = TCOD_darkest_gray;
    tile_info[TILETYPE_EMPTY].is_transparent = true;
    tile_info[TILETYPE_EMPTY].is_walkable = true;

    tile_info[TILETYPE_FLOOR].glyph = '.';
    tile_info[TILETYPE_FLOOR].light_color = TCOD_white;
    tile_info[TILETYPE_FLOOR].dark_color = TCOD_darkest_gray;
    tile_info[TILETYPE_FLOOR].is_transparent = true;
    tile_info[TILETYPE_FLOOR].is_walkable = true;

    tile_info[TILETYPE_WALL].glyph = '#';
    tile_info[TILETYPE_WALL].light_color = TCOD_white;
    tile_info[TILETYPE_WALL].dark_color = TCOD_darkest_gray;
    tile_info[TILETYPE_WALL].is_transparent = false;
    tile_info[TILETYPE_WALL].is_walkable = false;

    tile_info[TILETYPE_STAIR_DOWN].glyph = '>';
    tile_info[TILETYPE_STAIR_DOWN].light_color = TCOD_white;
    tile_info[TILETYPE_STAIR_DOWN].dark_color = TCOD_darkest_gray;
    tile_info[TILETYPE_STAIR_DOWN].is_transparent = true;
    tile_info[TILETYPE_STAIR_DOWN].is_walkable = true;

    tile_info[TILETYPE_STAIR_UP].glyph = '<';
    tile_info[TILETYPE_STAIR_UP].light_color = TCOD_white;
    tile_info[TILETYPE_STAIR_UP].dark_color = TCOD_darkest_gray;
    tile_info[TILETYPE_STAIR_UP].is_transparent = true;
    tile_info[TILETYPE_STAIR_UP].is_walkable = true;

    actor_info[ACTORTYPE_PLAYER].glyph = '@';
    actor_info[ACTORTYPE_PLAYER].color = TCOD_white;
    actor_info[ACTORTYPE_PLAYER].sight_radius = 5;

    actor_info[ACTORTYPE_MONSTER].glyph = '@';
    actor_info[ACTORTYPE_MONSTER].color = TCOD_red;
    actor_info[ACTORTYPE_MONSTER].sight_radius = 5;
}