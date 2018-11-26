#include <roguelike/roguelike.h>

struct object *object_create(enum object_type type, int floor, int x, int y, TCOD_color_t color, int light_radius, TCOD_color_t light_color, bool light_flicker)
{
    struct object *object = calloc(1, sizeof(struct object));

    object->type = type;
    object->floor = floor;
    object->x = x;
    object->y = y;
    object->color = color;
    object->light_radius = light_radius;
    object->light_color = light_color;
    object->light_flicker = light_flicker;
    object->light_fov = NULL;
    object->destroyed = false;

    return object;
}

void object_calc_light(struct object *object)
{
    struct map *map = &game->maps[object->floor];

    if (object->light_fov != NULL)
    {
        TCOD_map_delete(object->light_fov);
    }

    if (object->light_radius != -1)
    {
        object->light_fov = map_to_fov_map(map, object->x, object->y, object->light_radius);
    }
}

void object_destroy(struct object *object)
{
    if (object->light_fov != NULL)
    {
        TCOD_map_delete(object->light_fov);
    }

    free(object);
}
