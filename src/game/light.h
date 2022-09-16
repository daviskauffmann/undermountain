#ifndef LIGHT_H
#define LIGHT_H

#include <libtcod.h>

enum light_type
{
    LIGHT_TYPE_NONE,
    LIGHT_TYPE_ALTAR,
    LIGHT_TYPE_BRAZIER,
    LIGHT_TYPE_FIREBALL,
    LIGHT_TYPE_GLOW,
    LIGHT_TYPE_TORCH,

    NUM_LIGHT_TYPES
};

struct light_datum
{
    int radius;
    TCOD_color_t color;
    float intensity;
    bool flicker;
};

#endif
