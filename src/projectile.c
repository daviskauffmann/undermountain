#include <libtcod/libtcod.h>
#include <malloc.h>

#include "map.h"
#include "projectile.h"

struct projectile *projectile_create(unsigned char glyph, float x, float y, float dx, float dy, struct actor *shooter, void (*on_hit)(void *on_hit_params), void *on_hit_params)
{
    struct projectile *projectile = malloc(sizeof(struct projectile));

    projectile->glyph = glyph;
    projectile->x = x;
    projectile->y = y;
    projectile->dx = dx;
    projectile->dy = dy;
    projectile->shooter = shooter;
    projectile->on_hit = on_hit;
    projectile->on_hit_params = on_hit_params;

    return projectile;
}

void projectile_update(struct projectile *projectile)
{
    float next_x = projectile->x + projectile->dx;
    float next_y = projectile->y + projectile->dy;

    int x = (int)next_x;
    int y = (int)next_y;

    if (!map_is_inside(x, y))
    {
        return;
    }
}

void projectile_destroy(struct projectile *projectile)
{
    free(projectile);
}
