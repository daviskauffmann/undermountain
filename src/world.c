#include <stdio.h>
#include <stdint.h>
#include <libtcod.h>

#include "config.h"
#include "world.h"

world_t *world_create(void)
{
    world_t *world = (world_t *)malloc(sizeof(world_t));

    world->maps = TCOD_list_new();
    world->current_map_index = 0;
    world->current_map = map_create(world);
    world->player = actor_create(world->current_map, world->current_map->stair_up_x, world->current_map->stair_up_y, '@', TCOD_white, 10);

    return world;
}

void world_destroy(world_t *world)
{
    for (map_t **iterator = (map_t **)TCOD_list_begin(world->maps);
         iterator != (map_t **)TCOD_list_end(world->maps);
         iterator++)
    {
        map_t *map = *iterator;

        TCOD_list_clear_and_delete(map->actors);
    }

    TCOD_list_clear_and_delete(world->maps);
}