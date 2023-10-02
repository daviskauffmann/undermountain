#ifndef UM_GAME_EXPLOSION_H
#define UM_GAME_EXPLOSION_H

#include <libtcod.h>

struct actor;

enum explosion_type
{
    EXPLOSION_TYPE_ACID_SPLASH,
    EXPLOSION_TYPE_FIREBALL,

    NUM_EXPLOSION_TYPES
};

struct explosion_data
{
    float intensity;
    int radius;
    TCOD_ColorRGB color;
};

struct explosion
{
    enum explosion_type type;

    int floor;
    int x;
    int y;

    float time;

    int caster_level;

    TCOD_Map *fov;
};

extern const struct explosion_data explosion_database[NUM_EXPLOSION_TYPES];

struct explosion *explosion_new(
    enum explosion_type type,
    int floor,
    int x,
    int y,
    struct actor *initiator,
    int caster_level);
void explosion_delete(struct explosion *explosion);
bool explosion_update(struct explosion *explosion, float delta_time);

#endif
