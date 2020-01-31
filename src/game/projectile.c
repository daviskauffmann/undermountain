#include "projectile.h"

#include <assert.h>
#include <malloc.h>
#include <math.h>

#include "actor.h"
#include "assets.h"
#include "world.h"
#include "util.h"

struct projectile *projectile_new(unsigned char glyph, int floor, int x1, int y1, int x2, int y2, struct actor *shooter, struct item *ammunition)
{
    struct projectile *projectile = malloc(sizeof(struct projectile));
    assert(projectile);
    projectile->glyph = glyph;
    projectile->floor = floor;
    projectile->x = (float)x1;
    projectile->y = (float)y1;
    float distance = distance_between(x1, y1, x2, y2);
    projectile->dx = ((float)x2 - (float)x1) / distance;
    projectile->dy = ((float)y2 - (float)y1) / distance;
    projectile->shooter = shooter;
    projectile->ammunition = ammunition;
    projectile->destroyed = false;
    return projectile;
}

void projectile_delete(struct projectile *projectile)
{
    free(projectile);
}

void projectile_update(struct projectile *projectile, float delta_time)
{
    bool should_move = true;
    float next_x = projectile->x + projectile->dx * 50.0f * delta_time;
    float next_y = projectile->y + projectile->dy * 50.0f * delta_time;
    int x = (int)roundf(next_x);
    int y = (int)roundf(next_y);
    if (!map_is_inside(x, y))
    {
        should_move = false;
    }
    struct map *map = &world->maps[projectile->floor];
    struct tile *tile = &map->tiles[x][y];
    if (!tile_data[tile->type].is_walkable)
    {
        should_move = false;
    }
    if (tile->actor && tile->actor != projectile->shooter && !tile->actor->dead)
    {
        actor_attack(projectile->shooter, tile->actor, projectile->ammunition);
        should_move = false;
    }
    if (tile->object && !object_data[tile->object->type].is_walkable && tile->object->type != OBJECT_TYPE_DOOR_OPEN)
    {
        actor_bash(projectile->shooter, tile->object);
        should_move = false;
    }
    if (should_move)
    {
        projectile->x = next_x;
        projectile->y = next_y;
    }
    else
    {
        projectile->shooter->took_turn = true;
        projectile->destroyed = true;
    }
}
