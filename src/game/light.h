#ifndef UM_LIGHT_H
#define UM_LIGHT_H

#include <libtcod.h>

enum light_type
{
    LIGHT_TYPE_NONE,

    LIGHT_TYPE_ACID,
    LIGHT_TYPE_ACID_SPLASH,
    LIGHT_TYPE_ALTAR,
    LIGHT_TYPE_BRAZIER,
    LIGHT_TYPE_FIRE,
    LIGHT_TYPE_FIREBALL,
    LIGHT_TYPE_GLOW,
    LIGHT_TYPE_STAIRS,
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

extern const struct light_data light_database[NUM_LIGHT_TYPES];

#endif
