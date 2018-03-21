#include <libtcod/libtcod.h>
#include <malloc.h>

#include "object.h"

struct object *object_create(enum object_type type, int x, int y)
{
    struct object *object = malloc(sizeof(struct object));

    object->type = type;
    object->x = x;
    object->y = y;
    object->light_fov = NULL;

    return object;
}

void object_destroy(struct object *object)
{
    if (object->light_fov != NULL)
    {
        TCOD_map_delete(object->light_fov);
    }

    free(object);
}
