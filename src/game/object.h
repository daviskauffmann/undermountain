#ifndef UM_GAME_OBJECT_H
#define UM_GAME_OBJECT_H

#include <libtcod.h>

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

struct object_data
{
    const char *name;
    unsigned char glyph;
    TCOD_ColorRGB color;
    enum light_type light_type;

    bool is_walkable;
    bool is_transparent;
};

enum object_trap_detection_state
{
    OBJECT_TRAP_DETECTION_STATE_UNCHECKED,
    OBJECT_TRAP_DETECTION_STATE_DETECTED,
    OBJECT_TRAP_DETECTION_STATE_UNDETECTED
};

struct object
{
    enum object_type type;

    int floor;
    int x;
    int y;

    TCOD_Map *light_fov;

    enum object_trap_detection_state trap_detection_state;
};

extern const struct object_data object_database[NUM_OBJECT_TYPES];

struct object *object_new(
    enum object_type type,
    int floor,
    int x,
    int y);
void object_delete(struct object *object);

void object_update_light(struct object *object);

#endif
