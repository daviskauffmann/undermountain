#include "explosion.h"

#include "actor.h"
#include "color.h"
#include "surface.h"
#include "world.h"
#include <malloc.h>
#include <math.h>

struct explosion *explosion_new(
    const enum explosion_type type,
    const int floor,
    const int x,
    const int y,
    const int radius,
    const TCOD_ColorRGB color,
    struct actor *const initiator)
{
    struct explosion *const explosion = malloc(sizeof(*explosion));

    explosion->type = type;

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

    for (int tile_x = 0; tile_x < MAP_WIDTH; tile_x++)
    {
        for (int tile_y = 0; tile_y < MAP_HEIGHT; tile_y++)
        {
            if (TCOD_map_is_in_fov(
                    explosion->fov,
                    tile_x,
                    tile_y))
            {
                struct tile *const tile = &map->tiles[tile_x][tile_y];

                // TODO: explosion effect cannot be hardcoded here lol
                switch (explosion->type)
                {
                case EXPLOSION_TYPE_ACID_SPLASH:
                {
                    if (tile->actor &&
                        tile->actor != initiator)
                    {
                        const int damage = TCOD_random_dice_roll_s(world->random, "1d3");

                        world_log(
                            initiator->floor,
                            initiator->x,
                            initiator->y,
                            color_white,
                            "%s acidifies %s for %d damage.",
                            initiator->name,
                            tile->actor->name,
                            damage);

                        actor_damage_hit_points(tile->actor, initiator, damage);
                    }

                    // TODO: interactions with existing surfaces
                    if (!tile->surface)
                    {
                        struct surface *const surface = surface_new(
                            SURFACE_TYPE_ACID,
                            explosion->floor,
                            tile_x,
                            tile_y);

                        list_add(map->surfaces, surface);

                        tile->surface = surface;
                    }
                }
                break;
                case EXPLOSION_TYPE_FIREBALL:
                {
                    if (tile->actor &&
                        tile->actor != initiator)
                    {
                        const int damage = TCOD_random_dice_roll_s(world->random, "3d6");

                        world_log(
                            initiator->floor,
                            initiator->x,
                            initiator->y,
                            color_white,
                            "%s burns %s for %d.",
                            initiator->name,
                            tile->actor->name,
                            damage);

                        actor_damage_hit_points(tile->actor, initiator, damage);
                    }

                    // TODO: interactions with existing surfaces
                    if (!tile->surface)
                    {
                        struct surface *const surface = surface_new(
                            SURFACE_TYPE_FIRE,
                            explosion->floor,
                            tile_x,
                            tile_y);

                        list_add(map->surfaces, surface);

                        tile->surface = surface;
                    }
                }
                break;
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
    explosion->time += delta_time;

    return explosion->time < 0.25f;
}
