#include "object.h"

#include "color.h"
#include "world.h"
#include <malloc.h>

const struct object_data object_database[] = {
    [OBJECT_TYPE_ALTAR] = {
        .name = "Altar",
        .glyph = '_',
        .color = {COLOR_WHITE},
        .light_type = LIGHT_TYPE_ALTAR,

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_BRAZIER] = {
        .name = "Brazier",
        .glyph = '*',
        .color = {COLOR_LIGHT_AMBER},
        .light_type = LIGHT_TYPE_BRAZIER,

        .is_walkable = false,
        .is_transparent = true,
    },
    [OBJECT_TYPE_CHEST] = {
        .name = "Chest",
        .glyph = '~',
        .color = {COLOR_SEPIA},

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_DOOR_CLOSED] = {
        .name = "Closed Door",
        .glyph = '+',
        .color = {COLOR_WHITE},

        .is_walkable = false,
        .is_transparent = false,
    },
    [OBJECT_TYPE_DOOR_OPEN] = {
        .name = "Open Door",
        .glyph = '-',
        .color = {COLOR_WHITE},

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_FOUNTAIN] = {
        .name = "Fountain",
        .glyph = '{',
        .color = {COLOR_LIGHT_AZURE},

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_STAIR_DOWN] = {
        .name = "Stair Down",
        .glyph = '>',
        .color = {COLOR_WHITE},
        .light_type = LIGHT_TYPE_STAIRS,

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_STAIR_UP] = {
        .name = "Stair Up",
        .glyph = '<',
        .color = {COLOR_WHITE},
        .light_type = LIGHT_TYPE_STAIRS,

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_THRONE] = {
        .name = "Throne",
        .glyph = '\\',
        .color = {COLOR_GOLD},

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_TRAP] = {
        .name = "Trap",
        .glyph = '^',
        .color = {COLOR_WHITE},

        .is_walkable = true,
        .is_transparent = true,
    },
};

struct object *object_new(
    const enum object_type type,
    const int floor,
    const int x,
    const int y)
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
