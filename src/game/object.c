#include "object.h"

#include "assets.h"
#include "world.h"
#include <malloc.h>

struct object *object_new(
    const enum object_type type,
    const uint8_t floor,
    const uint8_t x,
    const uint8_t y)
{
    struct object *const object = malloc(sizeof(*object));

    object->type = type;

    object->floor = floor;
    object->x = x;
    object->y = y;

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

    const struct light_data *const light_data = &light_database[object_database[object->type].light_type];

    if (light_data->radius >= 0)
    {
        object->light_fov = map_to_fov_map(
            &world->maps[object->floor],
            object->x,
            object->y,
            light_data->radius);
    }
}
