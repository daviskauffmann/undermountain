#ifndef OBJECT_H
#define OBJECT_H

#include "light.h"
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
    OBJECT_TYPE_TRAP,

    NUM_OBJECT_TYPES
};

struct object_datum
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
    enum light_type light_type;

    bool is_walkable;
    bool is_transparent;
};

struct object
{
    enum object_type type;

    uint8_t floor;
    uint8_t x;
    uint8_t y;

    TCOD_map_t light_fov;
};

struct object *object_new(
    enum object_type type,
    uint8_t floor,
    uint8_t x,
    uint8_t y);
void object_delete(struct object *object);
void object_calc_light(struct object *object);

#endif
