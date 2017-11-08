#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

light_t *light_create(map_t *map, int x, int y, int radius, TCOD_color_t color)
{
    light_t *light = (light_t *)malloc(sizeof(light_t));

    light->map = map;
    light->x = x;
    light->y = y;
    light->radius = radius;
    light->color = color;
    light->on = false;
    light->fov_map = NULL;

    if (TCOD_random_get_int(NULL, 0, 1) == 0)
    {
        light->on = true;
    }

    light_calc_fov(light);

    return light;
}

void light_calc_fov(light_t *light)
{
    if (light->fov_map != NULL)
    {
        TCOD_map_delete(light->fov_map);
    }

    light->fov_map = map_to_TCOD_map(light->map);

    TCOD_map_compute_fov(light->fov_map, light->x, light->y, light->radius, true, FOV_DIAMOND);
}

void light_turn(light_t *light)
{
    light_calc_fov(light);
}

void light_tick(light_t *light)
{
}

void light_draw_turn(light_t *light)
{
    TCOD_console_set_char_foreground(NULL, light->x - view_x, light->y - view_y, light->color);
    TCOD_console_set_char(NULL, light->x - view_x, light->y - view_y, '*');
}

void light_draw_tick(light_t *light)
{
}

void light_destroy(light_t *light)
{
    if (light->fov_map != NULL)
    {
        TCOD_map_delete(light->fov_map);
    }

    free(light);
}