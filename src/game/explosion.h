#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <libtcod.h>

struct actor;

struct explosion
{
    uint8_t floor;
    int x;
    int y;

    int radius;
    TCOD_color_t color;
    float lifetime;

    TCOD_map_t fov;
};

struct explosion *explosion_new(
    uint8_t floor,
    int x,
    int y,
    int radius,
    TCOD_color_t color,
    struct actor *initiator);
void explosion_delete(struct explosion *explosion);
bool explosion_update(struct explosion *explosion, float delta_time);

#endif
