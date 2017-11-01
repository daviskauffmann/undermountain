#include <stdlib.h>
#include <math.h>
#include <libtcod.h>

#include "console.h"
#include "config.h"
#include "world.h"
#include "game.h"

void console_init(void)
{
    right_panel_width = screen_width / 2;
    right_panel_x = screen_width - right_panel_width;
    right_panel_y = 0;
    right_panel_height = screen_height;
    right_panel_visible = false;

    bottom_panel_x = 0;
    bottom_panel_width = screen_width;
    bottom_panel_height = screen_height / 4;
    bottom_panel_y = screen_height - bottom_panel_height;
    bottom_panel_visible = true;

    if (right_panel == NULL)
    {
        right_panel = TCOD_console_new(right_panel_width, right_panel_height);
    }

    if (bottom_panel == NULL)
    {
        bottom_panel = TCOD_console_new(bottom_panel_width, bottom_panel_height);
    }

    if (messages == NULL)
    {
        messages = TCOD_list_new();
    }
    else
    {
        TCOD_list_clear(messages);
    }
}

void console_log(const char *message)
{
    TCOD_list_push(messages, message);

    if (TCOD_list_size(messages) == bottom_panel_height - 1)
    {
        TCOD_list_reverse(messages);
        TCOD_list_pop(messages);
        TCOD_list_reverse(messages);
    }
}

void console_turn_draw(void)
{
    bottom_panel_width = screen_width - (right_panel_visible
                                             ? right_panel_width
                                             : 0);

    view_right = screen_width - (right_panel_visible
                                     ? right_panel_width
                                     : 0);
    view_bottom = screen_height - (bottom_panel_visible
                                       ? bottom_panel_height
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

    TCOD_console_set_default_background(NULL, default_background_color);
    TCOD_console_set_default_foreground(NULL, default_foreground_color);
    TCOD_console_clear(NULL);

    TCOD_console_set_default_background(right_panel, default_background_color);
    TCOD_console_set_default_foreground(right_panel, default_foreground_color);
    TCOD_console_clear(right_panel);

    TCOD_console_set_default_background(bottom_panel, default_background_color);
    TCOD_console_set_default_foreground(bottom_panel, default_foreground_color);
    TCOD_console_clear(bottom_panel);

    float r2 = pow(actorinfo[player->type].sight_radius, 2);

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

            if (actor != NULL && TCOD_map_is_in_fov(player->fov_map, x, y))
            {
                TCOD_console_set_char_foreground(NULL, actor->x - view_left, actor->y - view_top, actorinfo[actor->type].color);
                TCOD_console_set_char(NULL, actor->x - view_left, actor->y - view_top, actorinfo[actor->type].glyph);

                continue;
            }

            TCOD_color_t color;
            if (TCOD_map_is_in_fov(player->fov_map, x, y))
            {
                tile->seen = true;

                float d = pow(x - player->x, 2) + pow(y - player->y, 2);
                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

                if (torch)
                {
                    color = TCOD_color_lerp(tileinfo[tile->type].dark_color, TCOD_color_lerp(tileinfo[tile->type].light_color, torch_color, l), l);
                }
                else
                {
                    color = TCOD_color_lerp(tileinfo[tile->type].dark_color, tileinfo[tile->type].light_color, l);
                }
            }
            else
            {
                if (tile->seen)
                {
                    color = tileinfo[tile->type].dark_color;
                }
                else
                {
                    continue;
                }
            }

            TCOD_console_set_char_foreground(NULL, x - view_left, y - view_top, color);
            TCOD_console_set_char(NULL, x - view_left, y - view_top, tileinfo[tile->type].glyph);
        }
    }

    if (bottom_panel_visible)
    {
        int y = 1;
        for (const char **iterator = (const char **)TCOD_list_begin(messages);
             iterator != (const char **)TCOD_list_end(messages);
             iterator++)
        {
            const char *message = *iterator;

            TCOD_console_print_ex(bottom_panel, 1, y, TCOD_BKGND_NONE, TCOD_LEFT, message);

            y++;
        }
    }

    if (right_panel_visible)
    {
        TCOD_console_print_ex(right_panel, 1, 1, TCOD_BKGND_NONE, TCOD_LEFT, "HP: 15 / 20");
        TCOD_console_print_ex(right_panel, 1, 2, TCOD_BKGND_NONE, TCOD_LEFT, "MP:  7 / 16");

        TCOD_console_print_ex(right_panel, 1, 4, TCOD_BKGND_NONE, TCOD_LEFT, "STR: 16");
        TCOD_console_print_ex(right_panel, 1, 5, TCOD_BKGND_NONE, TCOD_LEFT, "DEX: 14");
        TCOD_console_print_ex(right_panel, 1, 6, TCOD_BKGND_NONE, TCOD_LEFT, "CON: 12");
        TCOD_console_print_ex(right_panel, 1, 7, TCOD_BKGND_NONE, TCOD_LEFT, "INT: 10");
        TCOD_console_print_ex(right_panel, 1, 8, TCOD_BKGND_NONE, TCOD_LEFT, "WIS: 8");
        TCOD_console_print_ex(right_panel, 1, 9, TCOD_BKGND_NONE, TCOD_LEFT, "CHA: 10");

        TCOD_console_print_ex(right_panel, 1, 11, TCOD_BKGND_NONE, TCOD_LEFT, "R-Hand: Sword");
        TCOD_console_print_ex(right_panel, 1, 12, TCOD_BKGND_NONE, TCOD_LEFT, "L-Hand: Shield");
        TCOD_console_print_ex(right_panel, 1, 13, TCOD_BKGND_NONE, TCOD_LEFT, "Head  : Helm");
        TCOD_console_print_ex(right_panel, 1, 14, TCOD_BKGND_NONE, TCOD_LEFT, "Chest : Cuirass");
        TCOD_console_print_ex(right_panel, 1, 15, TCOD_BKGND_NONE, TCOD_LEFT, "Legs  : Greaves");
        TCOD_console_print_ex(right_panel, 1, 16, TCOD_BKGND_NONE, TCOD_LEFT, "Feet  : Boots");
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

            float r2 = pow(actorinfo[player->type].sight_radius, 2);

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
                    TCOD_color_t color = TCOD_color_lerp(tileinfo[tile->type].dark_color, TCOD_color_lerp(tileinfo[tile->type].light_color, torch_color, l), l);

                    TCOD_console_set_char_foreground(NULL, x - view_left, y - view_top, color);
                }
            }
        }
    }

    if (right_panel_visible)
    {
        TCOD_console_blit(right_panel, 0, 0, right_panel_width, right_panel_height, NULL, right_panel_x, right_panel_y, 1, 1);
    }

    if (bottom_panel_visible)
    {
        TCOD_console_blit(bottom_panel, 0, 0, bottom_panel_width, bottom_panel_height, NULL, bottom_panel_x, bottom_panel_y, 1, 1);
    }

    TCOD_console_flush();
}