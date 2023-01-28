#include "explosion.h"

#include "actor.h"
#include "color.h"
#include "world.h"
#include <malloc.h>
#include <math.h>

struct explosion *explosion_new(
    const uint8_t floor,
    const uint8_t x,
    const uint8_t y,
    const int radius,
    const TCOD_ColorRGB color,
    struct actor *const initiator)
{
    struct explosion *const explosion = malloc(sizeof(*explosion));

    explosion->floor = floor;
    explosion->x = x;
    explosion->y = y;

    explosion->radius = radius;
    explosion->color = color;
    explosion->lifetime = 0.0f;

    explosion->fov = map_to_fov_map(
        &world->maps[explosion->floor],
        explosion->x,
        explosion->y,
        explosion->radius);

    if (initiator)
    {
        explosion_deal_damage(explosion, initiator);
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

void explosion_deal_damage(const struct explosion *const explosion, struct actor *const initiator)
{
    const struct map *const map = &world->maps[explosion->floor];

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
                    color_white,
                    "%s hits %s for 5.",
                    initiator->name,
                    tile->actor->name);

                actor_damage_hit_points(tile->actor, initiator, 5);
            }
        }
    }
}

bool explosion_update(struct explosion *const explosion, const float delta_time)
{
    explosion->lifetime += delta_time;

    return explosion->lifetime < 0.25f;
}
