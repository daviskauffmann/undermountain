#ifndef OBJECT_H
#define OBJECT_H

#include <libtcod/libtcod.h>

struct object_common
{
    char __placeholder;
};

enum object_type
{
    OBJECT_TYPE_ALTAR,
    OBJECT_TYPE_BRAZIER,
    OBJECT_TYPE_DOOR_CLOSED,
    OBJECT_TYPE_DOOR_OPEN,
    OBJECT_TYPE_FOUNTAIN,
    OBJECT_TYPE_STAIR_DOWN,
    OBJECT_TYPE_STAIR_UP,
    OBJECT_TYPE_THRONE,

    NUM_OBJECT_TYPES
};

struct object_info
{
    const char *name;
    unsigned char glyph;
    bool is_walkable;
    bool is_transparent;
};

struct object
{
    enum object_type type;
    struct game *game;
    int level;
    int x;
    int y;
    TCOD_color_t color;
    int light_radius;
    TCOD_color_t light_color;
    bool light_flicker;
    TCOD_map_t light_fov;
    bool destroyed;
};

struct object *object_create(enum object_type type, struct game *game, int level, int x, int y, TCOD_color_t color, int light_radius, TCOD_color_t light_color, bool light_flicker);
void object_calc_light(struct object *object);
void object_destroy(struct object *object);

#endif
