#ifndef ITEM_H
#define ITEM_H

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
    EQUIP_SLOT_MAIN_HAND,
    EQUIP_SLOT_OFF_HAND,

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

enum item_type
{
    ITEM_TYPE_ARROW,
    ITEM_TYPE_BOLT,
    ITEM_TYPE_BULLET,
    ITEM_TYPE_CLUB,
    ITEM_TYPE_COLD_IRON_BLADE,
    ITEM_TYPE_DAGGER,
    ITEM_TYPE_FULL_PLATE,
    ITEM_TYPE_GOLD,
    ITEM_TYPE_GREATSWORD,
    ITEM_TYPE_HEAVY_CROSSBOW,
    ITEM_TYPE_HEALING_POTION,
    ITEM_TYPE_LARGE_SHIELD,
    ITEM_TYPE_LIGHT_CROSSBOW,
    ITEM_TYPE_LONGBOW,
    ITEM_TYPE_LONGSWORD,
    ITEM_TYPE_MACE,
    ITEM_TYPE_MORNINGSTAR,
    ITEM_TYPE_QUARTERSTAFF,
    ITEM_TYPE_SCEPTER_OF_UNITY,
    ITEM_TYPE_SICKLE,
    ITEM_TYPE_SLING,
    ITEM_TYPE_SMALL_SHIELD,
    ITEM_TYPE_SPEAR,
    ITEM_TYPE_SPIKED_SHIELD,
    ITEM_TYPE_TOWER_SHIELD,

    NUM_ITEM_TYPES
};

struct item_datum
{
    const char *name;
    const char *description;
    unsigned char glyph;
    TCOD_color_t color;

    enum equip_slot equip_slot;
    bool two_handed;

    bool ranged;
    enum ammunition_type ammunition_type;

    int armor_class;

    const char *damage;
    int threat_range;
    int critical_multiplier;

    bool quaffable;

    int max_stack;

    int max_durability;

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
