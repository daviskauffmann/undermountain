#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"
#include "system.h"

void game_init(void)
{
    world_init();
    input_init();
    gfx_init();
}

void game_new()
{
    map_t *map = map_create(0);

    TCOD_list_push(maps, map);

    player = actor_create(map, map->stair_up_x, map->stair_up_y, '@', TCOD_white, NULL);

    TCOD_list_push(map->actors, player);
    map->tiles[player->x][player->y].actor = player;

    for (int i = 0; i < 30; i++)
    {
        item_t *item = item_create_random(player->x, player->y);

        TCOD_list_push(map->items, item);
        TCOD_list_push(player->items, item);
    }

    turn = 0;

    msg_log("Hail, Player!", player->map, player->x, player->y);

    actor_t *pet = actor_create(map, player->x + 1, player->y, '@', TCOD_yellow, &ai_pet);

    TCOD_list_push(map->actors, pet);
    map->tiles[pet->x][pet->y].actor = pet;
}

void game_turn(void)
{
    world_turn();

    turn++;
}

void game_tick(void)
{
    world_tick();
}

void game_save(void)
{
}

void game_load(void)
{
}

void game_draw_turn(void)
{
    gfx_draw_turn();
}

void game_draw_tick(void)
{
    gfx_draw_tick();
}

void game_uninit(void)
{
    world_uninit();
    input_uninit();
    gfx_uninit();
}