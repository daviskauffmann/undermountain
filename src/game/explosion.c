#include "explosion.h"

#include <assert.h>
#include <malloc.h>
#include <math.h>

#include "actor.h"
#include "world.h"

struct explosion *explosion_new(int floor, int x, int y, int max_radius, struct actor *initiator)
{
    struct explosion *explosion = malloc(sizeof(struct explosion));
    assert(explosion);
    explosion->floor = floor;
    explosion->x = x;
    explosion->y = y;
    explosion->max_radius = max_radius;
    explosion->current_radius = 0.0f;
    explosion->initiator = initiator;
    explosion->visitedTiles = TCOD_list_new();
    explosion->light_fov = NULL;
    return explosion;
}

bool explosion_update(struct explosion *explosion, float delta_time)
{
    if (explosion->light_fov)
    {
        TCOD_map_delete(explosion->light_fov);
    }
    struct map *map = &world->maps[explosion->floor];
    explosion->current_radius += 20 * delta_time;
    // explosion->light_fov = map_to_fov_map(map, explosion->x, explosion->y, (int)ceilf(explosion->current_radius));
    explosion->light_fov = map_to_TCOD_map(map);

    TCOD_map_t los_map = map_to_fov_map(map, explosion->x, explosion->y, 0);
    TCOD_list_t visistedTilesThisFrame = TCOD_list_new();
    for (float theta = 0.0f; theta < 360.0f; theta++)
    {
        int x = (int)roundf(explosion->x + explosion->current_radius * cosf(theta));
        int y = (int)roundf(explosion->y + explosion->current_radius * sinf(theta));
        if (!map_is_inside(x, y))
        {
            continue;
        }

        // if (!TCOD_map_is_in_fov(explosion->light_fov, x, y))
        if (!TCOD_map_is_in_fov(los_map, x, y))
        {
            continue;
        }

        long long hash = (x + y) * (x + y + 1) / 2 + x;
        if (!TCOD_list_contains(visistedTilesThisFrame, (void *)hash))
        {
            TCOD_map_t flame_light_fov = map_to_fov_map(map, x, y, 2);
            for (int x = 0; x < MAP_WIDTH; x++)
            {
                for (int y = 0; y < MAP_HEIGHT; y++)
                {
                    if (!TCOD_map_is_in_fov(explosion->light_fov, x, y) &&
                        TCOD_map_is_in_fov(flame_light_fov, x, y) &&
                        TCOD_map_is_in_fov(los_map, x, y))
                    {
                        TCOD_map_set_in_fov(explosion->light_fov, x, y, true);
                    }
                }
            }
            TCOD_map_delete(flame_light_fov);
        }

        if (TCOD_list_contains(explosion->visitedTiles, (void *)hash))
        {
            continue;
        }
        else
        {
            TCOD_list_push(explosion->visitedTiles, (void *)hash);
        }

        struct tile *tile = &map->tiles[x][y];
        if (tile->actor && tile->actor != explosion->initiator)
        {
            actor_take_damage(tile->actor, explosion->initiator, 5);
        }
    }
    TCOD_list_delete(visistedTilesThisFrame);
    TCOD_map_delete(los_map);

    return explosion->current_radius < explosion->max_radius;
}

void explosion_delete(struct explosion *explosion)
{
    if (explosion->light_fov)
    {
        TCOD_map_delete(explosion->light_fov);
    }
    TCOD_list_delete(explosion->visitedTiles);
    free(explosion);
}
