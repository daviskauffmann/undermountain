#include "explosion.h"

#include "actor.h"
#include "color.h"
#include "list.h"
#include "surface.h"
#include "world.h"
#include <malloc.h>
#include <math.h>

const struct explosion_data explosion_database[NUM_EXPLOSION_TYPES] = {
    [EXPLOSION_TYPE_ACID_SPLASH] = {
        .intensity = 0.5f,
        .radius = 3,
        .color = COLOR_LIME,
    },
    [EXPLOSION_TYPE_FIREBALL] = {
        .intensity = 0.5f,
        .radius = 5,
        .color = COLOR_FLAME,
    },
};

struct explosion *explosion_new(
    const enum explosion_type type,
    const int floor,
    const int x,
    const int y,
    struct actor *const initiator,
    const int caster_level)
{
    struct explosion *const explosion = malloc(sizeof(*explosion));

    explosion->type = type;

    explosion->floor = floor;
    explosion->x = x;
    explosion->y = y;

    explosion->time = 0;

    struct map *const map = &world->maps[explosion->floor];

    explosion->fov = map_to_fov_map(
        map,
        explosion->x,
        explosion->y,
        explosion_database[explosion->type].radius);

    explosion->caster_level = caster_level;

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
                        TCOD_dice_t dice = TCOD_random_dice_new("1d6");
                        dice.nb_rolls = MIN(explosion->caster_level, 10);

                        const int damage = TCOD_random_dice_roll(world->random, dice);

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
