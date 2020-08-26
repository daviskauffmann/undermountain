#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <libtcod.h>

struct actor;

struct explosion
{
    int floor;
    int x;
    int y;
    int radius;
    TCOD_color_t color;
    float lifetime;
    TCOD_map_t fov;
};

struct explosion *explosion_new(int floor, int x, int y, int radius, TCOD_color_t color, struct actor *initiator);
bool explosion_update(struct explosion *explosion, float delta_time);
void explosion_delete(struct explosion *explosion);

#endif
