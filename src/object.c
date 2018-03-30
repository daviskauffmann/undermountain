#include <libtcod/libtcod.h>
#include <malloc.h>

#include "game.h"
#include "map.h"
#include "object.h"

struct object *object_create(enum object_type type, struct game *game, int level, int x, int y)
{
    struct object *object = malloc(sizeof(struct object));

    object->type = type;
    object->game = game;
    object->level = level;
    object->x = x;
    object->y = y;
    object->light_fov = NULL;

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

    if (game->object_info[object->type].light_radius != -1)
    {
        object->light_fov = map_to_fov_map(map, object->x, object->y, game->object_info[object->type].light_radius);
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
