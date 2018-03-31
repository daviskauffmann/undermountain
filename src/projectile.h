#ifndef PROJECTILE_H
#define PROJECTILE_H

struct projectile
{
    unsigned char glyph;
    float x;
    float y;
    float dx;
    float dy;
    struct actor *shooter;
    void (*on_hit)(void *on_hit_params);
    void *on_hit_params;
};

struct projectile *projectile_create(unsigned char glyph, float x, float y, float dx, float dy, struct actor *shooter, void (*on_hit)(void *on_hit_params), void *on_hit_params);
void projectile_update(struct projectile *projectile);
void projectile_destroy(struct projectile *projectile);

#endif
