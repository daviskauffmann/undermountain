#include <libtcod.h>

#include "game.h"
#include "console.h"
#include "world.h"
#include "actor.h"

void game_initialize(void)
{
    map_t *map = map_create();

    current_map_index = 0;
    player = actor_create(map, '@', TCOD_white, map->stair_up_x, map->stair_up_y, 5);

    console_log("Hail, Player!", player->map, player->x, player->y);
}

void game_save(void)
{
}

void game_load(void)
{
}

void game_finalize(void)
{
}