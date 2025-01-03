#include "surface.h"

#include "color.h"
#include "light.h"
#include "world.h"
#include <malloc.h>

const struct surface_data surface_database[] = {
    [SURFACE_TYPE_ACID] = {
        .name = "Acid",
        .glyph = '\0',
        .color = {COLOR_LIME},

        .duration = 20,

        .damage = "1d2",

        .light_type = LIGHT_TYPE_ACID,
    },
    [SURFACE_TYPE_FIRE] = {
        .name = "Fire",
        .glyph = '\0',
        .color = {COLOR_FLAME},

        .duration = 20,

        .damage = "1d4",

        .light_type = LIGHT_TYPE_FIRE,
    },
    [SURFACE_TYPE_WATER] = {
        .name = "Water",
        .glyph = '~',
        .color = {COLOR_AZURE},

        .light_type = LIGHT_TYPE_NONE,
    },
};

struct surface *surface_new(
    const enum surface_type type,
    const int floor,
    const int x,
    const int y,
    struct actor *const initiator)
{
    struct surface *const surface = malloc(sizeof(*surface));

    surface->type = type;

    surface->floor = floor;
    surface->x = x;
    surface->y = y;

    surface->time = 0;

    surface->initiator = initiator;

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

void surface_update_light(struct surface *const surface)
{
    if (surface->light_fov)
    {
        TCOD_map_delete(surface->light_fov);
        surface->light_fov = NULL;
    }

    const int radius = light_database[surface_database[surface->type].light_type].radius;

    if (radius >= 0)
    {
        surface->light_fov = map_to_fov_map(
            &world->maps[surface->floor],
            surface->x,
            surface->y,
            radius);
    }
}
