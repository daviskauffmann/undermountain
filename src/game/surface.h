#ifndef UM_SURFACE_H
#define UM_SURFACE_H

#include "light.h"
#include <libtcod.h>

enum surface_type
{
    SURFACE_TYPE_FIRE,
    SURFACE_TYPE_WATER,

    NUM_SURFACES
};

struct surface_data
{
    char glyph;
    TCOD_color_t color;

    int duration;

    enum light_type light_type;
};

struct surface
{
    enum surface_type type;

    uint8_t x;
    uint8_t y;

    int time;

    TCOD_Map *light_fov;
};

struct surface *surface_new(
    const enum surface_type type,
    const uint8_t x,
    const uint8_t y);
void surface_delete(struct surface *surface);

#endif
