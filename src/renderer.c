#include <libtcod/libtcod.h>
#include <malloc.h>
#include <math.h>

#include "config.h"
#include "game.h"
#include "input.h"
#include "message.h"
#include "projectile.h"
#include "renderer.h"
#include "ui.h"

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
                                float r2 = powf((float)game->object_info[object->type].light_radius, 2);
                                float d = powf((float)(x - object->x + (game->object_info[object->type].light_flicker ? dx : 0)), 2) + powf((float)(y - object->y + (game->object_info[object->type].light_flicker ? dy : 0)), 2);
                                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + (game->object_info[object->type].light_flicker ? di : 0));

                                color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, game->object_info[object->type].light_color, l), l);
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
                TCOD_console_set_char_foreground(NULL, object->x - view_x, object->y - view_y, game->object_info[object->type].color);
                TCOD_console_set_char(NULL, object->x - view_x, object->y - view_y, game->object_info[object->type].glyph);
            }
        }

        for (void **iterator = TCOD_list_begin(map->items); iterator != TCOD_list_end(map->items); iterator++)
        {
            struct item *item = *iterator;

            if (TCOD_map_is_in_fov(game->player->fov, item->x, item->y))
            {
                TCOD_console_set_char_foreground(NULL, item->x - view_x, item->y - view_y, game->item_info[item->type].color);
                TCOD_console_set_char(NULL, item->x - view_x, item->y - view_y, game->item_info[item->type].glyph);
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
            struct object *object = TCOD_list_peek(tile->objects);
            struct actor *actor = TCOD_list_peek(tile->actors);
            struct item *item = TCOD_list_peek(tile->items);

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

            if (actor)
            {
                TCOD_console_print_ex(
                    NULL,
                    console_width / 2,
                    message_log_y - 2,
                    TCOD_BKGND_NONE,
                    TCOD_CENTER,
                    "%s %s, Health: %d, Kills: %d",
                    game->race_info[actor->race].name,
                    game->class_info[actor->class].name,
                    actor->health,
                    actor->kills);

                goto done;
            }

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
            TCOD_console_print(renderer->panel, 1, 1 - panel_status->scroll, "HP: 15 / 20");
            TCOD_console_print(renderer->panel, 1, 2 - panel_status->scroll, "MP:  7 / 16");

            TCOD_console_print(renderer->panel, 1, 4 - panel_status->scroll, "STR: 16");
            TCOD_console_print(renderer->panel, 1, 5 - panel_status->scroll, "DEX: 14");
            TCOD_console_print(renderer->panel, 1, 6 - panel_status->scroll, "CON: 12");
            TCOD_console_print(renderer->panel, 1, 7 - panel_status->scroll, "INT: 10");
            TCOD_console_print(renderer->panel, 1, 8 - panel_status->scroll, "WIS: 8");
            TCOD_console_print(renderer->panel, 1, 9 - panel_status->scroll, "CHA: 10");

            TCOD_console_print(renderer->panel, 1, 11 - panel_status->scroll, "R-Hand: Sword");
            TCOD_console_print(renderer->panel, 1, 12 - panel_status->scroll, "L-Hand: Shield");
            TCOD_console_print(renderer->panel, 1, 13 - panel_status->scroll, "Head  : Helm");
            TCOD_console_print(renderer->panel, 1, 14 - panel_status->scroll, "Chest : Cuirass");
            TCOD_console_print(renderer->panel, 1, 15 - panel_status->scroll, "Legs  : Greaves");
            TCOD_console_print(renderer->panel, 1, 16 - panel_status->scroll, "Feet  : Boots");

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

                TCOD_color_t color = panel_status->current_index == i ? TCOD_yellow : game->item_info[item->type].color;

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
    TCOD_console_print(NULL, 0, 4, "Health: %d", game->player->health);
    TCOD_console_print(NULL, 0, 5, "Kills: %d", game->player->kills);

    TCOD_console_flush();
}

void renderer_destroy(struct renderer *renderer)
{
    TCOD_noise_delete(renderer->noise);
    TCOD_console_delete(renderer->panel);
    TCOD_console_delete(renderer->message_log);

    free(renderer);
}
