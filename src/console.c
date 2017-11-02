#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libtcod.h>

#include "console.h"
#include "config.h"
#include "world.h"
#include "game.h"

void console_initialize(void)
{
    right_panel_visible = false;
    right_panel_width = screen_width / 2;
    right_panel_x = screen_width - right_panel_width;
    right_panel_y = 0;
    right_panel_height = screen_height;
    right_panel = TCOD_console_new(right_panel_width, right_panel_height);
    right_panel_content_type = CONTENT_NONE;
    right_panel_content[CONTENT_CHARACTER].type = CONTENT_CHARACTER;
    right_panel_content[CONTENT_CHARACTER].height = 18;
    right_panel_content[CONTENT_CHARACTER].scroll = 0;
    right_panel_content[CONTENT_INVENTORY].type = CONTENT_INVENTORY;
    right_panel_content[CONTENT_INVENTORY].height = 28;
    right_panel_content[CONTENT_INVENTORY].scroll = 0;

    message_log_visible = true;
    message_log_x = 0;
    message_log_width = screen_width;
    message_log_height = screen_height / 4;
    message_log_y = screen_height - message_log_height;
    message_log = TCOD_console_new(message_log_width, message_log_height);
    messages = TCOD_list_new();
}

void console_log(const char *message, map_t *map, int x, int y)
{
    if (map != player->map)
    {
        return;
    }

    if (!TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        return;
    }

    TCOD_list_push(messages, message);

    if (TCOD_list_size(messages) >= MAX_MESSAGES)
    {
        TCOD_list_remove(messages, *TCOD_list_begin(messages));
    }
}

void console_turn_draw(void)
{
    TCOD_console_set_default_background(NULL, default_background_color);
    TCOD_console_set_default_foreground(NULL, default_foreground_color);
    TCOD_console_clear(NULL);

    TCOD_console_set_default_background(right_panel, default_background_color);
    TCOD_console_set_default_foreground(right_panel, default_foreground_color);
    TCOD_console_clear(right_panel);

    TCOD_console_set_default_background(message_log, default_background_color);
    TCOD_console_set_default_foreground(message_log, default_foreground_color);
    TCOD_console_clear(message_log);

    // TODO: calc lines
    // current HP & MP
    // + # of stats
    // + equipment items * lines each item needs
    // + 2 padding
    right_panel_content[CONTENT_CHARACTER].height = 18;

    // TODO: calc lines
    // inventory items * lines each item needs
    // + 2 padding
    right_panel_content[CONTENT_INVENTORY].height = 28;

    message_log_width = screen_width - (right_panel_visible
                                            ? right_panel_width
                                            : 0);

    view_right = screen_width - (right_panel_visible
                                     ? right_panel_width
                                     : 0);
    view_bottom = screen_height - (message_log_visible
                                       ? message_log_height
                                       : 0);
    view_left = player->x - view_right / 2;
    view_top = player->y - view_bottom / 2;

#if CONSTRAIN_VIEW
    view_left = view_left < 0
                    ? 0
                    : view_left + view_right > MAP_WIDTH
                          ? MAP_WIDTH - view_right
                          : view_left;
    view_top = view_top < 0
                   ? 0
                   : view_top + view_bottom > MAP_HEIGHT
                         ? MAP_HEIGHT - view_bottom
                         : view_top;
#endif

    float r2 = pow(actor_info[player->type].sight_radius, 2);

    for (int x = view_left; x < view_left + view_right; x++)
    {
        for (int y = view_top; y < view_top + view_bottom; y++)
        {
            if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
            {
                continue;
            }

            tile_t *tile = &player->map->tiles[x][y];
            actor_t *actor = tile->actor;

            if (TCOD_map_is_in_fov(player->fov_map, x, y))
            {
                tile->seen = true;
            }

            if (actor != NULL && TCOD_map_is_in_fov(player->fov_map, x, y))
            {
                TCOD_console_set_char_foreground(NULL, actor->x - view_left, actor->y - view_top, actor_info[actor->type].color);
                TCOD_console_set_char(NULL, actor->x - view_left, actor->y - view_top, actor_info[actor->type].glyph);

                continue;
            }

            TCOD_color_t color;
            if (TCOD_map_is_in_fov(player->fov_map, x, y))
            {
                float d = pow(x - player->x, 2) + pow(y - player->y, 2);
                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

                if (torch)
                {
                    color = TCOD_color_lerp(tile_info[tile->type].dark_color, TCOD_color_lerp(tile_info[tile->type].light_color, torch_color, l), l);
                }
                else
                {
                    color = TCOD_color_lerp(tile_info[tile->type].dark_color, tile_info[tile->type].light_color, l);
                }
            }
            else
            {
                if (tile->seen)
                {
                    color = tile_info[tile->type].dark_color;
                }
                else
                {
                    continue;
                }
            }

            TCOD_console_set_char_foreground(NULL, x - view_left, y - view_top, color);
            TCOD_console_set_char(NULL, x - view_left, y - view_top, tile_info[tile->type].glyph);
        }
    }

    if (message_log_visible)
    {
        TCOD_list_t new_messages = TCOD_list_duplicate(messages);

        int total_lines = 0;
        for (const char **iterator = (const char **)TCOD_list_begin(messages);
             iterator != (const char **)TCOD_list_end(messages);
             iterator++)
        {
            const char *message = *iterator;

            total_lines += (int)ceil((float)strlen(message) / (float)(message_log_width - 2));
        }

        while (total_lines > message_log_height - 2)
        {
            const char *message = *TCOD_list_begin(new_messages);

            TCOD_list_remove(new_messages, message);

            total_lines -= (int)ceil((float)strlen(message) / (float)(message_log_width - 2));
        }

        int i = 0;
        int len = TCOD_list_size(new_messages);
        int y = 1;
        for (const char **iterator = (const char **)TCOD_list_begin(new_messages);
             iterator != (const char **)TCOD_list_end(new_messages);
             iterator++)
        {
            i++;

            const char *message = *iterator;

            TCOD_color_t color = i == len
                                     ? TCOD_white
                                     : i == len - 1
                                           ? TCOD_light_gray
                                           : i == len - 2
                                                 ? TCOD_gray
                                                 : TCOD_dark_gray;

            TCOD_console_set_default_foreground(message_log, color);
            y += TCOD_console_print_rect(message_log, 1, y, message_log_width - 2, message_log_height - 1, message);
        }

        TCOD_list_delete(new_messages);

        TCOD_console_set_default_foreground(message_log, default_foreground_color);
        TCOD_console_print_frame(message_log, 0, 0, message_log_width, message_log_height, false, TCOD_BKGND_SET, "Log");

        TCOD_console_blit(message_log, 0, 0, message_log_width, message_log_height, NULL, message_log_x, message_log_y, 1, 1);
    }

    if (right_panel_visible)
    {
        content_t *content = &right_panel_content[right_panel_content_type];

        switch (content->type)
        {
        case CONTENT_CHARACTER:
            TCOD_console_print_ex(right_panel, 1, 1 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "HP: 15 / 20");
            TCOD_console_print_ex(right_panel, 1, 2 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "MP:  7 / 16");

            TCOD_console_print_ex(right_panel, 1, 4 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "STR: 16");
            TCOD_console_print_ex(right_panel, 1, 5 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "DEX: 14");
            TCOD_console_print_ex(right_panel, 1, 6 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "CON: 12");
            TCOD_console_print_ex(right_panel, 1, 7 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "INT: 10");
            TCOD_console_print_ex(right_panel, 1, 8 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "WIS: 8");
            TCOD_console_print_ex(right_panel, 1, 9 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "CHA: 10");

            TCOD_console_print_ex(right_panel, 1, 11 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "R-Hand: Sword");
            TCOD_console_print_ex(right_panel, 1, 12 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "L-Hand: Shield");
            TCOD_console_print_ex(right_panel, 1, 13 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "Head  : Helm");
            TCOD_console_print_ex(right_panel, 1, 14 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "Chest : Cuirass");
            TCOD_console_print_ex(right_panel, 1, 15 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "Legs  : Greaves");
            TCOD_console_print_ex(right_panel, 1, 16 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "Feet  : Boots");

            TCOD_console_print_frame(right_panel, 0, 0, right_panel_width, right_panel_height, false, TCOD_BKGND_SET, "Character");

            break;

        case CONTENT_INVENTORY:
            TCOD_console_print_ex(right_panel, 1, 1 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "a) Sword");
            TCOD_console_print_ex(right_panel, 1, 2 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "b) Sword");
            TCOD_console_print_ex(right_panel, 1, 3 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "c) Sword");
            TCOD_console_print_ex(right_panel, 1, 4 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "d) Sword");
            TCOD_console_print_ex(right_panel, 1, 5 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "e) Sword");
            TCOD_console_print_ex(right_panel, 1, 6 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "f) Sword");
            TCOD_console_print_ex(right_panel, 1, 7 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "g) Sword");
            TCOD_console_print_ex(right_panel, 1, 8 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "h) Sword");
            TCOD_console_print_ex(right_panel, 1, 9 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "i) Sword");
            TCOD_console_print_ex(right_panel, 1, 10 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "j) Sword");
            TCOD_console_print_ex(right_panel, 1, 11 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "k) Sword");
            TCOD_console_print_ex(right_panel, 1, 12 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "l) Sword");
            TCOD_console_print_ex(right_panel, 1, 13 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "m) Sword");
            TCOD_console_print_ex(right_panel, 1, 14 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "n) Sword");
            TCOD_console_print_ex(right_panel, 1, 15 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "o) Sword");
            TCOD_console_print_ex(right_panel, 1, 16 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "p) Sword");
            TCOD_console_print_ex(right_panel, 1, 17 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "q) Sword");
            TCOD_console_print_ex(right_panel, 1, 18 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "r) Sword");
            TCOD_console_print_ex(right_panel, 1, 19 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "s) Sword");
            TCOD_console_print_ex(right_panel, 1, 20 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "t) Sword");
            TCOD_console_print_ex(right_panel, 1, 21 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "u) Sword");
            TCOD_console_print_ex(right_panel, 1, 22 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "v) Sword");
            TCOD_console_print_ex(right_panel, 1, 23 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "w) Sword");
            TCOD_console_print_ex(right_panel, 1, 24 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "x) Sword");
            TCOD_console_print_ex(right_panel, 1, 25 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "y) Sword");
            TCOD_console_print_ex(right_panel, 1, 26 - content->scroll, TCOD_BKGND_NONE, TCOD_LEFT, "z) Sword");

            TCOD_console_print_frame(right_panel, 0, 0, right_panel_width, right_panel_height, false, TCOD_BKGND_SET, "Inventory");

            break;
        }

        TCOD_console_blit(right_panel, 0, 0, right_panel_width, right_panel_height, NULL, right_panel_x, right_panel_y, 1, 1);
    }
}

void console_tick_draw(void)
{
    if (sfx)
    {
        if (torch)
        {
            static TCOD_noise_t noise = NULL;
            if (noise == NULL)
            {
                noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
            }

            static float torchx = 0.0f;
            float tdx;
            float dx;
            float dy;
            float di;

            torchx += 0.2f;
            tdx = torchx + 20.0f;
            dx = TCOD_noise_get(noise, &tdx) * 0.5f;
            tdx += 30.0f;
            dy = TCOD_noise_get(noise, &tdx) * 0.5f;
            di = 0.2f * TCOD_noise_get(noise, &torchx);

            float r2 = pow(actor_info[player->type].sight_radius, 2);

            for (int x = view_left; x < view_left + view_right; x++)
            {
                for (int y = view_top; y < view_top + view_bottom; y++)
                {
                    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
                    {
                        continue;
                    }

                    if (!TCOD_map_is_in_fov(player->fov_map, x, y))
                    {
                        continue;
                    }

                    tile_t *tile = &player->map->tiles[x][y];
                    actor_t *actor = tile->actor;

                    if (actor != NULL && TCOD_map_is_in_fov(player->fov_map, x, y))
                    {
                        continue;
                    }

                    float d = pow(x - player->x + dx, 2) + pow(y - player->y + dy, 2);
                    float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + di);
                    TCOD_color_t color = TCOD_color_lerp(tile_info[tile->type].dark_color, TCOD_color_lerp(tile_info[tile->type].light_color, torch_color, l), l);

                    TCOD_console_set_char_foreground(NULL, x - view_left, y - view_top, color);
                }
            }
        }
    }

    TCOD_console_flush();
}

void console_finalize(void)
{
    TCOD_console_delete(right_panel);

    TCOD_console_delete(message_log);

    TCOD_list_delete(messages);
}