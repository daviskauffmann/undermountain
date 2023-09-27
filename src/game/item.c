#include "item.h"

#include "data.h"
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

    return item;
}

void item_delete(struct item *const item)
{
    free(item);
}
