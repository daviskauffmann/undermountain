#ifndef UM_GAME_ITEM_H
#define UM_GAME_ITEM_H

#include "base_item.h"
#include "equip_slot.h"
#include "size.h"
#include "spell.h"
#include <libtcod.h>

enum item_type
{
    ITEM_TYPE_NONE,

    ITEM_TYPE_ADAMANTINE_BREASTPLATE,
    ITEM_TYPE_ARROW,
    ITEM_TYPE_BATTLEAXE,
    ITEM_TYPE_BREASTPLATE,
    ITEM_TYPE_BOLT,
    ITEM_TYPE_BULLET,
    ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS,
    ITEM_TYPE_DAGGER,
    ITEM_TYPE_FOOD,
    ITEM_TYPE_FULL_PLATE,
    ITEM_TYPE_GOLD,
    ITEM_TYPE_GREATSWORD,
    ITEM_TYPE_HALF_PLATE,
    ITEM_TYPE_HEAVY_CROSSBOW,
    ITEM_TYPE_HEAVY_MACE,
    ITEM_TYPE_HEAVY_SHIELD,
    ITEM_TYPE_JAVELIN,
    ITEM_TYPE_LEATHER_ARMOR,
    ITEM_TYPE_LIGHT_CROSSBOW,
    ITEM_TYPE_LIGHT_MACE,
    ITEM_TYPE_LIGHT_SHIELD,
    ITEM_TYPE_LONGBOW,
    ITEM_TYPE_LONGSWORD,
    ITEM_TYPE_MORNINGSTAR,
    ITEM_TYPE_POTION_RECOVER_LIGHT_ARCANA,
    ITEM_TYPE_SCIMITAR,
    ITEM_TYPE_SCROLL_CHAIN_LIGHTNING,
    ITEM_TYPE_SHORTBOW,
    ITEM_TYPE_SHORTSPEAR,
    ITEM_TYPE_SLING,
    ITEM_TYPE_SPEAR,
    ITEM_TYPE_STUDDED_LEATHER_ARMOR,
    ITEM_TYPE_TOME_FIREBALL,
    ITEM_TYPE_TOWER_SHIELD,
    ITEM_TYPE_WIZARDS_ROBE,

    NUM_ITEM_TYPES
};

struct item_data
{
    enum base_item_type type;

    const char *name;
    const char *description;
    TCOD_ColorRGB color;

    int enhancement_bonus;

    enum spell_type spell_type;

    int level;

    bool unique;
    bool no_spawn;
};

struct item
{
    enum item_type type;

    int floor;
    int x;
    int y;

    int stack;
};

extern const struct item_data item_database[NUM_ITEM_TYPES];

struct item *item_new(
    enum item_type type,
    int floor,
    int x,
    int y,
    int stack);
void item_delete(struct item *item);

#endif
