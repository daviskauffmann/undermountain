#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

item_t *item_create(item_type_t type, map_t *map, int x, int y, int stack)
{
    item_t *item = (item_t *)malloc(sizeof(item_t));

    item->type = type;
    item->map = map;
    item->x = x;
    item->y = y;
    item->stack = stack;
    item->torch = false;
    item->fov_map = NULL;

    item_calc_fov(item);

    return item;
}

void item_update(item_t *item)
{
    item_calc_fov(item);
}

void item_calc_fov(item_t *item)
{
    if (item->fov_map != NULL)
    {
        TCOD_map_delete(item->fov_map);
    }

    if (item->torch)
    {
        item->fov_map = map_to_fov_map(item->map, item->x, item->y, item_common.torch_radius);
    }
    else
    {
        item->fov_map = map_to_TCOD_map(item->map);
    }
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