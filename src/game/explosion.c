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
    explosion->hashes = TCOD_list_new();
    return explosion;
}

bool explosion_update(struct explosion *explosion, float delta_time)
{
    explosion->current_radius += 10 * delta_time;

    for (float theta = 0.0f; theta < 360.0f; theta++)
    {
        int x = (int)roundf(explosion->x + explosion->current_radius * cosf(theta));
        int y = (int)roundf(explosion->y + explosion->current_radius * sinf(theta));
        if (!map_is_inside(x, y))
        {
            continue;
        }

        long long hash = (x + y) * (x + y + 1) / 2 + x;
        if (TCOD_list_contains(explosion->hashes, (void *)hash))
        {
            continue;
        }
        else
        {
            TCOD_list_push(explosion->hashes, (void *)hash);
        }

        struct map *map = &world->maps[explosion->floor];
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
    TCOD_list_delete(explosion->hashes);
    free(explosion);
}
