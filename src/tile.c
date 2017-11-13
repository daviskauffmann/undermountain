#include <stdlib.h>
#include <math.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

void tile_init(tile_t *tile, tile_type_t type)
{
    tile->type = type;
    tile->seen = false;
    tile->light = NULL;
    tile->actor = NULL;
    tile->items = TCOD_list_new();
}

void tile_update(tile_t *tile)
{
}

void tile_draw(tile_t *tile, int x, int y, float dx, float dy, float di)
{
    if (TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        tile->seen = true;
    }

    for (void **i = TCOD_list_begin(player->map->lights); i != TCOD_list_end(player->map->lights); i++)
    {
        light_t *light = *i;

        if (light->on && TCOD_map_is_in_fov(light->fov_map, x, y))
        {
            tile->seen = true;
        }
    }

    for (void **i = TCOD_list_begin(player->map->actors); i != TCOD_list_end(player->map->actors); i++)
    {
        actor_t *actor = *i;

        if ((actor->light == ACTOR_LIGHT_DEFAULT || actor->light == ACTOR_LIGHT_TORCH) && TCOD_map_is_in_fov(actor->fov_map, x, y))
        {
            tile->seen = true;
        }
    }

    if (!TCOD_map_is_in_fov(player->fov_map, x, y) && !tile->seen)
    {
        return;
    }

    if (tile->light != NULL && TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        return;
    }

    item_t *item = TCOD_list_peek(tile->items);
    if (item != NULL && TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        return;
    }

    if (tile->actor != NULL && TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        return;
    }

    TCOD_color_t color = tile_color_dark;

    for (void **i = TCOD_list_begin(player->map->actors); i != TCOD_list_end(player->map->actors); i++)
    {
        actor_t *actor = *i;

        if (actor->light == ACTOR_LIGHT_DEFAULT && TCOD_map_is_in_fov(actor->fov_map, x, y))
        {
            float r2 = pow(actor_light_radius[actor->light], 2);
            float d = pow(x - actor->x, 2) + pow(y - actor->y, 2);
            float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

            color = TCOD_color_lerp(color, TCOD_color_lerp(tile_color_light, actor_light_color[actor->light], l), l);
        }
    }

    for (void **i = TCOD_list_begin(player->map->lights); i != TCOD_list_end(player->map->lights); i++)
    {
        light_t *light = *i;

        if (light->on && TCOD_map_is_in_fov(light->fov_map, x, y))
        {
            float r2 = pow(light->radius, 2);
            float d = pow(x - light->x, 2) + pow(y - light->y, 2);
            float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

            color = TCOD_color_lerp(color, TCOD_color_lerp(tile_color_light, light->color, l), l);
        }
    }

    for (void **i = TCOD_list_begin(player->map->actors); i != TCOD_list_end(player->map->actors); i++)
    {
        actor_t *actor = *i;

        if (actor->light == ACTOR_LIGHT_TORCH && TCOD_map_is_in_fov(actor->fov_map, x, y))
        {
            float r2 = pow(actor_light_radius[actor->light], 2);
            float d = pow(x - actor->x + dx, 2) + pow(y - actor->y + dy, 2);
            float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + di);

            color = TCOD_color_lerp(color, TCOD_color_lerp(tile_color_light, actor_light_color[actor->light], l), l);
        }
    }

    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, color);
    TCOD_console_set_char(NULL, x - view_x, y - view_y, tile_glyph[tile->type]);
}

void tile_uninit(tile_t *tile)
{
    TCOD_list_delete(tile->items);
}