#include "item.h"

armor_t *armor_create(unsigned char glyph, TCOD_color_t color, int ac)
{
    armor_t *armor = (armor_t *)malloc(sizeof(armor_t));
    item_t *item = (item_t *)armor;

    item->glyph = glyph;
    item->color = color;
    item->type = ITEM_ARMOR;
    armor->ac = ac;

    return armor;
}

weapon_t *weapon_create(unsigned char glyph, TCOD_color_t color, int a, int x, int b)
{
    weapon_t *weapon = (weapon_t *)malloc(sizeof(weapon_t));
    item_t *item = (item_t *)weapon;

    item->glyph = glyph;
    item->color = color;
    item->type = ITEM_WEAPON;
    weapon->a = a;
    weapon->x = x;
    weapon->b = b;

    return weapon;
}

void item_destroy(item_t *item)
{
}