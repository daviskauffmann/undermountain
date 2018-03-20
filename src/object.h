#ifndef OBJECT_H
#define OBJECT_H

#include <libtcod/libtcod.h>

typedef struct
{
    char __placeholder;
} ObjectCommon;

typedef enum {
    OBJECT_BRAZIER,

    NUM_OBJECT_TYPES
} ObjectType;

typedef struct
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
    int light_radius;
    TCOD_color_t light_color;
} ObjectInfo;

typedef struct
{
    int x;
    int y;
    TCOD_map_t light_fov;
} Object;

#endif
