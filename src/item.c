#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

item_t *item_create(item_type_t type, int x, int y, int stack)
{
    item_t *item = (item_t *)malloc(sizeof(item_t));

    item->type = type;
    item->x = x;
    item->y = y;
    item->stack = stack;

    return item;
}

void item_update(item_t *item)
{
}

void item_draw(item_t *item)
{
    if (TCOD_map_is_in_fov(player->fov_map, item->x, item->y))
    {
        TCOD_console_set_char_foreground(NULL, item->x - view_x, item->y - view_y, item_info[item->type].color);
        TCOD_console_set_char(NULL, item->x - view_x, item->y - view_y, item_info[item->type].glyph);
    }
}

void item_destroy(item_t *item)
{
    free(item);
}