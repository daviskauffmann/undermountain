#ifndef ITEM_H
#define ITEM_H

#include <libtcod.h>

typedef enum item_type_e {
    ITEM_ARMOR,
    ITEM_WEAPON,
    ITEM_POTION
} item_type_t;

typedef struct item_s
{
    unsigned char glyph;
    TCOD_color_t color;
    item_type_t type;
} item_t;

typedef struct armor_s
{
    item_t item;
    int ac;
} armor_t;

typedef struct weapon_s
{
    item_t item;
    int a;
    int x;
    int b;
} weapon_t;

typedef struct potion_s
{
    item_t item;
} potion_t;

armor_t *armor_create(TCOD_list_t items, unsigned char glyph, TCOD_color_t color, int ac);
weapon_t *weapon_create(TCOD_list_t items, unsigned char glyph, TCOD_color_t color, int a, int x, int b);
potion_t *potion_create(TCOD_list_t items, unsigned char glyph, TCOD_color_t color);
void item_destroy(item_t *item);

#endif