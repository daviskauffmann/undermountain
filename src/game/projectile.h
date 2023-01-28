#ifndef GAME_PROJECTILE_H
#define GAME_PROJECTILE_H

#include "light.h"
#include <libtcod.h>

enum projectile_type
{
    PROJECTILE_TYPE_ARROW,
    PROJECTILE_TYPE_FIREBALL,

    NUM_PROJECTILE_TYPES
};

struct projectile_data
{
    unsigned char glyph;
    TCOD_ColorRGB color;

    enum light_type light_type;

    float speed;
};

struct projectile
{
    enum projectile_type type;

    uint8_t floor;
    int origin_x;
    int origin_y;
    int target_x;
    int target_y;
    float x;
    float y;

    struct actor *shooter;

    struct item *ammunition;

    TCOD_Map * light_fov;
};

struct projectile *projectile_new(
    enum projectile_type type,
    uint8_t floor,
    int origin_x,
    int origin_y,
    int target_x,
    int target_y,
    struct actor *shooter,
    struct item *ammunition);
void projectile_delete(struct projectile *projectile);
bool projectile_move(struct projectile *projectile, float delta_time);
void projectile_calc_light(struct projectile *projectile);

#endif
