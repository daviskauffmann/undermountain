#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <libtcod.h>

struct explosion
{
    int floor;
    int x;
    int y;
    int max_radius;
    float current_radius;
    struct actor *initiator;
    TCOD_list_t visited_tiles;
    TCOD_map_t light_fov;
};

struct explosion *explosion_new(int floor, int x, int y, int max_radius, struct actor *initiator);
bool explosion_update(struct explosion *explosion, float delta_time);
void explosion_delete(struct explosion *explosion);

#endif
