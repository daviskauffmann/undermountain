#ifndef PROJECTILE_H
#define PROJECTILE_H

#include <libtcod.h>

enum projectile_type
{
    PROJECTILE_TYPE_ARROW,
    PROJECTILE_TYPE_FIREBALL,

    NUM_PROJECTILE_TYPES
};

struct projectile_datum
{
    unsigned char glyph;
    TCOD_color_t color;
    int light_radius;
    TCOD_color_t light_color;
    float light_intensity;
    bool light_flicker;
    float speed;
};

struct projectile
{
    enum projectile_type type;
    int floor;
    float distance;
    float angle;
    float x;
    float y;
    float dx;
    float dy;
    struct actor *shooter;
    struct item *ammunition;
    TCOD_map_t light_fov;
};

struct projectile *projectile_new(enum projectile_type type, int floor, int x1, int y1, int x2, int y2, struct actor *shooter, struct item *ammunition);
void projectile_delete(struct projectile *projectile);
bool projectile_move(struct projectile *projectile, float delta_time);
void projectile_calc_light(struct projectile *projectile);

#endif
