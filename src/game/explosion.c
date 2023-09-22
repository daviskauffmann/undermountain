#include "explosion.h"

#include "actor.h"
#include "color.h"
#include "surface.h"
#include "world.h"
#include <malloc.h>
#include <math.h>

struct explosion *explosion_new(
    const uint8_t floor,
    const uint8_t x,
    const uint8_t y,
    const int8_t radius,
    const TCOD_ColorRGB color,
    struct actor *const initiator)
{
    struct explosion *const explosion = malloc(sizeof(*explosion));

    explosion->floor = floor;
    explosion->x = x;
    explosion->y = y;

    explosion->radius = radius;
    explosion->color = color;

    explosion->time = 0.0f;

    struct map *const map = &world->maps[explosion->floor];

    explosion->fov = map_to_fov_map(
        map,
        explosion->x,
        explosion->y,
        explosion->radius);

    for (uint8_t tile_x = 0; tile_x < MAP_WIDTH; tile_x++)
    {
        for (uint8_t tile_y = 0; tile_y < MAP_HEIGHT; tile_y++)
        {
            if (TCOD_map_is_in_fov(
                    explosion->fov,
                    tile_x,
                    tile_y))
            {
                struct tile *const tile = &map->tiles[tile_x][tile_y];

                if (tile->actor &&
                    tile->actor != initiator)
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

                struct surface *const surface = surface_new(
                    SURFACE_TYPE_FIRE,
                    tile_x,
                    tile_y);

                list_add(map->surfaces, surface);

                tile->surface = surface;
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
    explosion->time += delta_time;

    return explosion->time < 0.25f;
}
