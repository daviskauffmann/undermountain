#include <stdio.h>
#include <libtcod.h>

#include "world.h"
#include "map.h"

tileinfo_t tileinfo[NB_TILETYPES];
TCOD_list_t maps;
uint8_t current_map_index;
map_t *current_map;
actor_t *player;

void world_init(void)
{
    // TODO: move this to a config file
    tileinfo[TILETYPE_EMPTY].glyph = ' ';
    tileinfo[TILETYPE_EMPTY].color = TCOD_white;
    tileinfo[TILETYPE_EMPTY].is_transparent = true;
    tileinfo[TILETYPE_EMPTY].is_walkable = true;

    tileinfo[TILETYPE_FLOOR].glyph = '.';
    tileinfo[TILETYPE_FLOOR].color = TCOD_white;
    tileinfo[TILETYPE_FLOOR].is_transparent = true;
    tileinfo[TILETYPE_FLOOR].is_walkable = true;

    tileinfo[TILETYPE_WALL].glyph = '#';
    tileinfo[TILETYPE_WALL].color = TCOD_white;
    tileinfo[TILETYPE_WALL].is_transparent = false;
    tileinfo[TILETYPE_WALL].is_walkable = false;

    tileinfo[TILETYPE_STAIR_DOWN].glyph = '>';
    tileinfo[TILETYPE_STAIR_DOWN].color = TCOD_white;
    tileinfo[TILETYPE_STAIR_DOWN].is_transparent = true;
    tileinfo[TILETYPE_STAIR_DOWN].is_walkable = true;

    tileinfo[TILETYPE_STAIR_UP].glyph = '<';
    tileinfo[TILETYPE_STAIR_UP].color = TCOD_white;
    tileinfo[TILETYPE_STAIR_UP].is_transparent = true;
    tileinfo[TILETYPE_STAIR_UP].is_walkable = true;

    maps = TCOD_list_new();
    TCOD_list_push(maps, map_create());
    current_map_index = 0;
    current_map = TCOD_list_get(maps, current_map_index);
    player = actor_create(current_map, MAP_WIDTH / 2, MAP_HEIGHT / 2, '@', TCOD_white, 10);
}

void world_save(void)
{
    for (map_t **iterator = (map_t **)TCOD_list_begin(maps);
         iterator != (map_t **)TCOD_list_end(maps);
         iterator++)
    {
        map_t *map = *iterator;

        for (actor_t **iterator = (actor_t **)TCOD_list_begin(map->actors);
             iterator != (actor_t **)TCOD_list_end(map->actors);
             iterator++)
        {
            actor_t *actor = *iterator;
        }
    }
}

void world_load(void)
{
    // load the maps list from file

    // clear out existing maps
    // first add all actors to the map actor list
    // then add all maps to the maps list

    // the player and current map pointers will need to be updated
    // just loop through all maps and actors, and if actor->is_player == true
    // update *map and *player to the current iteration
}

void world_destroy(void)
{
    for (map_t **iterator = (map_t **)TCOD_list_begin(maps);
         iterator != (map_t **)TCOD_list_end(maps);
         iterator++)
    {
        map_t *map = *iterator;

        TCOD_list_clear_and_delete(map->actors);
    }

    TCOD_list_clear_and_delete(maps);
}