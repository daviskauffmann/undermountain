#ifndef GAME_EXPLOSION_H
#define GAME_EXPLOSION_H

#include <libtcod.h>

struct actor;

struct explosion
{
    uint8_t floor;
    uint8_t x;
    uint8_t y;

    int radius;
    TCOD_ColorRGB color;
    float lifetime;

    TCOD_Map *fov;
};

struct explosion *explosion_new(
    uint8_t floor,
    uint8_t x,
    uint8_t y,
    int radius,
    TCOD_ColorRGB color,
    struct actor *initiator);
void explosion_delete(struct explosion *explosion);
bool explosion_update(struct explosion *explosion, float delta_time);

#endif
