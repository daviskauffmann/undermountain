#include <libtcod/libtcod.h>
#include <malloc.h>
#include <math.h>

#include "actor.h"
#include "assets.h"
#include "game.h"
#include "map.h"
#include "object.h"
#include "projectile.h"
#include "util.h"

struct projectile *projectile_create(unsigned char glyph, int level, int x1, int y1, int x2, int y2, struct actor *shooter, void (*on_hit)(void *on_hit_params), void *on_hit_params)
{
    struct projectile *projectile = calloc(1, sizeof(struct projectile));

    projectile->glyph = glyph;
    projectile->level = level;
    projectile->x = (float)x1;
    projectile->y = (float)y1;
    projectile->dx = ((float)x2 - (float)x1) / distance(x1, y1, x2, y2);
    projectile->dy = ((float)y2 - (float)y1) / distance(x1, y1, x2, y2);
    projectile->shooter = shooter;
    projectile->on_hit = on_hit;
    projectile->on_hit_params = on_hit_params;
    projectile->destroyed = false;

    return projectile;
}

void projectile_update(struct projectile *projectile)
{
    float next_x = projectile->x + projectile->dx;
    float next_y = projectile->y + projectile->dy;

    int x = (int)roundf(next_x);
    int y = (int)roundf(next_y);

    bool should_move = true;

    if (!map_is_inside(x, y))
    {
        should_move = false;
    }

    struct map *map = &game->maps[projectile->level];
    struct tile *tile = &map->tiles[x][y];

    if (!tile_info[tile->type].is_walkable)
    {
        should_move = false;
    }

    if (tile->actor && tile->actor != projectile->shooter && !tile->actor->dead)
    {
        actor_attack(projectile->shooter, tile->actor, true);

        should_move = false;
    }

    if (tile->object && !object_info[tile->object->type].is_walkable && tile->object->type != OBJECT_TYPE_DOOR_OPEN)
    {
        actor_bash(projectile->shooter, tile->object);

        should_move = false;
    }

    if (should_move)
    {
        projectile->x = next_x;
        projectile->y = next_y;

        game->state = GAME_STATE_WAIT;
    }
    else
    {
        if (projectile->on_hit)
        {
            projectile->on_hit(projectile->on_hit_params);
        }

        projectile->destroyed = true;
    }
}

void projectile_destroy(struct projectile *projectile)
{
    free(projectile);
}
