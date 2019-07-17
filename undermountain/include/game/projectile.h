#ifndef GAME_PROJECTILE_H
#define GAME_PROJECTILE_H

struct actor;

struct projectile
{
    unsigned char glyph;
    int floor;
    float x;
    float y;
    float dx;
    float dy;
    struct actor *shooter;
    void (*on_hit)(void *on_hit_params);
    void *on_hit_params;
    bool destroyed;
};

struct projectile *projectile_create(unsigned char glyph, int floor, int x1, int y1, int x2, int y2, struct actor *shooter, void (*on_hit)(void *on_hit_params), void *on_hit_params);
void projectile_update(struct projectile *projectile);
void projectile_destroy(struct projectile *projectile);

#endif
