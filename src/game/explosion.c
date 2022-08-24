#include "explosion.h"

#include "actor.h"
#include "world.h"
#include <assert.h>
#include <malloc.h>
#include <math.h>

struct explosion *explosion_new(
    const uint8_t floor,
    const int x,
    const int y,
    const int radius,
    const TCOD_color_t color,
    struct actor *const initiator)
{
    struct explosion *const explosion = malloc(sizeof(*explosion));
    assert(explosion);

    explosion->floor = floor;
    explosion->x = x;
    explosion->y = y;

    explosion->radius = radius;
    explosion->color = color;
    explosion->lifetime = 0.0f;

    const struct map *const map = &world->maps[explosion->floor];
    explosion->fov = map_to_fov_map(
        map,
        explosion->x,
        explosion->y,
        explosion->radius);

    if (initiator)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                const struct tile *const tile = &map->tiles[x][y];
                if (tile->actor &&
                    tile->actor != initiator &&
                    TCOD_map_is_in_fov(
                        explosion->fov,
                        tile->actor->x,
                        tile->actor->y))
                {
                    world_log(
                        initiator->floor,
                        initiator->x,
                        initiator->y,
                        TCOD_white,
                        "%s hits %s for 5.",
                        initiator->name,
                        tile->actor->name);

                    actor_take_damage(tile->actor, initiator, 5);
                }
            }
        }
    }

    return explosion;
}

void explosion_delete(struct explosion *const explosion)
{
    if (explosion->fov)
    {
        TCOD_map_delete(explosion->fov);
    }

    free(explosion);
}

bool explosion_update(struct explosion *const explosion, const float delta_time)
{
    explosion->lifetime += delta_time;

    return explosion->lifetime < 0.25f;
}
