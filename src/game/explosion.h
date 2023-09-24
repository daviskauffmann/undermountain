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

struct explosion
{
    enum explosion_type type;

    int floor;
    int x;
    int y;

    int radius;
    TCOD_ColorRGB color;

    float time;

    TCOD_Map *fov;
};

struct explosion *explosion_new(
    enum explosion_type type,
    int floor,
    int x,
    int y,
    int radius,
    TCOD_ColorRGB color,
    struct actor *initiator);
void explosion_delete(struct explosion *explosion);
bool explosion_update(struct explosion *explosion, float delta_time);

#endif
