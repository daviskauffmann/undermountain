#include <stdlib.h>
#include <math.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

void tile_initialize(tile_t *tile, tile_type_t type)
{
    tile->type = type;
    tile->seen = false;
    tile->light = NULL;
    tile->actor = NULL;
    tile->items = TCOD_list_new();
}

void tile_turn(tile_t *tile)
{
    // if (tile->actor != NULL)
    // {
    //     actor_turn(tile->actor);
    // }

    for (void **i = TCOD_list_begin(tile->items); i != TCOD_list_end(tile->items); i++)
    {
        item_t *item = *i;

        item_turn(item);
    }
}

void tile_tick(tile_t *tile)
{
    for (void **i = TCOD_list_begin(tile->items); i != TCOD_list_end(tile->items); i++)
    {
        item_t *item = *i;

        item_tick(item);
    }
}

void tile_draw_turn(tile_t *tile, int x, int y)
{
    if (TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        tile->seen = true;
    }

    for (void **i = TCOD_list_begin(player->map->lights); i != TCOD_list_end(player->map->lights); i++)
    {
        light_t *light = *i;

        if (TCOD_map_is_in_fov(light->fov_map, x, y))
        {
            tile->seen = true;
        }
    }

    for (void **i = TCOD_list_begin(player->map->actors); i != TCOD_list_end(player->map->actors); i++)
    {
        actor_t *actor = *i;

        if (actor->torch && TCOD_map_is_in_fov(actor->fov_map, x, y))
        {
            tile->seen = true;
        }
    }

    if (tile->light != NULL)
    {
        return;
    }

    if (tile->actor != NULL && TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        return;
    }

    item_t *item = TCOD_list_peek(tile->items);
    if (item != NULL && TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        item_draw_turn(item, x, y);

        return;
    }

    TCOD_color_t color;

    if (TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        float r2 = pow(player->fov_radius, 2);
        float d = pow(x - player->x, 2) + pow(y - player->y, 2);
        float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

        color = TCOD_color_lerp(tile_color_dark, tile_color_light, l);
    }
    else
    {
        if (tile->seen)
        {
            color = tile_color_dark;
        }
        else
        {
            return;
        }
    }

    for (void **i = TCOD_list_begin(player->map->lights); i != TCOD_list_end(player->map->lights); i++)
    {
        light_t *light = *i;

        if (TCOD_map_is_in_fov(light->fov_map, x, y))
        {
            float light_r2 = pow(light->radius, 2);
            float light_d = pow(x - light->x, 2) + pow(y - light->y, 2);
            float light_l = CLAMP(0.0f, 1.0f, (light_r2 - light_d) / light_r2);

            color = TCOD_color_lerp(color, light->color, light_l);
        }
    }

    for (void **i = TCOD_list_begin(player->map->actors); i != TCOD_list_end(player->map->actors); i++)
    {
        actor_t *actor = *i;

        if (actor->torch && TCOD_map_is_in_fov(actor->fov_map, x, y))
        {
            float torch_r2 = pow(actor->fov_radius, 2);
            float torch_d = pow(x - actor->x, 2) + pow(y - actor->y, 2);
            float torch_l = CLAMP(0.0f, 1.0f, (torch_r2 - torch_d) / torch_r2);

            color = TCOD_color_lerp(color, torch_color, torch_l);
        }
    }

    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, color);
    TCOD_console_set_char(NULL, x - view_x, y - view_y, tile_glyph[tile->type]);
}

void tile_draw_tick(tile_t *tile, int x, int y, float dx, float dy, float di)
{
    if (tile->light != NULL)
    {
        return;
    }

    if (tile->actor != NULL)
    {
        return;
    }

    item_t *item = TCOD_list_peek(tile->items);
    if (item != NULL)
    {
        item_draw_tick(item, x, y);

        return;
    }

    TCOD_color_t color;

    if (TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        float r2 = pow(player->fov_radius, 2);
        float d = pow(x - player->x, 2) + pow(y - player->y, 2);
        float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

        color = TCOD_color_lerp(tile_color_dark, tile_color_light, l);
    }
    else
    {
        if (tile->seen)
        {
            color = tile_color_dark;
        }
        else
        {
            return;
        }
    }

    for (void **i = TCOD_list_begin(player->map->lights); i != TCOD_list_end(player->map->lights); i++)
    {
        light_t *light = *i;

        if (TCOD_map_is_in_fov(light->fov_map, x, y))
        {
            float light_r2 = pow(light->radius, 2);
            float light_d = pow(x - light->x + dx, 2) + pow(y - light->y + dy, 2);
            float light_l = CLAMP(0.0f, 1.0f, (light_r2 - light_d) / light_r2 + di);

            color = TCOD_color_lerp(color, light->color, light_l);
        }
    }

    for (void **i = TCOD_list_begin(player->map->actors); i != TCOD_list_end(player->map->actors); i++)
    {
        actor_t *actor = *i;

        if (actor->torch && TCOD_map_is_in_fov(actor->fov_map, x, y))
        {
            float torch_r2 = pow(actor->fov_radius, 2);
            float torch_d = pow(x - actor->x + dx, 2) + pow(y - actor->y + dy, 2);
            float torch_l = CLAMP(0.0f, 1.0f, (torch_r2 - torch_d) / torch_r2 + di);

            color = TCOD_color_lerp(color, torch_color, torch_l);
        }
    }

    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, color);
}

void tile_finalize(tile_t *tile)
{
    for (void **i = TCOD_list_begin(tile->items); i != TCOD_list_end(tile->items); i++)
    {
        item_t *item = *i;

        item_destroy(item);
    }

    TCOD_list_delete(tile->items);
}