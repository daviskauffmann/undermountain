#include "item.h"

#include "data.h"
#include "world.h"
#include <malloc.h>

struct item *item_new(
    const enum item_type type,
    const int floor,
    const int x,
    const int y,
    const int stack)
{
    struct item *const item = malloc(sizeof(*item));

    item->type = type;

    item->floor = floor;
    item->x = x;
    item->y = y;

    item->stack = CLAMP(0, base_item_database[item_database[type].type].max_stack, stack);

    if (item_database[type].unique)
    {
        list_add(world->spawned_unique_item_types, (void *)(size_t)type);
    }

    return item;
}

void item_delete(struct item *const item)
{
    free(item);
}
