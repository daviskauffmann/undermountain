#include <stdio.h>
#include <stdlib.h>
#include <libtcod.h>

#include "world.h"
#include "config.h"
#include "game.h"

void world_initialize(void)
{
    maps = TCOD_list_new();

    tile_color_light = TCOD_white;
    tile_color_dark = TCOD_darkest_gray;
    torch_color = TCOD_light_amber;

    tile_info[TILE_EMPTY].glyph = ' ';
    tile_info[TILE_EMPTY].is_transparent = true;
    tile_info[TILE_EMPTY].is_walkable = true;

    tile_info[TILE_FLOOR].glyph = '.';
    tile_info[TILE_FLOOR].is_transparent = true;
    tile_info[TILE_FLOOR].is_walkable = true;

    tile_info[TILE_WALL].glyph = '#';
    tile_info[TILE_WALL].is_transparent = false;
    tile_info[TILE_WALL].is_walkable = false;

    tile_info[TILE_STAIR_DOWN].glyph = '>';
    tile_info[TILE_STAIR_DOWN].is_transparent = true;
    tile_info[TILE_STAIR_DOWN].is_walkable = true;

    tile_info[TILE_STAIR_UP].glyph = '<';
    tile_info[TILE_STAIR_UP].is_transparent = true;
    tile_info[TILE_STAIR_UP].is_walkable = true;
}

void world_turn(void)
{
#if SIMULATE_ALL_MAPS
    for (map_t **iterator = (map_t **)TCOD_list_begin(maps);
         iterator != (map_t **)TCOD_list_end(maps);
         iterator++)
    {
        map_t *map = *iterator;

        map_turn(map);
    }
#else
    map_turn(player->map);
#endif
}

void world_tick(void)
{
}

void world_finalize(void)
{
    for (map_t **iterator = (map_t **)TCOD_list_begin(maps);
         iterator != (map_t **)TCOD_list_end(maps);
         iterator++)
    {
        map_t *map = *iterator;

        map_destroy(map);
    }

    TCOD_list_clear_and_delete(maps);
}