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

    light_common.glyph = '*';

    item_info[ITEM_TYPE_GOLD] = (item_info_t){
        .name = "Gold",
        .glyph = '$',
        .color = TCOD_gold,
        .identified = true,
        .max_stack = 1000};
    item_info[ITEM_TYPE_SWORD_LONG] = (item_info_t){
        .name = "Longsword",
        .glyph = '|',
        .color = TCOD_white,
        .identified = true,
        .max_stack = 1};
    item_info[ITEM_TYPE_SHIELD_LARGE] = (item_info_t){
        .name = "Large Shield",
        .glyph = ')',
        .color = TCOD_white,
        .max_stack = 1};
    item_info[ITEM_TYPE_POTION_HEALING] = (item_info_t){
        .name = "Healing Potion",
        .glyph = '!',
        .color = TCOD_red,
        .identified = true,
        .max_stack = 10};
    item_info[ITEM_TYPE_POTION_MANA] = (item_info_t){
        .name = "Mana Potion",
        .glyph = '!',
        .color = TCOD_blue,
        .identified = true,
        .max_stack = 10};

    spell_info[SPELL_INSTAKILL] = (spell_info_t){
        .name = "Instakill",
        .cast = &spell_instakill};

    actor_info[ACTOR_TYPE_WARRIOR] = (actor_info_t){
        .name = "Warrior",
        .glyph = '@',
        .color = TCOD_white,
        .energy_per_turn = 0.5f};
    actor_info[ACTOR_TYPE_MAGE] = (actor_info_t){
        .name = "Mage",
        .glyph = '@',
        .color = TCOD_white,
        .energy_per_turn = 0.5f};
    actor_info[ACTOR_TYPE_ROGUE] = (actor_info_t){
        .name = "Rogue",
        .glyph = '@',
        .color = TCOD_white,
        .energy_per_turn = 0.5f};
    actor_info[ACTOR_TYPE_DOG] = (actor_info_t){
        .name = "Dog",
        .glyph = 'd',
        .color = TCOD_white,
        .energy_per_turn = 0.75f};
    actor_info[ACTOR_TYPE_SKELETON] = (actor_info_t){
        .name = "Skeleton",
        .glyph = 's',
        .color = TCOD_white,
        .energy_per_turn = 0.5f};
    actor_info[ACTOR_TYPE_SKELETON_CAPTAIN] = (actor_info_t){
        .name = "Skeleton Captain",
        .glyph = 'S',
        .color = TCOD_white,
        .energy_per_turn = 0.75f};
    actor_info[ACTOR_TYPE_ZOMBIE] = (actor_info_t){
        .name = "Zombie",
        .glyph = 'z',
        .color = TCOD_dark_green,
        .energy_per_turn = 0.25f};
    actor_info[ACTOR_TYPE_JACKAL] = (actor_info_t){
        .name = "Jackal",
        .glyph = 'j',
        .color = TCOD_dark_orange,
        .energy_per_turn = 1.5f};

    actor_light_info[ACTOR_LIGHT_TYPE_NONE] = (actor_light_info_t){
        .radius = 1,
        .color = TCOD_white};
    actor_light_info[ACTOR_LIGHT_TYPE_GLOW] = (actor_light_info_t){
        .radius = 5,
        .color = TCOD_white};
    actor_light_info[ACTOR_LIGHT_TYPE_TORCH] = (actor_light_info_t){
        .radius = 10,
        .color = TCOD_light_amber};
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