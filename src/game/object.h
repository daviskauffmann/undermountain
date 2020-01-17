#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include <libtcod.h>

struct object_common
{
    char __placeholder;
};

enum object_type
{
    OBJECT_TYPE_ALTAR,
    OBJECT_TYPE_BRAZIER,
    OBJECT_TYPE_CHEST,
    OBJECT_TYPE_DOOR_CLOSED,
    OBJECT_TYPE_DOOR_OPEN,
    OBJECT_TYPE_FOUNTAIN,
    OBJECT_TYPE_STAIR_DOWN,
    OBJECT_TYPE_STAIR_UP,
    OBJECT_TYPE_THRONE,

    NUM_OBJECT_TYPES
};

struct object_data
{
    const char *name;
    unsigned char glyph;
    bool is_walkable;
    bool is_transparent;
};

struct object
{
    enum object_type type;
    int floor;
    int x;
    int y;
    TCOD_color_t color;
    int light_radius;
    TCOD_color_t light_color;
    float light_intensity;
    bool light_flicker;
    TCOD_map_t light_fov;
    bool destroyed;
};

struct object *object_new(enum object_type type, int floor, int x, int y, TCOD_color_t color, int light_radius, TCOD_color_t light_color, float light_intensity, bool light_flicker);
void object_delete(struct object *object);
void object_calc_light(struct object *object);

#endif
