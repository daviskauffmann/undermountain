#include <libtcod.h>
#include <stdio.h>

#include "CMemLeak.h"
#include "system.h"
#include "game.h"

#define AUTOMOVE_DELAY 0.1f

static spell_t *target_spell;

static game_input_t tooltip_option_move(tooltip_data_t data);
static game_input_t tooltip_option_stair_descend(tooltip_data_t data);
static game_input_t tooltip_option_stair_ascend(tooltip_data_t data);
static game_input_t tooltip_option_light_on(tooltip_data_t data);
static game_input_t tooltip_option_light_off(tooltip_data_t data);
static game_input_t tooltip_option_item_take(tooltip_data_t data);
static game_input_t tooltip_option_item_take_all(tooltip_data_t data);
static game_input_t tooltip_option_item_drop(tooltip_data_t data);
static game_input_t tooltip_option_item_potion_quaff(tooltip_data_t data);
static game_input_t tooltip_option_spell_ready(tooltip_data_t data);
static game_input_t tooltip_option_spell_cast_ready(tooltip_data_t data);
static game_input_t tooltip_option_spell_cast_spellbook(tooltip_data_t data);
static game_input_t tooltip_option_actor_attack(tooltip_data_t data);

void input_init(void)
{
    target_spell = NULL;
}

game_input_t input_handle(void)
{
    game_input_t input = GAME_INPUT_TICK;

    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    mouse_x = mouse.cx;
    mouse_y = mouse.cy;
    mouse_tile_x = mouse.cx + view_x;
    mouse_tile_y = mouse.cy + view_y;

    switch (ev)
    {
    case TCOD_EVENT_MOUSE_PRESS:
    {
        if (mouse.lbutton)
        {
            if (tooltip_visible)
            {
                if (tooltip_is_inside(mouse_x, mouse_y))
                {
                    tooltip_option_t *selected = NULL;

                    int y = 1;
                    for (void **i = TCOD_list_begin(tooltip_options); i != TCOD_list_end(tooltip_options); i++)
                    {
                        tooltip_option_t *option = *i;

                        if (mouse_x > tooltip_x && mouse_x < tooltip_x + strlen(option->text) + 1 && mouse_y == y + tooltip_y)
                        {
                            selected = option;

                            break;
                        }

                        y++;
                    }

                    if (selected != NULL)
                    {
                        input = selected->fn(selected->data);

                        tooltip_hide();
                    }
                }
                else
                {
                    input = GAME_INPUT_DRAW;

                    tooltip_hide();
                }
            }
            else
            {
                if (view_is_inside(mouse_x, mouse_y) && map_is_inside(mouse_tile_x, mouse_tile_y))
                {
                    if (target_spell != NULL)
                    {
                        input = GAME_INPUT_TURN;

                        cast_data_t data = {
                            .caster = player,
                            .x = mouse_tile_x,
                            .y = mouse_tile_y};

                        target_spell->cast(data);

                        target_spell = NULL;
                    }
                    else
                    {
                        if (TCOD_map_is_in_fov(player->fov_map, mouse_tile_x, mouse_tile_y))
                        {
                            actor_target_set(player, mouse_tile_x, mouse_tile_y, INTERACTIONS_ALL);
                        }
                        else
                        {
                            actor_target_set(player, mouse_tile_x, mouse_tile_y, INTERACTIONS_NONE);
                        }
                    }
                }
            }
        }
        else if (mouse.rbutton)
        {
            if (view_is_inside(mouse_x, mouse_y) && map_is_inside(mouse_tile_x, mouse_tile_y))
            {
                input = GAME_INPUT_DRAW;

                tooltip_show(mouse_x, mouse_y);

                tile_t *tile = &player->map->tiles[mouse_tile_x][mouse_tile_y];

                tooltip_data_t data = {
                    .x = mouse_tile_x,
                    .y = mouse_tile_y,
                    .item = TCOD_list_peek(tile->items)};

                tooltip_options_add("Move", &tooltip_option_move, data);

                if (player->spell_ready != NULL)
                {
                    tooltip_options_add("Cast", &tooltip_option_spell_cast_ready, data);
                }

                if (tile->type == TILE_TYPE_STAIR_DOWN)
                {
                    tooltip_options_add("Descend", &tooltip_option_stair_descend, data);
                }

                if (tile->type == TILE_TYPE_STAIR_UP)
                {
                    tooltip_options_add("Ascend", &tooltip_option_stair_ascend, data);
                }

                if (TCOD_map_is_in_fov(player->fov_map, mouse_tile_x, mouse_tile_y))
                {
                    if (tile->light != NULL)
                    {
                        if (tile->light->on)
                        {
                            tooltip_options_add("Turn Off", &tooltip_option_light_off, data);
                        }
                        else
                        {
                            tooltip_options_add("Turn On", &tooltip_option_light_on, data);
                        }
                    }

                    if (tile->actor != NULL)
                    {
                        if (tile->actor->ai == ai_monster)
                        {
                            tooltip_options_add("Attack", &tooltip_option_actor_attack, data);
                        }
                    }

                    if (TCOD_list_peek(tile->items))
                    {
                        tooltip_options_add("Take Item", &tooltip_option_item_take, data);
                    }

                    if (TCOD_list_size(tile->items) > 1)
                    {
                        tooltip_options_add("Take All", &tooltip_option_item_take_all, data);
                    }
                }
            }
            else if (panel_is_inside(mouse_x, mouse_y))
            {
                switch (content)
                {
                case CONTENT_INVENTORY:
                {
                    item_t *selected = NULL;

                    int y = 1;
                    for (void **i = TCOD_list_begin(player->items); i != TCOD_list_end(player->items); i++)
                    {
                        item_t *item = *i;

                        if (mouse_x > panel_x && mouse_x < panel_x + strlen(item->name) + 1 && mouse_y == y + panel_y - content_scroll[content])
                        {
                            selected = item;

                            break;
                        }

                        y++;
                    }

                    if (selected != NULL)
                    {
                        input = GAME_INPUT_DRAW;

                        tooltip_show(panel_x + 1, y - content_scroll[content]);

                        tooltip_data_t data = {
                            .item = selected};

                        tooltip_options_add("Drop", &tooltip_option_item_drop, data);

                        if (selected->type == ITEM_TYPE_POTION)
                        {
                            tooltip_options_add("Quaff", &tooltip_option_item_potion_quaff, data);
                        }
                    }

                    break;
                }
                case CONTENT_SPELLBOOK:
                {
                    spell_t *selected = NULL;

                    int y = 1;
                    for (void **i = TCOD_list_begin(player->spells); i != TCOD_list_end(player->spells); i++)
                    {
                        spell_t *spell = *i;

                        if (mouse_x > panel_x && mouse_x < panel_x + strlen(spell->name) + 1 && mouse_y == y + panel_y - content_scroll[content])
                        {
                            selected = spell;

                            break;
                        }

                        y++;
                    }

                    if (selected != NULL)
                    {
                        input = GAME_INPUT_DRAW;

                        tooltip_show(panel_x + 1, y - content_scroll[content]);

                        tooltip_data_t data = {
                            .spell = selected};

                        tooltip_options_add("Cast", &tooltip_option_spell_cast_spellbook, data);

                        if (selected != player->spell_ready)
                        {
                            tooltip_options_add("Ready", &tooltip_option_spell_ready, data);
                        }
                    }

                    break;
                }
                }
            }
        }
        else if (mouse.wheel_down)
        {
            if (msg_is_inside(mouse_x, mouse_y))
            {
                // scroll message log down
            }
            else if (panel_is_inside(mouse_x, mouse_y))
            {
                input = GAME_INPUT_DRAW;

                panel_content_scroll_down();
            }
        }
        else if (mouse.wheel_up)
        {
            if (msg_is_inside(mouse_x, mouse_y))
            {
                // scroll message log down
            }
            else if (panel_is_inside(mouse_x, mouse_y))
            {
                input = GAME_INPUT_DRAW;

                panel_content_scroll_up();
            }
        }

        break;
    }
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            input = GAME_INPUT_QUIT;

            break;
        }
        case TCODK_CHAR:
        {
            switch (key.c)
            {
            case 'b':
            {
                input = GAME_INPUT_DRAW;

                panel_toggle(CONTENT_SPELLBOOK);

                break;
            }
            case 'c':
            {
                input = GAME_INPUT_DRAW;

                panel_toggle(CONTENT_CHARACTER);

                break;
            }
            case 'i':
            {
                input = GAME_INPUT_DRAW;

                panel_toggle(CONTENT_INVENTORY);

                break;
            }
            case 'l':
            {
                if (key.lctrl)
                {
                    input = GAME_INPUT_DRAW;

                    game_uninit();
                    game_init();
                    game_load();
                }
                else
                {
                    input = GAME_INPUT_DRAW;

                    msg_visible = !msg_visible;
                }

                break;
            }
            case 'r':
            {
                input = GAME_INPUT_DRAW;

                game_uninit();
                game_init();
                game_new();

                break;
            }
            case 's':
            {
                if (key.lctrl)
                {
                    input = GAME_INPUT_DRAW;

                    game_save();
                }

                break;
            }
            case 't':
            {
                input = GAME_INPUT_TURN;

                if (player->light != ACTOR_LIGHT_TORCH)
                {
                    player->light = ACTOR_LIGHT_TORCH;
                }
                else
                {
                    player->light = ACTOR_LIGHT_DEFAULT;
                }

                actor_calc_fov(player);

                break;
            }
            }

            break;
        }
        }

        break;
    }
    }

    static float automove_timer = 0.0f;

    if (player->target)
    {
        if (automove_timer >= AUTOMOVE_DELAY)
        {
            input = GAME_INPUT_TURN;

            actor_target_process(player);

            automove_timer = 0.0f;
        }
        else
        {
            automove_timer += TCOD_sys_get_last_frame_length();
        }
    }

    return input;
}

static game_input_t tooltip_option_move(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.x, data.y, interactions);

    return GAME_INPUT_TICK;
}

static game_input_t tooltip_option_stair_descend(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = true,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.x, data.y, interactions);

    return GAME_INPUT_TICK;
}

static game_input_t tooltip_option_stair_ascend(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = true,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.x, data.y, interactions);

    return GAME_INPUT_TICK;
}

static game_input_t tooltip_option_light_on(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = true,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.x, data.y, interactions);

    return GAME_INPUT_TICK;
}

static game_input_t tooltip_option_light_off(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = true,
        .attack = false,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.x, data.y, interactions);

    return GAME_INPUT_TICK;
}

static game_input_t tooltip_option_item_take(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = true,
        .take_items = false};

    actor_target_set(player, data.x, data.y, interactions);

    return GAME_INPUT_TICK;
}

static game_input_t tooltip_option_item_take_all(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = false,
        .take_item = false,
        .take_items = true};

    actor_target_set(player, data.x, data.y, interactions);

    return GAME_INPUT_TICK;
}

static game_input_t tooltip_option_item_potion_quaff(tooltip_data_t data)
{
    return GAME_INPUT_TURN;
}

static game_input_t tooltip_option_item_drop(tooltip_data_t data)
{
    tile_t *tile = &player->map->tiles[player->x][player->y];

    TCOD_list_remove(player->items, data.item);
    TCOD_list_push(tile->items, data.item);

    return GAME_INPUT_TURN;
}

static game_input_t tooltip_option_spell_ready(tooltip_data_t data)
{
    player->spell_ready = data.spell;

    return GAME_INPUT_DRAW;
}

static game_input_t tooltip_option_spell_cast_ready(tooltip_data_t data)
{
    cast_data_t cast_data = {
        .caster = player,
        .x = data.x,
        .y = data.y};

    player->spell_ready->cast(cast_data);

    return GAME_INPUT_TURN;
}

static game_input_t tooltip_option_spell_cast_spellbook(tooltip_data_t data)
{
    // msg_log("select a target", player->map, player->x, player->y);

    target_spell = data.spell;

    return GAME_INPUT_DRAW;
}

static game_input_t tooltip_option_actor_attack(tooltip_data_t data)
{
    interactions_t interactions = {
        .descend = false,
        .ascend = false,
        .light_on = false,
        .light_off = false,
        .attack = true,
        .take_item = false,
        .take_items = false};

    actor_target_set(player, data.x, data.y, interactions);

    return GAME_INPUT_TICK;
}

void input_uninit(void)
{
}