#include "projectile.h"

#include "actor.h"
#include "color.h"
#include "explosion.h"
#include "object.h"
#include "surface.h"
#include "util.h"
#include "world.h"
#include <malloc.h>
#include <math.h>
#include <stdio.h>

const struct projectile_data projectile_database[] = {
    [PROJECTILE_TYPE_ACID_SPLASH] = {
        .glyph = '*',
        .color = {COLOR_LIME},

        .light_type = LIGHT_TYPE_ACID_SPLASH,

        .speed = 30,
    },
    [PROJECTILE_TYPE_ARROW] = {
        .glyph = '`',
        .color = {COLOR_WHITE},

        .light_type = LIGHT_TYPE_NONE,

        .speed = 50,
    },
    [PROJECTILE_TYPE_FIREBALL] = {
        .glyph = '*',
        .color = {COLOR_FLAME},

        .light_type = LIGHT_TYPE_FIREBALL,

        .speed = 30,
    },
    [PROJECTILE_TYPE_MAGIC_MISSILE] = {
        .glyph = '*',
        .color = {COLOR_AZURE},

        .light_type = LIGHT_TYPE_MAGIC_MISSILE,

        .speed = 30,
    },
};

struct projectile *projectile_new(
    const enum projectile_type type,
    const int floor,
    const int origin_x,
    const int origin_y,
    const int target_x,
    const int target_y,
    struct actor *const shooter,
    struct item *const ammunition,
    const int caster_level)
{
    struct projectile *const projectile = malloc(sizeof(*projectile));

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

    projectile->caster_level = caster_level;

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
    const float next_x = projectile->x + dx * projectile_database[projectile->type].speed * delta_time;
    const float next_y = projectile->y + dy * projectile_database[projectile->type].speed * delta_time;
    const int x = (int)roundf(next_x);
    const int y = (int)roundf(next_y);

    if (!map_is_inside(x, y))
    {
        should_move = false;

        goto done;
    }

    const struct map *const map = &world->maps[projectile->floor];
    const struct tile *const tile = &map->tiles[x][y];
    if (!tile_database[tile->type].is_walkable)
    {
        should_move = false;
    }

    switch (projectile->type)
    {
    case PROJECTILE_TYPE_ACID_SPLASH:
    {
        if (tile->actor &&
            tile->actor != projectile->shooter)
        {
            should_move = false;
        }

        if (tile->object &&
            !object_database[tile->object->type].is_walkable &&
            tile->object->type != OBJECT_TYPE_DOOR_OPEN)
        {
            should_move = false;
        }

        if (!should_move)
        {
            list_add(
                map->explosions,
                explosion_new(
                    EXPLOSION_TYPE_ACID_SPLASH,
                    projectile->floor,
                    x,
                    y,
                    3,
                    projectile_database[projectile->type].color,
                    projectile->shooter,
                    projectile->caster_level));
        }
    }
    break;
    case PROJECTILE_TYPE_ARROW:
    {
        if (tile->actor &&
            tile->actor != projectile->shooter)
        {
            actor_attack(projectile->shooter, tile->actor, projectile->ammunition);

            should_move = false;
        }

        if (tile->object &&
            !object_database[tile->object->type].is_walkable &&
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
            !object_database[tile->object->type].is_walkable &&
            tile->object->type != OBJECT_TYPE_DOOR_OPEN)
        {
            should_move = false;
        }

        if (!should_move)
        {
            list_add(
                map->explosions,
                explosion_new(
                    EXPLOSION_TYPE_FIREBALL,
                    projectile->floor,
                    x,
                    y,
                    5,
                    projectile_database[projectile->type].color,
                    projectile->shooter,
                    projectile->caster_level));
        }
    }
    break;
    case PROJECTILE_TYPE_MAGIC_MISSILE:
    {
        if (tile->actor &&
            tile->actor != projectile->shooter)
        {
            int num_missiles = 1;
            if (projectile->caster_level >= 9)
            {
                num_missiles = 5;
            }
            else if (projectile->caster_level >= 7)
            {
                num_missiles = 4;
            }
            else if (projectile->caster_level >= 5)
            {
                num_missiles = 3;
            }
            else if (projectile->caster_level >= 3)
            {
                num_missiles = 2;
            }

            int damage = 0;
            for (int i = 0; i < num_missiles; ++i)
            {
                damage += TCOD_random_dice_roll_s(world->random, "1d4+1");
            }

            world_log(
                projectile->shooter->floor,
                projectile->shooter->x,
                projectile->shooter->y,
                color_white,
                "%s bombards %s for %d damage.",
                projectile->shooter->name,
                tile->actor->name,
                damage);

            actor_damage_hit_points(tile->actor, projectile->shooter, damage);

            should_move = false;
        }
    }
    break;
    }

done:
    if (should_move)
    {
        projectile->x = next_x;
        projectile->y = next_y;
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

    const struct projectile_data *const projectile_data = &projectile_database[projectile->type];
    const struct light_data *const light_data = &light_database[projectile_data->light_type];

    if (light_data->radius >= 0)
    {
        projectile->light_fov = map_to_fov_map(
            &world->maps[projectile->floor],
            (int)projectile->x,
            (int)projectile->y,
            light_data->radius);
    }
}
