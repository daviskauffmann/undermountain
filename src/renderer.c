#include <libtcod/libtcod.h>
#include <malloc.h>
#include <math.h>

#include "game.h"
#include "input.h"
#include "message.h"
#include "projectile.h"
#include "renderer.h"
#include "ui.h"
#include "window.h"

#include "CMemleak.h"

struct renderer *renderer_create(void)
{
    struct renderer *renderer = malloc(sizeof(struct renderer));

    renderer->noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
    renderer->noise_x = 0.0f;
    renderer->panel = TCOD_console_new(console_width, console_height);
    renderer->message_log = TCOD_console_new(console_width, console_height);

    return renderer;
}

void renderer_draw(struct renderer *renderer, struct game *game, struct input *input, struct ui *ui)
{
    TCOD_console_set_default_background(NULL, TCOD_black);
    TCOD_console_set_default_foreground(NULL, TCOD_white);
    TCOD_console_clear(NULL);

    switch (game->state)
    {
    case STATE_MENU:
    {
        switch (ui->menu_state)
        {
        case MENU_STATE_MAIN:
        {
            TCOD_console_print_ex(NULL, console_width / 2, console_height / 4, TCOD_BKGND_SET, TCOD_CENTER, "Roguelike thing");

            for (int i = 0; i < NUM_MENU_OPTIONS; i++)
            {
                struct menu_option_info *menu_option_info = &ui->menu_option_info[i];

                TCOD_color_t color = ui->menu_index == i ? TCOD_yellow : TCOD_white;

                TCOD_console_set_default_foreground(NULL, color);
                TCOD_console_print_ex(NULL, console_width / 2, console_height / 2 + i, TCOD_BKGND_SET, TCOD_CENTER, menu_option_info->text);
            }
        }
        break;
        case MENU_STATE_ABOUT:
        {
            TCOD_console_print_rect(NULL, console_width / 2, console_height / 4, 100, 100, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.");

            TCOD_console_print_rect(NULL, console_width / 2, console_height / 2, 100, 100, "Press ENTER to return.");
        }
        break;
        }
    }
    break;
    case STATE_PLAYING:
    {
        int message_log_x = 0;
        int message_log_height = console_height / 4;
        int message_log_y = console_height - message_log_height;
        int message_log_width = console_width;

        int panel_width = console_width / 2;
        int panel_x = console_width - panel_width;
        int panel_y = 0;
        int panel_height = console_height - (ui->message_log_visible ? message_log_height : 0);

        int view_width = console_width - (ui->panel_visible ? panel_width : 0);
        int view_height = console_height - (ui->message_log_visible ? message_log_height : 0);
        int view_x = game->player->x - view_width / 2;
        int view_y = game->player->y - view_height / 2;

        if (view_x + view_width > MAP_WIDTH)
            view_x = MAP_WIDTH - view_width;
        if (view_x < 0)
            view_x = 0;
        if (view_y + view_height > MAP_HEIGHT)
            view_y = MAP_HEIGHT - view_height;
        if (view_y < 0)
            view_y = 0;

        struct map *map = &game->maps[game->player->level];

        {
            renderer->noise_x += 0.2f;
            float noise_dx = renderer->noise_x + 20.0f;
            float dx = TCOD_noise_get(renderer->noise, &noise_dx) * 0.5f;
            noise_dx += 30.0f;
            float dy = TCOD_noise_get(renderer->noise, &noise_dx) * 0.5f;
            float di = 0.2f * TCOD_noise_get(renderer->noise, &renderer->noise_x);

            for (int x = view_x; x < view_x + view_width; x++)
            {
                for (int y = view_y; y < view_y + view_height; y++)
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
                            TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, color);
                            TCOD_console_set_char(NULL, x - view_x, y - view_y, tile_info->glyph);
                        }
                    }
                }
            }

            for (void **iterator = TCOD_list_begin(map->actors); iterator != TCOD_list_end(map->actors); iterator++)
            {
                struct actor *actor = *iterator;

                if (actor->dead && TCOD_map_is_in_fov(game->player->fov, actor->x, actor->y))
                {
                    TCOD_console_set_char_foreground(NULL, actor->x - view_x, actor->y - view_y, TCOD_dark_red);
                    TCOD_console_set_char(NULL, actor->x - view_x, actor->y - view_y, '%');
                }
            }

            for (void **iterator = TCOD_list_begin(map->objects); iterator != TCOD_list_end(map->objects); iterator++)
            {
                struct object *object = *iterator;

                if (TCOD_map_is_in_fov(game->player->fov, object->x, object->y))
                {
                    TCOD_console_set_char_foreground(NULL, object->x - view_x, object->y - view_y, object->color);
                    TCOD_console_set_char(NULL, object->x - view_x, object->y - view_y, game->object_info[object->type].glyph);
                }
            }

            for (void **iterator = TCOD_list_begin(map->items); iterator != TCOD_list_end(map->items); iterator++)
            {
                struct item *item = *iterator;

                if (TCOD_map_is_in_fov(game->player->fov, item->x, item->y))
                {
                    TCOD_console_set_char_foreground(NULL, item->x - view_x, item->y - view_y, game->base_item_info[game->item_info[item->type].base_item].color);
                    TCOD_console_set_char(NULL, item->x - view_x, item->y - view_y, game->base_item_info[game->item_info[item->type].base_item].glyph);
                }
            }

            for (void **iterator = TCOD_list_begin(map->projectiles); iterator != TCOD_list_end(map->projectiles); iterator++)
            {
                struct projectile *projectile = *iterator;

                int x = (int)projectile->x;
                int y = (int)projectile->y;

                if (TCOD_map_is_in_fov(game->player->fov, x, y))
                {
                    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, TCOD_white);
                    TCOD_console_set_char(NULL, x - view_x, y - view_y, projectile->glyph);
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

                    TCOD_console_set_char_foreground(NULL, actor->x - view_x, actor->y - view_y, color);
                    TCOD_console_set_char(NULL, actor->x - view_x, actor->y - view_y, game->race_info[actor->race].glyph);
                }
            }
        }

        if (input->targeting != TARGETING_NONE)
        {
            TCOD_console_set_char_foreground(NULL, input->target_x - view_x, input->target_y - view_y, TCOD_red);
            TCOD_console_set_char(NULL, input->target_x - view_x, input->target_y - view_y, 'X');

            struct tile *tile = &map->tiles[input->target_x][input->target_y];

            if (TCOD_map_is_in_fov(game->player->fov, input->target_x, input->target_y))
            {
                {
                    struct actor *actor = TCOD_list_peek(tile->actors);

                    if (actor)
                    {
                        TCOD_console_print_ex(
                            NULL,
                            console_width / 2,
                            message_log_y - 2,
                            TCOD_BKGND_NONE,
                            TCOD_CENTER,
                            "%s, Race: %s, Class: %s, HP: %d, Kills: %d",
                            actor->name,
                            game->race_info[actor->race].name,
                            game->class_info[actor->class].name,
                            actor->current_hp,
                            actor->kills);

                        goto done;
                    }
                }

                {
                    struct item *item = TCOD_list_peek(tile->items);

                    if (item)
                    {
                        TCOD_console_print_ex(
                            NULL,
                            console_width / 2,
                            message_log_y - 2,
                            TCOD_BKGND_NONE,
                            TCOD_CENTER,
                            game->item_info[item->type].name);

                        goto done;
                    }
                }

                {
                    struct object *object = TCOD_list_peek(tile->objects);

                    if (object)
                    {
                        TCOD_console_print_ex(
                            NULL,
                            console_width / 2,
                            message_log_y - 2,
                            TCOD_BKGND_NONE,
                            TCOD_CENTER,
                            game->object_info[object->type].name);

                        goto done;
                    }
                }

                TCOD_console_print_ex(
                    NULL,
                    console_width / 2,
                    message_log_y - 2,
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
                        console_width / 2,
                        message_log_y - 2,
                        TCOD_BKGND_NONE,
                        TCOD_CENTER,
                        "%s (known)",
                        game->tile_info[tile->type].name);
                }
                else
                {
                    TCOD_console_print_ex(
                        NULL,
                        console_width / 2,
                        message_log_y - 2,
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
            TCOD_console_print_frame(renderer->message_log, 0, 0, message_log_width, message_log_height, false, TCOD_BKGND_SET, "Log");

            TCOD_console_blit(renderer->message_log, 0, 0, message_log_width, message_log_height, NULL, message_log_x, message_log_y, 1, 1);
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
                int i = 1;
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "NAME     : %s", game->player->name);
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "ALIGNMENT: Neutral Good");
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "RACE     : %s", game->race_info[game->player->race].name);
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "CLASS    : %s", game->class_info[game->player->class].name);
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "EXP      : 3653");
                i++;
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "STR: %d", game->player->strength);
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "DEX: %d", game->player->dexterity);
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "CON: %d", game->player->constitution);
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "INT: %d", game->player->intelligence);
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "WIS: %d", game->player->wisdom);
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "CHA: %d", game->player->charisma);
                i++;
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "Armor");
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "Belt");
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "Boots");
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "Cloak");
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "Gloves");
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "Hand");
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "Helmet");
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "M-Hand");
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "O-Hand");
                i++;
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "AC: %d", actor_calc_armor_class(game->player));
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "HP: %d / %d", game->player->current_hp, actor_calc_max_hp(game->player));
                i++;
                int num_dice;
                int die_to_roll;
                int crit_threat;
                int crit_mult;
                actor_calc_weapon(game->player, &num_dice, &die_to_roll, &crit_threat, &crit_mult, false);
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "MELEE: %dd%d (%d-20x%d)", num_dice, die_to_roll, crit_threat, crit_mult);
                actor_calc_weapon(game->player, &num_dice, &die_to_roll, &crit_threat, &crit_mult, true);
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "RANGED: %dd%d (%d-20x%d)", num_dice, die_to_roll, crit_threat, crit_mult);
                i++;
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "ATTACK: +%d", actor_calc_attack_bonus(game->player));
                TCOD_console_print(renderer->panel, 1, i++ - panel_status->scroll, "DAMAGE: +%d", actor_calc_damage_bonus(game->player));

                TCOD_console_set_default_foreground(renderer->panel, TCOD_white);
                TCOD_console_print_frame(renderer->panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Character");
            }
            break;
            case PANEL_INVENTORY:
            {
                int i = 0;
                int y = 1;
                for (void **iterator = TCOD_list_begin(game->player->items); iterator != TCOD_list_end(game->player->items); iterator++)
                {
                    struct item *item = *iterator;

                    TCOD_color_t color = panel_status->current_index == i ? TCOD_yellow : game->base_item_info[game->item_info[item->type].base_item].color;

                    TCOD_console_set_default_foreground(renderer->panel, color);
                    TCOD_console_print(renderer->panel, 1, y - panel_status->scroll, game->item_info[item->type].name);

                    i++;
                    y++;
                }

                TCOD_console_set_default_foreground(renderer->panel, TCOD_white);
                TCOD_console_print_frame(renderer->panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Inventory");
            }
            break;
            case PANEL_SPELLBOOK:
            {
                TCOD_console_set_default_foreground(renderer->panel, TCOD_white);
                TCOD_console_print_frame(renderer->panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Spellbook");
            }
            break;
            }

            TCOD_console_blit(renderer->panel, 0, 0, panel_width, panel_height, NULL, panel_x, panel_y, 1, 1);
        }

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
    TCOD_console_delete(renderer->panel);
    TCOD_console_delete(renderer->message_log);

    free(renderer);
}
