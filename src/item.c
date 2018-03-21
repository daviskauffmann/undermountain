#include <libtcod/libtcod.h>
#include <malloc.h>

#include "item.h"

struct item *item_create(enum item_type type, int x, int y)
{
    struct item *item = malloc(sizeof(struct item));

    item->type = type;
    item->x = x;
    item->y = y;

    return item;
}

void item_destroy(struct item *item)
{
    free(item);
}
