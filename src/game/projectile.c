#include "projectile.h"

#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>

#include "actor.h"
#include "assets.h"
#include "explosion.h"
#include "world.h"
#include "util.h"

// TODO: scale damage over distance?

struct projectile *projectile_new(enum projectile_type type, int floor, int origin_x, int origin_y, int target_x, int target_y, struct actor *shooter, struct item *ammunition)
{
    struct projectile *projectile = malloc(sizeof *projectile);
    assert(projectile);
    projectile->type = type;
    projectile->floor = floor;
    projectile->origin_x = origin_x;
    projectile->origin_y = origin_y;
    projectile->target_x = target_x;
    projectile->target_y = target_y;
    projectile->x = (float)origin_x;
    projectile->y = (float)origin_y;
    projectile->shooter = shooter;
    projectile->ammunition = ammunition;
    projectile->light_fov = NULL;
    return projectile;
}

void projectile_delete(struct projectile *projectile)
{
    if (projectile->light_fov)
    {
        TCOD_map_delete(projectile->light_fov);
    }
    free(projectile);
}

bool projectile_move(struct projectile *projectile, float delta_time)
{
    bool should_move = true;

    float distance = distance_between(projectile->origin_x, projectile->origin_y, projectile->target_x, projectile->target_y);
    float dx = ((float)projectile->target_x - (float)projectile->origin_x) / distance;
    float dy = ((float)projectile->target_y - (float)projectile->origin_y) / distance;
    float next_x = projectile->x + dx * projectile_data[projectile->type].speed * delta_time;
    float next_y = projectile->y + dy * projectile_data[projectile->type].speed * delta_time;
    int x = (int)roundf(next_x);
    int y = (int)roundf(next_y);

    if (!map_is_inside(x, y))
    {
        should_move = false;
        goto done;
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
            struct explosion *explosion = explosion_new(projectile->floor, x, y, 10, projectile_data[projectile->type].color, projectile->shooter);
            struct map *map = &world->maps[projectile->floor];
            TCOD_list_push(map->explosions, explosion);
        }
    }
    break;
    case NUM_PROJECTILE_TYPES:
    {
    }
    break;
    }

done:
    if (should_move)
    {
        projectile->x = next_x;
        projectile->y = next_y;
    }
    else
    {
        projectile->shooter->took_turn = true;
    }

    return should_move;
}

void projectile_calc_light(struct projectile *projectile)
{
    if (projectile->light_fov)
    {
        TCOD_map_delete(projectile->light_fov);
    }

    struct projectile_datum projectile_datum = projectile_data[projectile->type];
    if (projectile_datum.light_radius >= 0)
    {
        struct map *map = &world->maps[projectile->floor];
        projectile->light_fov = map_to_fov_map(map, projectile->x, projectile->y, projectile_datum.light_radius);
    }
}
