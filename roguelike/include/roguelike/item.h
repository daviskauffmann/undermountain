#ifndef ITEM_H
#define ITEM_H

#include <libtcod/libtcod.h>

struct actor;

struct item_common
{
    char __placeholder;
};

enum base_item
{
    BASE_ITEM_BATTLEAXE,
    BASE_ITEM_CLUB,
    BASE_ITEM_DAGGER,
    BASE_ITEM_GREATAXE,
    BASE_ITEM_GREATSWORD,
    BASE_ITEM_HALBERD,
    BASE_ITEM_HEAVY_CROSSBOW,
    BASE_ITEM_KATANA,
    BASE_ITEM_LARGE_SHIELD,
    BASE_ITEM_LIGHT_CROSSBOW,
    BASE_ITEM_LONGBOW,
    BASE_ITEM_LONGSWORD,
    BASE_ITEM_MACE,
    BASE_ITEM_POTION,
    BASE_ITEM_QUARTERSTAFF,
    BASE_ITEM_SCIMITAR,
    BASE_ITEM_SHORTBOW,
    BASE_ITEM_SHORTSWORD,
    BASE_ITEM_SMALL_SHIELD,
    BASE_ITEM_SPEAR,
    BASE_ITEM_TOWER_SHIELD,
    BASE_ITEM_WARHAMMER,

    NUM_BASE_ITEMS
};

enum equip_slot
{
    EQUIP_SLOT_NONE,
    EQUIP_SLOT_ARMOR,
    EQUIP_SLOT_BELT,
    EQUIP_SLOT_BOOTS,
    EQUIP_SLOT_CLOAK,
    EQUIP_SLOT_GLOVES,
    EQUIP_SLOT_HELMET,
    EQUIP_SLOT_MAIN_HAND,
    EQUIP_SLOT_OFF_HAND,

    NUM_EQUIP_SLOTS
};

struct equip_slot_info
{
    char *name;
    char *label;
};

enum damage
{
    DAMAGE_ACID,
    DAMAGE_BLUDGEONING,
    DAMAGE_COLD,
    DAMAGE_DIVINE,
    DAMAGE_ELECTRICAL,
    DAMAGE_FIRE,
    DAMAGE_MAGICAL,
    DAMAGE_NEGATIVE,
    DAMAGE_PIERCING,
    DAMAGE_POSITIVE,
    DAMAGE_SLASHING,
    DAMAGE_SONIC
};

enum weapon_size
{
    WEAPON_SIZE_LARGE,
    WEAPON_SIZE_MEDIUM,
    WEAPON_SIZE_SMALL,
    WEAPON_SIZE_TINY
};

struct base_item_info
{
    unsigned char glyph;
    TCOD_color_t color;
    float weight;
    enum equip_slot equip_slot;
    enum damage damage;
    enum weapon_size weapon_size;
    bool ranged;
    int num_dice;
    int die_to_roll;
    int crit_threat;
    int crit_mult;
    int base_cost;
    int stack;
    int base_ac;
    int armor_check_penalty;
    int arcane_spell_failure;
    int starting_charges;
};

enum item_type
{
    ITEM_TYPE_BATTLEAXE,
    ITEM_TYPE_BATTLEAXE_1,
    ITEM_TYPE_CLUB,
    ITEM_TYPE_CLUB_1,
    ITEM_TYPE_DAGGER,
    ITEM_TYPE_DAGGER_1,
    ITEM_TYPE_GREATAXE,
    ITEM_TYPE_GREATAXE_1,
    ITEM_TYPE_GREATSWORD,
    ITEM_TYPE_GREATSWORD_1,
    ITEM_TYPE_HALBERD,
    ITEM_TYPE_HALBERD_1,
    ITEM_TYPE_HEAVY_CROSSBOW,
    ITEM_TYPE_HEAVY_CROSSBOW_1,
    ITEM_TYPE_KATANA,
    ITEM_TYPE_KATANA_1,
    ITEM_TYPE_LARGE_SHIELD,
    ITEM_TYPE_LARGE_SHIELD_1,
    ITEM_TYPE_LIGHT_CROSSBOW,
    ITEM_TYPE_LIGHT_CROSSBOW_1,
    ITEM_TYPE_LONGBOW,
    ITEM_TYPE_LONGBOW_1,
    ITEM_TYPE_LONGSWORD,
    ITEM_TYPE_LONGSWORD_1,
    ITEM_TYPE_MACE,
    ITEM_TYPE_MACE_1,
    ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS,
    ITEM_TYPE_QUARTERSTAFF,
    ITEM_TYPE_QUARTERSTAFF_1,
    ITEM_TYPE_SCIMITAR,
    ITEM_TYPE_SCIMITAR_1,
    ITEM_TYPE_SHORTBOW,
    ITEM_TYPE_SHORTBOW_1,
    ITEM_TYPE_SHORTSWORD,
    ITEM_TYPE_SHORTSWORD_1,
    ITEM_TYPE_SMALL_SHIELD,
    ITEM_TYPE_SMALL_SHIELD_1,
    ITEM_TYPE_SPEAR,
    ITEM_TYPE_SPEAR_1,
    ITEM_TYPE_TOWER_SHIELD,
    ITEM_TYPE_TOWER_SHIELD_1,
    ITEM_TYPE_WARHAMMER,
    ITEM_TYPE_WARHAMMER_1,

    NUM_ITEM_TYPES
};

enum item_property
{
    ITEM_PROPERTY_AC_SHIELD_1,
    ITEM_PROPERTY_ENHANCEMENT_BONUS_1,

    NUM_ITEM_PROPERTIES
};

enum ac
{
    AC_ARMOR,
    AC_DEFLECTION,
    AC_DODGE,
    AC_NATURAL,
    AC_SHIELD
};

struct item_property_info
{
    const char *text;
    int enhancement_bonus;
    enum ac ac;
    int ac_bonus;
    // TODO: contained spell (used for scrolls and potions)
};

struct item_info
{
    enum base_item base_item;
    const char *name;
    const char *description;
    bool item_properties[NUM_ITEM_PROPERTIES];
};

struct item
{
    enum item_type type;
    int floor;
    int x;
    int y;
};

struct item *item_create(enum item_type type, int floor, int x, int y);
bool item_is_two_handed(struct item *item, struct actor *actor);
void item_destroy(struct item *item);

#endif
