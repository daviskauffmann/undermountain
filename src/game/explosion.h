#ifndef UM_GAME_EXPLOSION_H
#define UM_GAME_EXPLOSION_H

#include <libtcod.h>

struct actor;

struct explosion
{
    uint8_t floor;
    uint8_t x;
    uint8_t y;

    int8_t radius;
    TCOD_ColorRGB color;

    float time;

    TCOD_Map *fov;
};

struct explosion *explosion_new(
    uint8_t floor,
    uint8_t x,
    uint8_t y,
    int8_t radius,
    TCOD_ColorRGB color,
    struct actor *initiator);
void explosion_delete(struct explosion *explosion);
bool explosion_update(struct explosion *explosion, float delta_time);

#endif
