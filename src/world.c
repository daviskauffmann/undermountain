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
    for (void **i = TCOD_list_begin(maps); i != TCOD_list_end(maps); i++)
    {
        map_t *map = *i;

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
    for (void **i = TCOD_list_begin(maps); i != TCOD_list_end(maps); i++)
    {
        map_t *map = *i;

        map_destroy(map);
    }

    TCOD_list_clear_and_delete(maps);
}