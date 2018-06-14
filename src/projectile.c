#include <libtcod/libtcod.h>
#include <malloc.h>
#include <math.h>

#include "actor.h"
#include "game.h"
#include "map.h"
#include "object.h"
#include "projectile.h"
#include "util.h"

struct projectile *projectile_create(struct game *game, unsigned char glyph, int level, int x1, int y1, int x2, int y2, struct actor *shooter, void (*on_hit)(void *on_hit_params), void *on_hit_params)
{
    struct projectile *projectile = malloc(sizeof(struct projectile));

    projectile->game = game;
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

    struct game *game = projectile->game;
    struct map *map = &game->maps[projectile->level];
    struct tile *tile = &map->tiles[x][y];
    struct tile_info *tile_info = &game->tile_info[tile->type];

    if (!tile_info->is_walkable)
    {
        should_move = false;
    }

    for (void **iterator = TCOD_list_begin(tile->actors); iterator != TCOD_list_end(tile->actors); iterator++)
    {
        struct actor *actor = *iterator;

        if (actor == projectile->shooter)
        {
            continue;
        }

        if (actor->dead)
        {
            continue;
        }

        actor_attack(projectile->shooter, actor);

        should_move = false;

        break;
    }

    for (void **iterator = TCOD_list_begin(tile->objects); iterator != TCOD_list_end(tile->objects); iterator++)
    {
        struct object *object = *iterator;

        if (!game->object_info[object->type].is_walkable)
        {
            should_move = false;

            break;
        }
    }

    if (should_move)
    {
        projectile->x = next_x;
        projectile->y = next_y;

        game->turn_available = false;
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
