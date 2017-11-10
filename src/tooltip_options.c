#include <libtcod.h>

#include "game.h"

void tooltip_options_add(char *text, void (*fn)(tooltip_data_t data), tooltip_data_t data)
{
    tooltip_option_t *option = (tooltip_option_t *)malloc(sizeof(tooltip_option_t));

    option->text = text;
    option->fn = fn;
    option->data = data;

    TCOD_list_push(tooltip_options, option);
}

void tooltip_options_clear()
{
    for (void **i = TCOD_list_begin(tooltip_options); i != TCOD_list_end(tooltip_options); i++)
    {
        tooltip_option_t *option = *i;

        i = TCOD_list_remove_iterator(tooltip_options, i);

        free(option);
    }
}

void tooltip_move(tooltip_data_t data)
{
    interactions_t interactions = {
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_light_on(tooltip_data_t data)
{
    interactions_t interactions = {
        .light_on = true,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_light_off(tooltip_data_t data)
{
    interactions_t interactions = {
        .light_on = false,
        .light_off = true,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_take_item(tooltip_data_t data)
{
    interactions_t interactions = {
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = true,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_take_items(tooltip_data_t data)
{
    interactions_t interactions = {
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = true};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}

void tooltip_drop_item(tooltip_data_t data)
{
    tile_t *tile = &player->map->tiles[player->x][player->y];

    TCOD_list_remove(player->items, data.item);
    TCOD_list_push(tile->items, data.item);
}

void tooltip_attack(tooltip_data_t data)
{
    interactions_t interactions = {
        .light_on = false,
        .light_off = false,
        .attack = true,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.tile_x, data.tile_y, interactions);
}