#include <libtcod.h>

#include "game.h"

armor_t *armor_create(TCOD_list_t items, unsigned char glyph, TCOD_color_t color, int ac)
{
    armor_t *armor = (armor_t *)malloc(sizeof(armor_t));
    item_t *item = (item_t *)armor;

    item->glyph = glyph;
    item->color = color;
    item->type = ITEM_ARMOR;
    armor->ac = ac;

    TCOD_list_push(items, item);

    return armor;
}

weapon_t *weapon_create(TCOD_list_t items, unsigned char glyph, TCOD_color_t color, int a, int x, int b)
{
    weapon_t *weapon = (weapon_t *)malloc(sizeof(weapon_t));
    item_t *item = (item_t *)weapon;

    item->glyph = glyph;
    item->color = color;
    item->type = ITEM_WEAPON;
    weapon->a = a;
    weapon->x = x;
    weapon->b = b;

    TCOD_list_push(items, item);

    return weapon;
}

potion_t *potion_create(TCOD_list_t items, unsigned char glyph, TCOD_color_t color)
{
    potion_t *potion = (potion_t *)malloc(sizeof(potion_t));
    item_t *item = (item_t *)potion;

    item->glyph = glyph;
    item->color = color;
    item->type = ITEM_POTION;

    TCOD_list_push(items, item);

    return potion;
}

void item_turn(item_t *item)
{
}

void item_tick(item_t *item)
{
}

void item_draw_turn(item_t *item, int x, int y)
{
    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, item->color);
    TCOD_console_set_char(NULL, x - view_x, y - view_y, item->glyph);
}

void item_draw_tick(item_t *item, int x, int y)
{
}

void item_destroy(item_t *item)
{
}