#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

item_t *item_create(int x, int y, char *name, unsigned char glyph, TCOD_color_t color)
{
    item_t *item = (item_t *)malloc(sizeof(item_t));

    item->x = x;
    item->y = y;
    item->name = name;
    item->glyph = glyph;
    item->color = color;

    return item;
}

item_t *item_create_random(int x, int y)
{
    item_t *item;

    // TODO: item database
    switch (TCOD_random_get_int(NULL, 0, 2))
    {
    case 0:
        item = item_create(x, y, "Shield", ')', TCOD_white);

        break;

    case 1:
        item = item_create(x, y, "Sword", '|', TCOD_white);

        break;

    case 2:
        item = item_create(x, y, "Potion", '!', TCOD_color_RGB(TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255)));

        break;
    }

    return item;
}

void item_update(item_t *item)
{
}

void item_draw(item_t *item)
{
    if (TCOD_map_is_in_fov(player->fov_map, item->x, item->y))
    {
        TCOD_console_set_char_foreground(NULL, item->x - view_x, item->y - view_y, item->color);
        TCOD_console_set_char(NULL, item->x - view_x, item->y - view_y, item->glyph);
    }
}

void item_destroy(item_t *item)
{
    free(item);
}