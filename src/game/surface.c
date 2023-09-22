#include "surface.h"

#include <malloc.h>

struct surface *surface_new(
    const enum surface_type type,
    const uint8_t x,
    const uint8_t y)
{
    struct surface *const surface = malloc(sizeof(*surface));

    surface->type = type;

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
