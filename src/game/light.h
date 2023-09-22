#ifndef UM_LIGHT_H
#define UM_LIGHT_H

#include <libtcod.h>

enum light_type
{
    LIGHT_TYPE_NONE,

    LIGHT_TYPE_ALTAR,
    LIGHT_TYPE_BRAZIER,
    LIGHT_TYPE_FIRE,
    LIGHT_TYPE_FIREBALL,
    LIGHT_TYPE_GLOW,
    LIGHT_TYPE_TORCH,

    NUM_LIGHT_TYPES
};

struct light_data
{
    int radius;
    TCOD_ColorRGB color;
    float intensity;
    bool flicker;
};

#endif
