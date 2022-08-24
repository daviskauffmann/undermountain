#include "object.h"

#include "world.h"
#include <assert.h>
#include <malloc.h>

struct object *object_new(
    const enum object_type type,
    const uint8_t floor,
    const uint8_t x,
    const uint8_t y,
    const TCOD_color_t color,
    const int light_radius,
    const TCOD_color_t light_color,
    const float light_intensity,
    const bool light_flicker)
{
    struct object *const object = malloc(sizeof(*object));
    assert(object);

    object->type = type;

    object->floor = floor;
    object->x = x;
    object->y = y;

    object->color = color;

    object->light_radius = light_radius;
    object->light_color = light_color;
    object->light_intensity = light_intensity;
    object->light_flicker = light_flicker;
    object->light_fov = NULL;

    return object;
}

void object_delete(struct object *const object)
{
    if (object->light_fov)
    {
        TCOD_map_delete(object->light_fov);
    }

    free(object);
}

void object_calc_light(struct object *const object)
{
    if (object->light_fov)
    {
        TCOD_map_delete(object->light_fov);
        object->light_fov = NULL;
    }

    if (object->light_radius >= 0)
    {
        object->light_fov = map_to_fov_map(
            &world->maps[object->floor],
            object->x,
            object->y,
            object->light_radius);
    }
}
