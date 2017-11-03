#ifndef LIGHT_H
#define LIGHT_H

#include <libtcod.h>

typedef struct map_s map_t;

typedef struct light_s
{
    map_t *map;
    int x;
    int y;
    int radius;
    TCOD_color_t color;
    TCOD_map_t fov_map;
} light_t;

light_t *light_create(map_t *map, int x, int y, int radius, TCOD_color_t color);
void light_calc_fov(light_t *light);
void light_destroy(light_t *light);

#endif