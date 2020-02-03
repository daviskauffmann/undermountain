#include "projectile.h"

#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>

#include "actor.h"
#include "assets.h"
#include "assets.h"
#include "world.h"
#include "util.h"

// TODO: scale damage over distance?

struct projectile *projectile_new(enum projectile_type type, int floor, int x1, int y1, int x2, int y2, struct actor *shooter, struct item *ammunition)
{
    struct projectile *projectile = malloc(sizeof(struct projectile));
    assert(projectile);
    projectile->type = type;
    projectile->floor = floor;
    projectile->angle = angle_between(x1, y1, x2, y2);
    projectile->distance = distance_between(x1, y1, x2, y2);
    projectile->x = (float)x1;
    projectile->y = (float)y1;
    projectile->dx = ((float)x2 - (float)x1) / projectile->distance;
    projectile->dy = ((float)y2 - (float)y1) / projectile->distance;
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
    float next_x = projectile->x + projectile->dx * projectile_data[projectile->type].speed * delta_time;
    float next_y = projectile->y + projectile->dy * projectile_data[projectile->type].speed * delta_time;
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
    switch (projectile->type)
    {
    case PROJECTILE_TYPE_ARROW:
    {
        if (tile->actor && tile->actor != projectile->shooter)
        {
            actor_attack(projectile->shooter, tile->actor, projectile->ammunition);
            should_move = false;
        }
        if (tile->object && !object_data[tile->object->type].is_walkable && tile->object->type != OBJECT_TYPE_DOOR_OPEN)
        {
            actor_bash(projectile->shooter, tile->object);
            should_move = false;
        }
    }
    break;
    case PROJECTILE_TYPE_FIREBALL:
    {
        if (tile->actor && tile->actor != projectile->shooter)
        {
            should_move = false;
        }
        if (tile->object && !object_data[tile->object->type].is_walkable && tile->object->type != OBJECT_TYPE_DOOR_OPEN)
        {
            should_move = false;
        }
        if (!should_move)
        {
            TCOD_LIST_FOREACH(map->actors)
            {
                struct actor *actor = *iterator;
                if (distance_between(x, y, actor->x, actor->y) < 10.0f)
                {
                    actor_take_damage(actor, projectile->shooter, 5);
                }
            }
        }
    }
    break;
    default:
        break;
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
