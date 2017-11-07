#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

item_t *item_create_random(void)
{
    item_t *item;

    // TODO: item database
    switch (TCOD_random_get_int(NULL, 0, 2))
    {
    case 0:
        item = (item_t *)armor_create(')', TCOD_white, 3);

        break;

    case 1:
        item = (item_t *)weapon_create('|', TCOD_white, 1, 8, 0);

        break;

    case 2:
        item = (item_t *)potion_create('!', TCOD_color_RGB(TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255)));

        break;
    }

    return item;
}

armor_t *armor_create(unsigned char glyph, TCOD_color_t color, int ac)
{
    armor_t *armor = (armor_t *)malloc(sizeof(armor_t));
    item_t *item = (item_t *)armor;

    item->glyph = glyph;
    item->color = color;
    item->type = ITEM_TYPE_ARMOR;
    armor->ac = ac;

    return armor;
}

weapon_t *weapon_create(unsigned char glyph, TCOD_color_t color, int a, int x, int b)
{
    weapon_t *weapon = (weapon_t *)malloc(sizeof(weapon_t));
    item_t *item = (item_t *)weapon;

    item->glyph = glyph;
    item->color = color;
    item->type = ITEM_TYPE_WEAPON;
    weapon->a = a;
    weapon->x = x;
    weapon->b = b;

    return weapon;
}

potion_t *potion_create(unsigned char glyph, TCOD_color_t color)
{
    potion_t *potion = (potion_t *)malloc(sizeof(potion_t));
    item_t *item = (item_t *)potion;

    item->glyph = glyph;
    item->color = color;
    item->type = ITEM_TYPE_POTION;

    return potion;
}

corpse_t *corpse_create(actor_t *actor)
{
    corpse_t *corpse = (corpse_t *)malloc(sizeof(corpse_t));
    item_t *item = (item_t *)corpse;

    item->glyph = '%';
    item->color = actor->color;
    item->type = ITEM_TYPE_CORPSE;

    return corpse;
}

void item_turn(item_t *item)
{
}

void item_tick(item_t *item)
{
}

void item_draw_turn(item_t *item, int x, int y)
{
    if (!TCOD_map_is_in_fov(player->fov_map, x, y))
    {
        return;
    }

    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, item->color);
    TCOD_console_set_char(NULL, x - view_x, y - view_y, item->glyph);
}

void item_draw_tick(item_t *item, int x, int y)
{
}

void item_destroy(item_t *item)
{
    free(item);
}