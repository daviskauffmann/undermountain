#include "light.h"
#include "map.h"

light_t *light_create(map_t *map, int x, int y, int radius, TCOD_color_t color)
{
    light_t *light = (light_t *)malloc(sizeof(light_t));

    light->map = map;
    light->x = x;
    light->y = y;
    light->radius = radius;
    light->color = color;
    light->fov_map = NULL;

    TCOD_list_push(map->lights, light);
    map->tiles[x][y].light = light;

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

void light_destroy(light_t *light)
{
    if (light->fov_map != NULL)
    {
        TCOD_map_delete(light->fov_map);
    }

    light->map->tiles[light->x][light->y].light = NULL;
}