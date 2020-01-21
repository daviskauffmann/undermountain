#include "item.h"

#include <assert.h>
#include <malloc.h>
#include <stdio.h>

#include "assets.h"

struct item *item_new(enum item_type type, int floor, int x, int y, int stack)
{
    struct item *item = malloc(sizeof(struct item));
    assert(item);
    item->type = type;
    item->floor = floor;
    item->x = x;
    item->y = y;
    item->current_durability = item_data[type].max_durability;
    item->current_stack = MIN(item_data[type].max_stack, stack);
    return item;
}

void item_delete(struct item *item)
{
    free(item);
}
