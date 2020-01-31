#ifndef GAME_PROJECTILE_H
#define GAME_PROJECTILE_H

#include <stdbool.h>

struct projectile
{
    unsigned char glyph;
    int floor;
    float x;
    float y;
    float dx;
    float dy;
    struct actor *shooter;
    struct item *ammunition;
    bool destroyed;
};

struct projectile *projectile_new(unsigned char glyph, int floor, int x1, int y1, int x2, int y2, struct actor *shooter, struct item *ammunition);
void projectile_delete(struct projectile *projectile);
void projectile_update(struct projectile *projectile, float delta_time);

#endif
