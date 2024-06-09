#ifndef UM_GAME_PROJECTILE_H
#define UM_GAME_PROJECTILE_H

#include <libtcod.h>

enum projectile_type
{
    PROJECTILE_TYPE_ACID_SPLASH,
    PROJECTILE_TYPE_ARROW,
    PROJECTILE_TYPE_FIREBALL,
    PROJECTILE_TYPE_MAGIC_MISSILE,

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

    int floor;
    int origin_x;
    int origin_y;
    int target_x;
    int target_y;
    float x;
    float y;

    struct actor *shooter;

    struct item *ammunition;

    int caster_level;

    TCOD_Map *light_fov;
};

extern const struct projectile_data projectile_database[NUM_PROJECTILE_TYPES];

struct projectile *projectile_new(
    enum projectile_type type,
    int floor,
    int origin_x,
    int origin_y,
    int target_x,
    int target_y,
    struct actor *shooter,
    struct item *ammunition,
    int caster_level);
void projectile_delete(struct projectile *projectile);
bool projectile_move(struct projectile *projectile, float delta_time);
void projectile_update_light(struct projectile *projectile);

#endif
