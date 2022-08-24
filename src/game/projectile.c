#include "projectile.h"

#include "actor.h"
#include "assets.h"
#include "explosion.h"
#include "util.h"
#include "world.h"
#include <assert.h>
#include <malloc.h>
#include <math.h>
#include <stdio.h>

// TODO: scale damage over distance?

struct projectile *projectile_new(
    const enum projectile_type type,
    const uint8_t floor,
    const int origin_x,
    const int origin_y,
    const int target_x,
    const int target_y,
    struct actor *const shooter,
    struct item *const ammunition)
{
    struct projectile *const projectile = malloc(sizeof(*projectile));
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

void projectile_delete(struct projectile *const projectile)
{
    if (projectile->light_fov)
    {
        TCOD_map_delete(projectile->light_fov);
    }

    if (projectile->ammunition)
    {
        item_delete(projectile->ammunition);
    }

    free(projectile);
}

bool projectile_move(struct projectile *const projectile, const float delta_time)
{
    bool should_move = true;

    const float distance = distance_between(projectile->origin_x, projectile->origin_y, projectile->target_x, projectile->target_y);
    const float dx = ((float)projectile->target_x - (float)projectile->origin_x) / distance;
    const float dy = ((float)projectile->target_y - (float)projectile->origin_y) / distance;
    const float next_x = projectile->x + dx * projectile_data[projectile->type].speed * delta_time;
    const float next_y = projectile->y + dy * projectile_data[projectile->type].speed * delta_time;
    const int x = (int)roundf(next_x);
    const int y = (int)roundf(next_y);

    if (!map_is_inside(x, y))
    {
        should_move = false;

        goto done;
    }

    const struct map *const map = &world->maps[projectile->floor];
    const struct tile *const tile = &map->tiles[x][y];
    if (!tile_data[tile->type].is_walkable)
    {
        should_move = false;
    }

    switch (projectile->type)
    {
    case PROJECTILE_TYPE_ARROW:
    {
        if (tile->actor &&
            tile->actor != projectile->shooter)
        {
            actor_attack(projectile->shooter, tile->actor, projectile->ammunition);

            should_move = false;
        }

        if (tile->object &&
            !object_data[tile->object->type].is_walkable &&
            tile->object->type != OBJECT_TYPE_DOOR_OPEN)
        {
            actor_bash(projectile->shooter, tile->object);

            should_move = false;
        }
    }
    break;
    case PROJECTILE_TYPE_FIREBALL:
    {
        if (tile->actor &&
            tile->actor != projectile->shooter)
        {
            should_move = false;
        }

        if (tile->object &&
            !object_data[tile->object->type].is_walkable &&
            tile->object->type != OBJECT_TYPE_DOOR_OPEN)
        {
            should_move = false;
        }

        if (!should_move)
        {
            const struct explosion *const explosion = explosion_new(
                projectile->floor,
                x,
                y,
                10,
                projectile_data[projectile->type].color,
                projectile->shooter);
            TCOD_list_push(map->explosions, explosion);
        }
    }
    break;
    case NUM_PROJECTILE_TYPES:
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
        // TODO: i'm actually not sure why reducing the energy needs to happen here
        // i would expect the game update loop to do it when took_turn is set to true
        // however, enemies seem to fire many more projectiles than they should if this isn't done, so leaving it for now
        // tbh, enemies seem to be shooting too many projectiles anyways even with this, so idk whats going on
        projectile->shooter->energy -= 1.0f;
        projectile->shooter->took_turn = true;
    }

    return should_move;
}

void projectile_calc_light(struct projectile *const projectile)
{
    if (projectile->light_fov)
    {
        TCOD_map_delete(projectile->light_fov);
        projectile->light_fov = NULL;
    }

    const struct projectile_datum *const projectile_datum = &projectile_data[projectile->type];
    if (projectile_datum->light_radius >= 0)
    {
        projectile->light_fov = map_to_fov_map(
            &world->maps[projectile->floor],
            (int)projectile->x,
            (int)projectile->y,
            projectile_datum->light_radius);
    }
}
