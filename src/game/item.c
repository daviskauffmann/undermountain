#include "item.h"

#include "assets.h"
#include <assert.h>
#include <malloc.h>

struct item *item_new(
    const enum item_type type,
    const uint8_t floor,
    const int x,
    const int y,
    const int stack)
{
    struct item *const item = malloc(sizeof(*item));
    assert(item);

    item->type = type;

    item->floor = floor;
    item->x = x;
    item->y = y;

    item->current_durability = item_data[type].max_durability;
    item->current_stack = MIN(item_data[type].max_stack, stack);

    if (item_data[type].unique)
    {
        // assert(!item_data[type].spawned);
        item_data[type].spawned = true;
    }

    return item;
}

void item_delete(struct item *const item)
{
    free(item);
}
