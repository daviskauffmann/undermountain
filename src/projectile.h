#ifndef PROJECTILE_H
#define PROJECTILE_H

struct projectile
{
    struct game *game;
    unsigned char glyph;
    int level;
    float x;
    float y;
    float dx;
    float dy;
    struct actor *shooter;
    void (*on_hit)(void *on_hit_params);
    void *on_hit_params;
    bool destroyed;
};

struct projectile *projectile_create(struct game *game, unsigned char glyph, int level, float x, float y, float dx, float dy, struct actor *shooter, void (*on_hit)(void *on_hit_params), void *on_hit_params);
void projectile_update(struct projectile *projectile);
void projectile_destroy(struct projectile *projectile);

#endif
