#include "item.h"

#include "assets.h"
#include <assert.h>
#include <malloc.h>

struct item *item_new(
    const enum item_type type,
    const uint8_t floor,
    const uint8_t x,
    const uint8_t y,
    const int stack)
{
    struct item *const item = malloc(sizeof(*item));
    assert(item);

    item->type = type;

    item->floor = floor;
    item->x = x;
    item->y = y;

    item->stack = CLAMP(0, base_item_data[item_data[type].type].max_stack, stack);

    item->durability = base_item_data[item_data[type].type].max_durability;

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
