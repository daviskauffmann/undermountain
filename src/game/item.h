#ifndef ITEM_H
#define ITEM_H

#include "size.h"
#include "spell.h"
#include <libtcod.h>

struct item_common
{
    char __placeholder;
};

enum equip_slot
{
    EQUIP_SLOT_NONE,

    EQUIP_SLOT_AMMUNITION,
    EQUIP_SLOT_ARMOR,
    EQUIP_SLOT_SHIELD,
    EQUIP_SLOT_WEAPON,

    NUM_EQUIP_SLOTS
};

struct equip_slot_datum
{
    char *name;
};

enum ammunition_type
{
    AMMUNITION_TYPE_NONE,

    AMMUNITION_TYPE_ARROW,
    AMMUNITION_TYPE_BOLT,
    AMMUNITION_TYPE_BULLET,
};

enum base_item_type
{
    BASE_ITEM_TYPE_ARROW,
    BASE_ITEM_TYPE_BOLT,
    BASE_ITEM_TYPE_BULLET,
    BASE_ITEM_TYPE_CROSSBOW,
    BASE_ITEM_TYPE_DAGGER,
    BASE_ITEM_TYPE_FULL_PLATE,
    BASE_ITEM_TYPE_GOLD,
    BASE_ITEM_TYPE_GREATSWORD,
    BASE_ITEM_TYPE_POTION,
    BASE_ITEM_TYPE_LARGE_SHIELD,
    BASE_ITEM_TYPE_LONGBOW,
    BASE_ITEM_TYPE_LONGSWORD,
    BASE_ITEM_TYPE_MACE,
    BASE_ITEM_TYPE_SCROLL,
    BASE_ITEM_TYPE_SLING,
    BASE_ITEM_TYPE_SMALL_SHIELD,
    BASE_ITEM_TYPE_SPEAR,
    BASE_ITEM_TYPE_TOME,
    BASE_ITEM_TYPE_TOWER_SHIELD,

    NUM_BASE_ITEM_TYPES
};

struct base_item_datum
{
    const char *name;
    unsigned char glyph;

    enum equip_slot equip_slot;
    enum size size;

    bool ranged;
    enum ammunition_type ammunition_type;

    int armor_class;

    const char *damage;
    int threat_range;
    int critical_multiplier;

    int max_stack;

    int max_durability;
};

enum item_type
{
    ITEM_TYPE_ARROW,
    ITEM_TYPE_ARROW_1,
    ITEM_TYPE_BOLT,
    ITEM_TYPE_BULLET,
    ITEM_TYPE_COLD_IRON_BLADE,
    ITEM_TYPE_CROSSBOW,
    ITEM_TYPE_DAGGER,
    ITEM_TYPE_FULL_PLATE,
    ITEM_TYPE_FULL_PLATE_1,
    ITEM_TYPE_GOLD,
    ITEM_TYPE_GREATSWORD,
    ITEM_TYPE_LARGE_SHIELD,
    ITEM_TYPE_LONGBOW,
    ITEM_TYPE_LONGBOW_1,
    ITEM_TYPE_LONGSWORD,
    ITEM_TYPE_LONGSWORD_1,
    ITEM_TYPE_MACE,
    ITEM_TYPE_POTION_MINOR_HEAL,
    ITEM_TYPE_SCEPTER_OF_UNITY,
    ITEM_TYPE_SCROLL_LIGHTNING,
    ITEM_TYPE_SLING,
    ITEM_TYPE_SMALL_SHIELD,
    ITEM_TYPE_SPEAR,
    ITEM_TYPE_SPIKED_SHIELD,
    ITEM_TYPE_TOME_FIREBALL,
    ITEM_TYPE_TOWER_SHIELD,
    ITEM_TYPE_TOWER_SHIELD_1,

    NUM_ITEM_TYPES
};

struct item_datum
{
    enum base_item_type type;

    const char *name;
    const char *description;
    TCOD_color_t color;

    int enhancement_bonus;

    enum spell_type spell_type;

    bool unique;
    bool spawned;
};

struct item
{
    enum item_type type;

    uint8_t floor;
    uint8_t x;
    uint8_t y;

    int stack;

    int durability;
};

struct item *item_new(
    enum item_type type,
    uint8_t floor,
    uint8_t x,
    uint8_t y,
    int stack);
void item_delete(struct item *item);

#endif
