#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

#define SIMULATE_ALL_MAPS 1

void world_init(void)
{
    maps = TCOD_list_new();

    tile_transparent[TILE_TYPE_EMPTY] = true;
    tile_transparent[TILE_TYPE_FLOOR] = true;
    tile_transparent[TILE_TYPE_WALL] = false;
    tile_transparent[TILE_TYPE_STAIR_DOWN] = true;
    tile_transparent[TILE_TYPE_STAIR_UP] = true;

    tile_walkable[TILE_TYPE_EMPTY] = true;
    tile_walkable[TILE_TYPE_FLOOR] = true;
    tile_walkable[TILE_TYPE_WALL] = false;
    tile_walkable[TILE_TYPE_STAIR_DOWN] = true;
    tile_walkable[TILE_TYPE_STAIR_UP] = true;

    actor_light_radius[ACTOR_LIGHT_NONE] = 1;
    actor_light_radius[ACTOR_LIGHT_DEFAULT] = 5;
    actor_light_radius[ACTOR_LIGHT_TORCH] = 10;
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
#if SIMULATE_ALL_MAPS
    for (void **i = TCOD_list_begin(maps); i != TCOD_list_end(maps); i++)
    {
        map_t *map = *i;

        map_tick(map);
    }
#else
    map_tick(player->map);
#endif
}

void world_draw_turn(void)
{
    TCOD_console_set_default_background(NULL, background_color);
    TCOD_console_set_default_foreground(NULL, foreground_color);
    TCOD_console_clear(NULL);

    map_draw_turn(player->map);
}

void world_draw_tick(void)
{
    map_draw_tick(player->map);

    TCOD_console_set_default_foreground(NULL, foreground_color);
    TCOD_console_print_ex(NULL, 0, 0, TCOD_BKGND_SET, TCOD_LEFT, "Turn: %d", turn);
    TCOD_console_print_ex(NULL, 0, 1, TCOD_BKGND_SET, TCOD_LEFT, "Lvl: %d", player->map->level);
    TCOD_console_print_ex(NULL, 0, 2, TCOD_BKGND_SET, TCOD_LEFT, "Loc: (%d, %d)", player->x, player->y);
    TCOD_console_print_ex(NULL, 0, 3, TCOD_BKGND_SET, TCOD_LEFT, "Tgt: (%d, %d)", mouse_tile_x, mouse_tile_y);
}

void world_uninit(void)
{
    for (void **i = TCOD_list_begin(maps); i != TCOD_list_end(maps); i++)
    {
        map_t *map = *i;

        map_destroy(map);
    }

    TCOD_list_delete(maps);
}