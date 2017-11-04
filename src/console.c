#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <libtcod.h>

#include "console.h"
#include "config.h"
#include "tile.h"
#include "light.h"
#include "actor.h"
#include "item.h"
#include "world.h"
#include "game.h"

void console_initialize(void)
{
    background_color = TCOD_black;
    foreground_color = TCOD_white;
    tile_color_light = TCOD_white;
    tile_color_dark = TCOD_darkest_gray;
    torch_color = TCOD_light_amber;

    sfx = true;

    message_log = TCOD_console_new(screen_width, screen_height);
    messages = TCOD_list_new();
    message_log_visible = true;

    menu = TCOD_console_new(screen_width, screen_height);
    menu_visible = false;
    content = CONTENT_CHARACTER;
    content_scroll[CONTENT_CHARACTER] = 0;
    content_scroll[CONTENT_INVENTORY] = 0;
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
    TCOD_console_set_default_background(NULL, background_color);
    TCOD_console_set_default_foreground(NULL, foreground_color);
    TCOD_console_clear(NULL);

    TCOD_console_set_default_background(menu, background_color);
    TCOD_console_set_default_foreground(menu, foreground_color);
    TCOD_console_clear(menu);

    TCOD_console_set_default_background(message_log, background_color);
    TCOD_console_set_default_foreground(message_log, foreground_color);
    TCOD_console_clear(message_log);

    menu_width = screen_width / 2;
    menu_x = screen_width - menu_width;
    menu_y = 0;
    menu_height = screen_height;
    content_height[CONTENT_CHARACTER] = 18;
    content_height[CONTENT_INVENTORY] = TCOD_list_size(player->items) + 2;

    message_log_x = 0;
    message_log_width = screen_width - (menu_visible ? menu_width : 0);
    message_log_height = screen_height / 4;
    message_log_y = screen_height - message_log_height;

    view_width = screen_width - (menu_visible ? menu_width : 0);
    view_height = screen_height - (message_log_visible ? message_log_height : 0);
    view_x = player->x - view_width / 2;
    view_y = player->y - view_height / 2;

#if CONSTRAIN_VIEW
    view_x = view_x < 0
                 ? 0
                 : view_x + view_width > MAP_WIDTH
                       ? MAP_WIDTH - view_width
                       : view_x;
    view_y = view_y < 0
                 ? 0
                 : view_y + view_height > MAP_HEIGHT
                       ? MAP_HEIGHT - view_height
                       : view_y;
#endif

    for (int x = view_x; x < view_x + view_width; x++)
    {
        for (int y = view_y; y < view_y + view_height; y++)
        {
            if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
            {
                continue;
            }

            tile_t *tile = &player->map->tiles[x][y];
            light_t *light = tile->light;
            actor_t *actor = tile->actor;
            item_t *item = TCOD_list_peek(tile->items);

            if (TCOD_map_is_in_fov(player->fov_map, x, y))
            {
                tile->seen = true;

                if (actor != NULL)
                {
                    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, actor->color);
                    TCOD_console_set_char(NULL, x - view_x, y - view_y, actor->glyph);

                    continue;
                }

                if (item != NULL)
                {
                    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, item->color);
                    TCOD_console_set_char(NULL, x - view_x, y - view_y, item->glyph);

                    continue;
                }

                if (light != NULL)
                {
                    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, light->color);
                    TCOD_console_set_char(NULL, x - view_x, y - view_y, '!');

                    continue;
                }
            }

            TCOD_color_t color;

            if (TCOD_map_is_in_fov(player->fov_map, x, y))
            {
                float r2 = pow(player->fov_radius, 2);
                float d = pow(x - player->x, 2) + pow(y - player->y, 2);
                float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

                color = TCOD_color_lerp(tile_color_dark, tile_color_light, l);

                for (light_t **iterator = (light_t **)TCOD_list_begin(player->map->lights);
                     iterator != (light_t **)TCOD_list_end(player->map->lights);
                     iterator++)
                {
                    light_t *light = *iterator;

                    if (TCOD_map_is_in_fov(light->fov_map, x, y))
                    {
                        float light_r2 = pow(light->radius, 2);
                        float light_d = pow(x - light->x, 2) + pow(y - light->y, 2);
                        float light_l = CLAMP(0.0f, 1.0f, (light_r2 - light_d) / light_r2);

                        color = color = TCOD_color_lerp(color, TCOD_color_lerp(color, light->color, light_l), l);
                    }
                }

                for (actor_t **iterator = (actor_t **)TCOD_list_begin(player->map->actors);
                     iterator != (actor_t **)TCOD_list_end(player->map->actors);
                     iterator++)
                {
                    actor_t *actor = *iterator;

                    if (actor->torch && TCOD_map_is_in_fov(actor->fov_map, x, y))
                    {
                        float torch_r2 = pow(actor->fov_radius, 2);
                        float torch_d = pow(x - actor->x, 2) + pow(y - actor->y, 2);
                        float torch_l = CLAMP(0.0f, 1.0f, (torch_r2 - torch_d) / torch_r2);

                        color = TCOD_color_lerp(color, torch_color, torch_l);
                    }
                }
            }
            else
            {
                if (tile->seen)
                {
                    color = tile_color_dark;
                }
                else
                {
                    continue;
                }
            }

            TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, color);
            TCOD_console_set_char(NULL, x - view_x, y - view_y, tile_glyph[tile->type]);
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

        TCOD_console_set_default_foreground(message_log, foreground_color);
        TCOD_console_print_frame(message_log, 0, 0, message_log_width, message_log_height, false, TCOD_BKGND_SET, "Log");

        TCOD_console_blit(message_log, 0, 0, message_log_width, message_log_height, NULL, message_log_x, message_log_y, 1, 1);
    }

    if (menu_visible)
    {
        switch (content)
        {
        case CONTENT_CHARACTER:
            TCOD_console_print_ex(menu, 1, 1 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "HP: 15 / 20");
            TCOD_console_print_ex(menu, 1, 2 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "MP:  7 / 16");

            TCOD_console_print_ex(menu, 1, 4 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "STR: 16");
            TCOD_console_print_ex(menu, 1, 5 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "DEX: 14");
            TCOD_console_print_ex(menu, 1, 6 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "CON: 12");
            TCOD_console_print_ex(menu, 1, 7 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "INT: 10");
            TCOD_console_print_ex(menu, 1, 8 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "WIS: 8");
            TCOD_console_print_ex(menu, 1, 9 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "CHA: 10");

            TCOD_console_print_ex(menu, 1, 11 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "R-Hand: Sword");
            TCOD_console_print_ex(menu, 1, 12 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "L-Hand: Shield");
            TCOD_console_print_ex(menu, 1, 13 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "Head  : Helm");
            TCOD_console_print_ex(menu, 1, 14 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "Chest : Cuirass");
            TCOD_console_print_ex(menu, 1, 15 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "Legs  : Greaves");
            TCOD_console_print_ex(menu, 1, 16 - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "Feet  : Boots");

            TCOD_console_print_frame(menu, 0, 0, menu_width, menu_height, false, TCOD_BKGND_SET, "Character");

            break;

        case CONTENT_INVENTORY:
            int y = 1;
            for (item_t **iterator = (item_t **)TCOD_list_begin(player->items);
                 iterator != (item_t **)TCOD_list_end(player->items);
                 iterator++)
            {
                item_t *item = *iterator;

                TCOD_console_set_default_foreground(menu, item->color);
                TCOD_console_print_ex(menu, 1, y - content_scroll[content], TCOD_BKGND_NONE, TCOD_LEFT, "{name}");

                y++;
            }

            TCOD_console_set_default_foreground(menu, foreground_color);
            TCOD_console_print_frame(menu, 0, 0, menu_width, menu_height, false, TCOD_BKGND_SET, "Inventory");

            break;
        }

        TCOD_console_blit(menu, 0, 0, menu_width, menu_height, NULL, menu_x, menu_y, 1, 1);
    }
}

void console_tick_draw(void)
{
    if (sfx)
    {
        static TCOD_noise_t noise = NULL;
        if (noise == NULL)
        {
            noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
        }

        static float noise_x = 0.0f;
        float noise_dx;
        float dx;
        float dy;
        float di;

        noise_x += 0.2f;
        noise_dx = noise_x + 20.0f;
        dx = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        noise_dx += 30.0f;
        dy = TCOD_noise_get(noise, &noise_dx) * 0.5f;
        di = 0.2f * TCOD_noise_get(noise, &noise_x);

        for (int x = view_x; x < view_x + view_width; x++)
        {
            for (int y = view_y; y < view_y + view_height; y++)
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

                if (actor != NULL)
                {
                    continue;
                }

                TCOD_color_t color;

                if (TCOD_map_is_in_fov(player->fov_map, x, y))
                {
                    float r2 = pow(player->fov_radius, 2);
                    float d = pow(x - player->x, 2) + pow(y - player->y, 2);
                    float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

                    color = TCOD_color_lerp(tile_color_dark, tile_color_light, l);

                    for (light_t **iterator = (light_t **)TCOD_list_begin(player->map->lights);
                         iterator != (light_t **)TCOD_list_end(player->map->lights);
                         iterator++)
                    {
                        light_t *light = *iterator;

                        if (TCOD_map_is_in_fov(light->fov_map, x, y))
                        {
                            float light_r2 = pow(light->radius, 2);
                            float light_d = pow(x - light->x + dx, 2) + pow(y - light->y + dy, 2);
                            float light_l = CLAMP(0.0f, 1.0f, (light_r2 - light_d) / light_r2 + di);

                            color = TCOD_color_lerp(color, TCOD_color_lerp(color, light->color, light_l), l);
                        }
                    }

                    for (actor_t **iterator = (actor_t **)TCOD_list_begin(player->map->actors);
                         iterator != (actor_t **)TCOD_list_end(player->map->actors);
                         iterator++)
                    {
                        actor_t *actor = *iterator;

                        if (actor->torch && TCOD_map_is_in_fov(actor->fov_map, x, y))
                        {
                            float torch_r2 = pow(actor->fov_radius, 2);
                            float torch_d = pow(x - actor->x + dx, 2) + pow(y - actor->y + dy, 2);
                            float torch_l = CLAMP(0.0f, 1.0f, (torch_r2 - torch_d) / torch_r2 + di);

                            color = (TCOD_color_lerp(color, torch_color, torch_l));
                        }
                    }

                    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, color);
                }
            }
        }
    }

    TCOD_console_flush();
}

void console_finalize(void)
{
    TCOD_console_delete(menu);
    TCOD_list_delete(messages);

    TCOD_console_delete(message_log);
}