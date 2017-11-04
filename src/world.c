#include <libtcod.h>

#include "world.h"
#include "map.h"

void world_initialize(void)
{
    maps = TCOD_list_new();
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