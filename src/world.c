#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

#define SIMULATE_ALL_MAPS 1

void world_init(void)
{
    maps = TCOD_list_new();

    tile_glyph[TILE_TYPE_EMPTY] = ' ';
    tile_glyph[TILE_TYPE_FLOOR] = '.';
    tile_glyph[TILE_TYPE_WALL] = '#';
    tile_glyph[TILE_TYPE_STAIR_DOWN] = '>';
    tile_glyph[TILE_TYPE_STAIR_UP] = '<';

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

    spell[SPELL_INSTAKILL].name = "Instakill";
    spell[SPELL_INSTAKILL].cast = &spell_instakill;

    actor_name[ACTOR_TYPE_PLAYER] = "Blinky";
    actor_name[ACTOR_TYPE_MONSTER] = "Monster";
    actor_name[ACTOR_TYPE_PET] = "Pet";

    actor_glyph[ACTOR_TYPE_PLAYER] = '@';
    actor_glyph[ACTOR_TYPE_MONSTER] = '@';
    actor_glyph[ACTOR_TYPE_PET] = '@';

    actor_color[ACTOR_TYPE_PLAYER] = TCOD_white;
    actor_color[ACTOR_TYPE_MONSTER] = TCOD_red;
    actor_color[ACTOR_TYPE_PET] = TCOD_yellow;

    actor_energy_per_turn[ACTOR_TYPE_PLAYER] = 1.0f;
    actor_energy_per_turn[ACTOR_TYPE_MONSTER] = 0.5f;
    actor_energy_per_turn[ACTOR_TYPE_PET] = 0.5f;

    actor_ai[ACTOR_TYPE_PLAYER] = NULL;
    actor_ai[ACTOR_TYPE_MONSTER] = &ai_monster;
    actor_ai[ACTOR_TYPE_PET] = &ai_pet;

    actor_light_radius[ACTOR_LIGHT_NONE] = 1;
    actor_light_radius[ACTOR_LIGHT_DEFAULT] = 5;
    actor_light_radius[ACTOR_LIGHT_TORCH] = 10;

    actor_light_color[ACTOR_LIGHT_NONE] = TCOD_white;
    actor_light_color[ACTOR_LIGHT_DEFAULT] = TCOD_white;
    actor_light_color[ACTOR_LIGHT_TORCH] = TCOD_light_amber;
}

void world_update(void)
{
#if SIMULATE_ALL_MAPS
    for (void **i = TCOD_list_begin(maps); i != TCOD_list_end(maps); i++)
    {
        map_t *map = *i;

        map_update(map);
    }
#else
    map_update(player->map);
#endif
}

void world_draw(void)
{
    TCOD_console_set_default_background(NULL, background_color);
    TCOD_console_set_default_foreground(NULL, foreground_color);
    TCOD_console_clear(NULL);

    map_draw(player->map);
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