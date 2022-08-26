#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <libtcod.h>

struct actor;

struct explosion
{
    uint8_t floor;
    uint8_t x;
    uint8_t y;

    int radius;
    TCOD_color_t color;
    float lifetime;

    TCOD_map_t fov;
};

struct explosion *explosion_new(
    uint8_t floor,
    uint8_t x,
    uint8_t y,
    int radius,
    TCOD_color_t color,
    struct actor *initiator);
void explosion_delete(struct explosion *explosion);
void explosion_deal_damage(const struct explosion *explosion, struct actor *initiator);
bool explosion_update(struct explosion *explosion, float delta_time);

#endif
