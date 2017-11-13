#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

item_t *item_create_random(int x, int y)
{
    item_t *item;

    // TODO: item database
    switch (TCOD_random_get_int(NULL, 0, 2))
    {
    case 0:
        item = (item_t *)armor_create(x, y, "Shield", ')', TCOD_white, 3);

        break;

    case 1:
        item = (item_t *)weapon_create(x, y, "Sword", '|', TCOD_white, 1, 8, 0);

        break;

    case 2:
        item = (item_t *)potion_create(x, y, "Potion", '!', TCOD_color_RGB(TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255)));

        break;
    }

    return item;
}

armor_t *armor_create(int x, int y, char *name, unsigned char glyph, TCOD_color_t color, int ac)
{
    armor_t *armor = (armor_t *)malloc(sizeof(armor_t));
    item_t *item = (item_t *)armor;

    item->x = x;
    item->y = y;
    item->name = name;
    item->glyph = glyph;
    item->color = color;
    item->type = ITEM_TYPE_ARMOR;
    armor->ac = ac;

    return armor;
}

weapon_t *weapon_create(int x, int y, char *name, unsigned char glyph, TCOD_color_t color, int a, int b, int c)
{
    weapon_t *weapon = (weapon_t *)malloc(sizeof(weapon_t));
    item_t *item = (item_t *)weapon;

    item->x = x;
    item->y = y;
    item->name = name;
    item->glyph = glyph;
    item->color = color;
    item->type = ITEM_TYPE_WEAPON;
    weapon->a = a;
    weapon->b = b;
    weapon->c = c;

    return weapon;
}

potion_t *potion_create(int x, int y, char *name, unsigned char glyph, TCOD_color_t color)
{
    potion_t *potion = (potion_t *)malloc(sizeof(potion_t));
    item_t *item = (item_t *)potion;

    item->x = x;
    item->y = y;
    item->name = name;
    item->glyph = glyph;
    item->color = color;
    item->type = ITEM_TYPE_POTION;

    return potion;
}

corpse_t *corpse_create(int x, int y, actor_t *actor)
{
    corpse_t *corpse = (corpse_t *)malloc(sizeof(corpse_t));
    item_t *item = (item_t *)corpse;

    item->x = x;
    item->y = y;
    item->name = "Corpse";
    item->glyph = '%';
    item->color = actor_color[actor->type];
    item->type = ITEM_TYPE_CORPSE;

    return corpse;
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