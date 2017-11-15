#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

#define SIMULATE_ALL_MAPS 1

void world_init(void)
{
    maps = TCOD_list_new();

    tile_info[TILE_TYPE_EMPTY] = (tile_info_t){
        .glyph = ' ',
        .is_transparent = true,
        .is_walkable = true};
    tile_info[TILE_TYPE_FLOOR] = (tile_info_t){
        .glyph = '.',
        .is_transparent = true,
        .is_walkable = true};
    tile_info[TILE_TYPE_WALL] = (tile_info_t){
        .glyph = '#',
        .is_transparent = false,
        .is_walkable = false};
    tile_info[TILE_TYPE_STAIR_DOWN] = (tile_info_t){
        .glyph = '>',
        .is_transparent = true,
        .is_walkable = true};
    tile_info[TILE_TYPE_STAIR_UP] = (tile_info_t){
        .glyph = '<',
        .is_transparent = true,
        .is_walkable = true};

    spell[SPELL_INSTAKILL].name = "Instakill";
    spell[SPELL_INSTAKILL].cast = &spell_instakill;

    actor_name[ACTOR_TYPE_WARRIOR] = "Warrior";
    actor_name[ACTOR_TYPE_MAGE] = "Mage";
    actor_name[ACTOR_TYPE_ROGUE] = "Rogue";
    actor_name[ACTOR_TYPE_DOG] = "Dog";
    actor_name[ACTOR_TYPE_SKELETON] = "Skeleton";
    actor_name[ACTOR_TYPE_SKELETON_CAPTAIN] = "Skeleton Captain";
    actor_name[ACTOR_TYPE_ZOMBIE] = "Zombie";

    actor_glyph[ACTOR_TYPE_WARRIOR] = '@';
    actor_glyph[ACTOR_TYPE_MAGE] = '@';
    actor_glyph[ACTOR_TYPE_ROGUE] = '@';
    actor_glyph[ACTOR_TYPE_DOG] = 'd';
    actor_glyph[ACTOR_TYPE_SKELETON] = 's';
    actor_glyph[ACTOR_TYPE_SKELETON_CAPTAIN] = 'S';
    actor_glyph[ACTOR_TYPE_ZOMBIE] = 'z';

    actor_color[ACTOR_TYPE_WARRIOR] = TCOD_white;
    actor_color[ACTOR_TYPE_MAGE] = TCOD_white;
    actor_color[ACTOR_TYPE_ROGUE] = TCOD_white;
    actor_color[ACTOR_TYPE_DOG] = TCOD_white;
    actor_color[ACTOR_TYPE_SKELETON] = TCOD_white;
    actor_color[ACTOR_TYPE_SKELETON_CAPTAIN] = TCOD_white;
    actor_color[ACTOR_TYPE_ZOMBIE] = TCOD_green;

    actor_energy_per_turn[ACTOR_TYPE_WARRIOR] = 0.5f;
    actor_energy_per_turn[ACTOR_TYPE_MAGE] = 0.5f;
    actor_energy_per_turn[ACTOR_TYPE_ROGUE] = 0.5f;
    actor_energy_per_turn[ACTOR_TYPE_DOG] = 0.75f;
    actor_energy_per_turn[ACTOR_TYPE_SKELETON] = 0.5f;
    actor_energy_per_turn[ACTOR_TYPE_SKELETON_CAPTAIN] = 0.75f;
    actor_energy_per_turn[ACTOR_TYPE_ZOMBIE] = 0.25f;

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