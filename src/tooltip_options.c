#include <libtcod.h>

#include "game.h"

void tooltip_option_move(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_option_descend(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = true,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_option_ascend(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = true,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_option_light_on(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = true,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_option_light_off(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = true,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_option_take_item(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = true,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_option_take_items(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = true};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_option_drop_item(tooltip_data_t data)
{
    tile_t *tile = &player->map->tiles[player->x][player->y];

    TCOD_list_remove(player->items, data.item);
    TCOD_list_push(tile->items, data.item);
}

void tooltip_option_attack(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = true,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}