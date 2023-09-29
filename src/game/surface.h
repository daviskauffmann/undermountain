#ifndef UM_SURFACE_H
#define UM_SURFACE_H

#include "light.h"
#include <libtcod.h>

enum surface_type
{
    SURFACE_TYPE_ACID,
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

    int floor;
    int x;
    int y;

    int time;

    TCOD_Map *light_fov;
};

extern const struct surface_data surface_database[NUM_SURFACES];

struct surface *surface_new(
    enum surface_type type,
    int floor,
    int x,
    int y);
void surface_delete(struct surface *surface);
void surface_calc_light(struct surface *surface);

#endif
