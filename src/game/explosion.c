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
    explosion->visited_tiles = TCOD_list_new();
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
    explosion->light_fov = map_to_fov_map(map, explosion->x, explosion->y, (int)ceilf(explosion->current_radius));

    for (float theta = 0.0f; theta < 360.0f; theta++)
    {
        int x = (int)roundf(explosion->x + explosion->current_radius * cosf(theta));
        int y = (int)roundf(explosion->y + explosion->current_radius * sinf(theta));
        if (!map_is_inside(x, y))
        {
            continue;
        }

        if (!TCOD_map_is_in_fov(explosion->light_fov, x, y))
        {
            continue;
        }

        long long hash = (x + y) * (x + y + 1) / 2 + x;
        if (TCOD_list_contains(explosion->visited_tiles, (void *)hash))
        {
            continue;
        }
        else
        {
            TCOD_list_push(explosion->visited_tiles, (void *)hash);
        }

        struct tile *tile = &map->tiles[x][y];
        if (tile->actor && tile->actor != explosion->initiator)
        {
            actor_take_damage(tile->actor, explosion->initiator, 5);
        }
    }

    return explosion->current_radius < explosion->max_radius;
}

void explosion_delete(struct explosion *explosion)
{
    if (explosion->light_fov)
    {
        TCOD_map_delete(explosion->light_fov);
    }
    TCOD_list_delete(explosion->visited_tiles);
    free(explosion);
}
