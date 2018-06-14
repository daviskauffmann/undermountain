#include <libtcod/libtcod.h>
#include <malloc.h>

#include "game.h"
#include "map.h"
#include "object.h"

struct object *object_create(
    enum object_type type,
    struct game *game,
    int level,
    int x,
    int y,
    TCOD_color_t color,
    int light_radius,
    TCOD_color_t light_color,
    bool light_flicker)
{
    struct object *object = malloc(sizeof(struct object));

    object->type = type;
    object->game = game;
    object->level = level;
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
    struct game *game = object->game;
    struct map *map = &game->maps[object->level];

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
