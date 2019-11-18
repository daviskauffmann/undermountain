#include "item.h"

#include <assert.h>
#include <malloc.h>
#include <stdio.h>

#include "assets.h"

struct item *item_new(enum item_type type, int floor, int x, int y)
{
    struct item *item = malloc(sizeof(struct item));
    assert(item);
    item->type = type;
    item->floor = floor;
    item->x = x;
    item->y = y;
    item->current_durability = item_datum[type].max_durability;
    return item;
}

void item_delete(struct item *item)
{
    free(item);
}
