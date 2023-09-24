#include "surface.h"

#include "data.h"
#include "world.h"
#include <malloc.h>

struct surface *surface_new(
    const enum surface_type type,
    const int floor,
    const int x,
    const int y)
{
    struct surface *const surface = malloc(sizeof(*surface));

    surface->type = type;

    surface->floor = floor;
    surface->x = x;
    surface->y = y;

    surface->time = 0;

    surface->light_fov = NULL;

    return surface;
}

void surface_delete(struct surface *surface)
{
    if (surface->light_fov)
    {
        TCOD_map_delete(surface->light_fov);
    }

    free(surface);
}

void surface_calc_light(struct surface *const surface)
{
    if (surface->light_fov)
    {
        TCOD_map_delete(surface->light_fov);
        surface->light_fov = NULL;
    }

    const struct light_data *const light_data = &light_database[surface_database[surface->type].light_type];

    if (light_data->radius >= 0)
    {
        surface->light_fov = map_to_fov_map(
            &world->maps[surface->floor],
            surface->x,
            surface->y,
            light_data->radius);
    }
}
