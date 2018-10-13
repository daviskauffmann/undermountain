#include <libtcod/libtcod.h>
#include <malloc.h>
#include <math.h>
#include <string.h>

#include "config.h"
#include "engine.h"
#include "game.h"
#include "input.h"
#include "message.h"
#include "projectile.h"
#include "renderer.h"
#include "tooltip_option.h"
#include "ui.h"

#include "CMemleak.h"

struct renderer *renderer_create(void)
{
    struct renderer *renderer = calloc(1, sizeof(struct renderer));

    renderer->noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
    renderer->noise_x = 0.0f;
    renderer->panel = TCOD_console_new(console_width, console_height);
    renderer->message_log = TCOD_console_new(console_width, console_height);
    renderer->tooltip = TCOD_console_new(console_width, console_height);

    return renderer;
}

void renderer_draw(struct renderer *renderer, struct engine *engine, struct game *game, struct ui *ui)
{
    TCOD_console_set_default_background(NULL, TCOD_black);
    TCOD_console_set_default_foreground(NULL, TCOD_white);
    TCOD_console_clear(NULL);

    switch (engine->state)
    {
    case ENGINE_STATE_MENU:
    {
        switch (ui->menu_state)
        {
        case MENU_STATE_MAIN:
        {
            int y = 1;
            for (enum main_menu_option main_menu_option = 0; main_menu_option < NUM_MAIN_MENU_OPTIONS; main_menu_option++)
            {
                struct main_menu_option_info *main_menu_option_info = &ui->main_menu_option_info[main_menu_option];

                TCOD_console_set_default_foreground(NULL, main_menu_option == ui_main_menu_get_selected(ui) ? TCOD_yellow : TCOD_white);
                TCOD_console_print(NULL, 1, y++, "%c) %s", main_menu_option + 97, main_menu_option_info->text);
            }
        }
        break;
        case MENU_STATE_ABOUT:
        {
            int y = 1;

            y += TCOD_console_print_rect(NULL, 1, y, console_width - 2, console_height - 2, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");

            TCOD_console_print_rect(NULL, 1, y + 1, console_width - 2, console_height - 2, "Press ESC to return.");
        }
        break;
        }

        TCOD_console_set_default_foreground(NULL, TCOD_white);
        TCOD_console_print_frame(NULL, 0, 0, console_width, console_height, false, TCOD_BKGND_SET, WINDOW_TITLE);
    }
    break;
    case ENGINE_STATE_PLAYING:
    {
        struct map *map = &game->maps[game->player->level];

        {
            renderer->noise_x += 0.2f;
            float noise_dx = renderer->noise_x + 20.0f;
            float dx = TCOD_noise_get(renderer->noise, &noise_dx) * 0.5f;
            noise_dx += 30.0f;
            float dy = TCOD_noise_get(renderer->noise, &noise_dx) * 0.5f;
            float di = 0.2f * TCOD_noise_get(renderer->noise, &renderer->noise_x);

            for (int x = ui->view_x; x < ui->view_x + ui->view_width; x++)
            {
                for (int y = ui->view_y; y < ui->view_y + ui->view_height; y++)
                {
                    if (map_is_inside(x, y))
                    {
                        struct tile *tile = &map->tiles[x][y];
                        struct tile_info *tile_info = &game->tile_info[tile->type];

                        TCOD_color_t color = game->tile_common.shadow_color;

                        if (TCOD_map_is_in_fov(game->player->fov, x, y))
                        {
                            tile->seen = true;

                            for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
                            {
                                struct actor *actor = *iterator;

                                if (actor->glow_fov && TCOD_map_is_in_fov(actor->glow_fov, x, y))
                                {
                                    float r2 = powf((float)game->actor_common.glow_radius, 2);
                                    float d = powf((float)(x - actor->x), 2) + powf((float)(y - actor->y), 2);
                                    float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

                                    color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, game->actor_common.glow_color, l), l);
                                }
                            }

                            for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
                            {
                                struct object *object = *iterator;

                                if (object->light_fov && TCOD_map_is_in_fov(object->light_fov, x, y))
                                {
                                    float r2 = powf((float)object->light_radius, 2);
                                    float d = powf((float)(x - object->x + (object->light_flicker ? dx : 0)), 2) + powf((float)(y - object->y + (object->light_flicker ? dy : 0)), 2);
                                    float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + (object->light_flicker ? di : 0));

                                    color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, object->light_color, l), l);
                                }
                            }

                            for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
                            {
                                struct actor *actor = *iterator;

                                if (actor->torch_fov && TCOD_map_is_in_fov(actor->torch_fov, x, y))
                                {
                                    float r2 = powf((float)game->actor_common.torch_radius, 2);
                                    float d = powf(x - actor->x + dx, 2) + powf(y - actor->y + dy, 2);
                                    float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + di);

                                    color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, game->actor_common.torch_color, l), l);
                                }
                            }
                        }

                        if (tile->seen)
                        {
                            TCOD_console_set_char_foreground(NULL, x - ui->view_x, y - ui->view_y, color);
                            TCOD_console_set_char(NULL, x - ui->view_x, y - ui->view_y, tile_info->glyph);
                        }
                    }
                }
            }

            for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
            {
                struct actor *actor = *iterator;

                if (actor->dead && TCOD_map_is_in_fov(game->player->fov, actor->x, actor->y))
                {
                    TCOD_console_set_char_foreground(NULL, actor->x - ui->view_x, actor->y - ui->view_y, TCOD_dark_red);
                    TCOD_console_set_char(NULL, actor->x - ui->view_x, actor->y - ui->view_y, '%');
                }
            }

            for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
            {
                struct object *object = *iterator;

                if (TCOD_map_is_in_fov(game->player->fov, object->x, object->y))
                {
                    TCOD_console_set_char_foreground(NULL, object->x - ui->view_x, object->y - ui->view_y, object->color);
                    TCOD_console_set_char(NULL, object->x - ui->view_x, object->y - ui->view_y, game->object_info[object->type].glyph);
                }
            }

            for (void **iterator = TCOD_list_begin(map->items); iterator != TCOD_list_end(map->items); iterator++)
            {
                struct item *item = *iterator;

                if (TCOD_map_is_in_fov(game->player->fov, item->x, item->y))
                {
                    TCOD_console_set_char_foreground(NULL, item->x - ui->view_x, item->y - ui->view_y, game->base_item_info[game->item_info[item->type].base_item].color);
                    TCOD_console_set_char(NULL, item->x - ui->view_x, item->y - ui->view_y, game->base_item_info[game->item_info[item->type].base_item].glyph);
                }
            }

            for (void **iterator = TCOD_list_begin(map->projectiles); iterator != TCOD_list_end(map->projectiles); iterator++)
            {
                struct projectile *projectile = *iterator;

                int x = (int)projectile->x;
                int y = (int)projectile->y;

                if (TCOD_map_is_in_fov(game->player->fov, x, y))
                {
                    TCOD_console_set_char_foreground(NULL, x - ui->view_x, y - ui->view_y, TCOD_white);
                    TCOD_console_set_char(NULL, x - ui->view_x, y - ui->view_y, projectile->glyph);
                }
            }

            for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
            {
                struct object *object = *iterator;

                if ((object->type == OBJECT_TYPE_STAIR_DOWN || object->type == OBJECT_TYPE_STAIR_UP) && TCOD_map_is_in_fov(game->player->fov, object->x, object->y))
                {
                    TCOD_console_set_char_foreground(NULL, object->x - ui->view_x, object->y - ui->view_y, object->color);
                    TCOD_console_set_char(NULL, object->x - ui->view_x, object->y - ui->view_y, game->object_info[object->type].glyph);
                }
            }

            for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
            {
                struct actor *actor = *iterator;

                if (!actor->dead && TCOD_map_is_in_fov(game->player->fov, actor->x, actor->y))
                {
                    TCOD_color_t color = game->class_info[actor->class].color;

                    if (actor->flash_fade > 0)
                    {
                        color = TCOD_color_lerp(color, actor->flash_color, actor->flash_fade);
                    }

                    TCOD_console_set_char_foreground(NULL, actor->x - ui->view_x, actor->y - ui->view_y, color);
                    TCOD_console_set_char(NULL, actor->x - ui->view_x, actor->y - ui->view_y, game->race_info[actor->race].glyph);
                }
            }
        }

        if (ui->targeting != TARGETING_NONE)
        {
            TCOD_console_set_char_foreground(NULL, ui->target_x - ui->view_x, ui->target_y - ui->view_y, TCOD_red);
            TCOD_console_set_char(NULL, ui->target_x - ui->view_x, ui->target_y - ui->view_y, 'X');

            struct tile *tile = &map->tiles[ui->target_x][ui->target_y];

            if (TCOD_map_is_in_fov(game->player->fov, ui->target_x, ui->target_y))
            {
                if (tile->actor)
                {
                    TCOD_console_print_ex(
                        NULL,
                        ui->view_width / 2,
                        ui->view_height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        tile->actor->name);

                    goto done;
                }

                {
                    struct item *item = TCOD_list_peek(tile->items);

                    if (item)
                    {
                        TCOD_console_print_ex(
                            NULL,
                            ui->view_width / 2,
                            ui->view_height - 2,
                            TCOD_BKGND_NONE,
                            TCOD_CENTER,
                            game->item_info[item->type].name);

                        goto done;
                    }
                }

                if (tile->object)
                {
                    TCOD_console_print_ex(
                        NULL,
                        ui->view_width / 2,
                        ui->view_height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        game->object_info[tile->object->type].name);

                    goto done;
                }

                TCOD_console_print_ex(
                    NULL,
                    ui->view_width / 2,
                    ui->view_height - 2,
                    TCOD_BKGND_NONE,
                    TCOD_CENTER,
                    game->tile_info[tile->type].name);

            done:;
            }
            else
            {
                if (tile->seen)
                {
                    TCOD_console_print_ex(
                        NULL,
                        ui->view_width / 2,
                        ui->view_height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        "%s (known)",
                        game->tile_info[tile->type].name);
                }
                else
                {
                    TCOD_console_print_ex(
                        NULL,
                        ui->view_width / 2,
                        ui->view_height - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        "Unknown");
                }
            }
        }

        if (ui->message_log_visible)
        {
            TCOD_console_set_default_background(renderer->message_log, TCOD_black);
            TCOD_console_set_default_foreground(renderer->message_log, TCOD_white);
            TCOD_console_clear(renderer->message_log);

            int y = 1;
            for (void **i = TCOD_list_begin(game->messages); i != TCOD_list_end(game->messages); i++)
            {
                struct message *message = *i;

                TCOD_console_set_default_foreground(renderer->message_log, message->color);
                TCOD_console_print(renderer->message_log, 1, y, message->text);

                y++;
            }

            TCOD_console_set_default_foreground(renderer->message_log, TCOD_white);
            TCOD_console_print_frame(renderer->message_log, 0, 0, ui->message_log_width, ui->message_log_height, false, TCOD_BKGND_SET, "Log");

            TCOD_console_blit(renderer->message_log, 0, 0, ui->message_log_width, ui->message_log_height, NULL, ui->message_log_x, ui->message_log_y, 1, 1);
        }

        if (ui->panel_visible)
        {
            TCOD_console_set_default_background(renderer->panel, TCOD_black);
            TCOD_console_set_default_foreground(renderer->panel, TCOD_white);
            TCOD_console_clear(renderer->panel);

            struct panel_status *panel_status = &ui->panel_status[ui->current_panel];

            switch (ui->current_panel)
            {
            case PANEL_CHARACTER:
            {
                int y = 1;
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "NAME     : %s", game->player->name);
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "ALIGNMENT: Neutral Good");
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "RACE     : %s", game->race_info[game->player->race].name);
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "CLASS    : %s", game->class_info[game->player->class].name);
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "LEVEL    : %d", game->player->class_level);
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "EXP      : %d", game->player->experience);
                y++;
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "STR: %d", game->player->strength);
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "DEX: %d", game->player->dexterity);
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "CON: %d", game->player->constitution);
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "INT: %d", game->player->intelligence);
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "WIS: %d", game->player->wisdom);
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "CHA: %d", game->player->charisma);
                y++;
                for (enum equip_slot equip_slot = EQUIP_SLOT_ARMOR; equip_slot < NUM_EQUIP_SLOTS; equip_slot++)
                {
                    TCOD_console_set_default_foreground(renderer->panel, equip_slot == ui_panel_character_get_selected(ui, game) ? TCOD_yellow : TCOD_white);

                    if (game->player->equipment[equip_slot])
                    {
                        if (panel_status->selection_mode)
                        {
                            TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "%c) %s: %s",
                                               equip_slot + 97 - 1,
                                               game->equip_slot_info[equip_slot].label,
                                               game->item_info[game->player->equipment[equip_slot]->type].name);
                        }
                        else
                        {
                            TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "%s: %s",
                                               game->equip_slot_info[equip_slot].label,
                                               game->item_info[game->player->equipment[equip_slot]->type].name);
                        }
                    }
                    else
                    {
                        if (panel_status->selection_mode)
                        {
                            TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "%c) %s: N/A",
                                               equip_slot + 97 - 1,
                                               game->equip_slot_info[equip_slot].label);
                        }
                        else
                        {
                            TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "%s: N/A",
                                               game->equip_slot_info[equip_slot].label);
                        }
                    }

                    TCOD_console_set_default_foreground(renderer->panel, TCOD_white);
                }
                y++;
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "AC: %d", actor_calc_armor_class(game->player));
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "HP: %d / %d", game->player->current_hp, actor_calc_max_hp(game->player));
                y++;
                int num_dice;
                int die_to_roll;
                int crit_threat;
                int crit_mult;
                actor_calc_weapon(game->player, &num_dice, &die_to_roll, &crit_threat, &crit_mult, false);
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "MELEE: %dd%d (%d-20x%d)", num_dice, die_to_roll, crit_threat, crit_mult);
                actor_calc_weapon(game->player, &num_dice, &die_to_roll, &crit_threat, &crit_mult, true);
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "RANGED: %dd%d (%d-20x%d)", num_dice, die_to_roll, crit_threat, crit_mult);
                y++;
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "ATTACK: +%d", actor_calc_attack_bonus(game->player));
                TCOD_console_print(renderer->panel, 1, y++ - panel_status->scroll, "DAMAGE: +%d", actor_calc_damage_bonus(game->player));

                TCOD_console_set_default_foreground(renderer->panel, TCOD_white);
                TCOD_console_print_frame(renderer->panel, 0, 0, ui->panel_width, ui->panel_height, false, TCOD_BKGND_SET, "Character");
            }
            break;
            case PANEL_EXAMINE:
            {
                TCOD_console_set_default_foreground(renderer->panel, TCOD_white);
                TCOD_console_print_frame(renderer->panel, 0, 0, ui->panel_width, ui->panel_height, false, TCOD_BKGND_SET, "Examine");
            }
            break;
            case PANEL_INVENTORY:
            {
                int y = 1;
                for (void **iterator = TCOD_list_begin(game->player->items); iterator != TCOD_list_end(game->player->items); iterator++)
                {
                    struct item *item = *iterator;

                    TCOD_console_set_default_foreground(renderer->panel, item == ui_panel_inventory_get_selected(ui, game) ? TCOD_yellow : game->base_item_info[game->item_info[item->type].base_item].color);
                    if (panel_status->selection_mode)
                    {
                        TCOD_console_print(renderer->panel, 1, y - panel_status->scroll, "%c) %s", y - 1 + 97 - panel_status->scroll, game->item_info[item->type].name);
                    }
                    else
                    {
                        TCOD_console_print(renderer->panel, 1, y - panel_status->scroll, game->item_info[item->type].name);
                    }

                    y++;
                }

                TCOD_console_set_default_foreground(renderer->panel, TCOD_white);
                TCOD_console_print_frame(renderer->panel, 0, 0, ui->panel_width, ui->panel_height, false, TCOD_BKGND_SET, "Inventory");
            }
            break;
            case PANEL_SPELLBOOK:
            {
                for (int y = 1; y <= 26; y++)
                {
                    TCOD_console_print(renderer->panel, 1, y - panel_status->scroll, "%d) spell", y);
                }

                TCOD_console_set_default_foreground(renderer->panel, TCOD_white);
                TCOD_console_print_frame(renderer->panel, 0, 0, ui->panel_width, ui->panel_height, false, TCOD_BKGND_SET, "Spellbook");
            }
            break;
            }

            TCOD_console_blit(renderer->panel, 0, 0, ui->panel_width, ui->panel_height, NULL, ui->panel_x, ui->panel_y, 1, 1);
        }

        if (ui->tooltip_visible)
        {
            TCOD_console_set_default_background(renderer->tooltip, TCOD_black);
            TCOD_console_set_default_foreground(renderer->tooltip, TCOD_white);
            TCOD_console_clear(renderer->tooltip);

            int y = 1;
            for (void **i = TCOD_list_begin(ui->tooltip_options); i != TCOD_list_end(ui->tooltip_options); i++)
            {
                struct tooltip_option *option = *i;

                TCOD_console_set_default_foreground(renderer->tooltip, option == ui_tooltip_get_selected(ui) ? TCOD_yellow : TCOD_white);
                TCOD_console_print(renderer->tooltip, 1, y, option->text);

                y++;
            }

            TCOD_console_set_default_foreground(renderer->tooltip, TCOD_white);
            TCOD_console_print_frame(renderer->tooltip, 0, 0, ui->tooltip_width, ui->tooltip_height, false, TCOD_BKGND_SET, "");

            TCOD_console_blit(renderer->tooltip, 0, 0, ui->tooltip_width, ui->tooltip_height, NULL, ui->tooltip_x, ui->tooltip_y, 1, 1);
        }

        // DEBUG
        TCOD_console_print(NULL, 0, 0, "Turn: %d", game->turn);
        TCOD_console_print(NULL, 0, 1, "Depth: %d", game->player->level);
        TCOD_console_print(NULL, 0, 2, "X: %d", game->player->x);
        TCOD_console_print(NULL, 0, 3, "Y: %d", game->player->y);
        TCOD_console_print(NULL, 0, 4, "HP: %d", game->player->current_hp);
        TCOD_console_print(NULL, 0, 5, "Kills: %d", game->player->kills);
    }
    break;
    }

    TCOD_console_flush();
}

void renderer_destroy(struct renderer *renderer)
{
    TCOD_noise_delete(renderer->noise);
    TCOD_console_delete(renderer->message_log);
    TCOD_console_delete(renderer->panel);
    TCOD_console_delete(renderer->tooltip);

    free(renderer);
}
