#ifndef OBJECT_H
#define OBJECT_H

#include <libtcod/libtcod.h>

struct object_common
{
    char __placeholder;
};

enum object_type
{
    OBJECT_BRAZIER,

    NUM_OBJECT_TYPES
};

struct object_info
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
    bool is_walkable;
    bool is_transparent;
    int light_radius;
    TCOD_color_t light_color;
};

struct object
{
    enum object_type type;
    int x;
    int y;
    TCOD_map_t light_fov;
};

struct object *object_create(enum object_type type, int x, int y);
void object_destroy(struct object *object);

#endif
