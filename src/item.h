#ifndef ITEM_H
#define ITEM_H

#include <libtcod/libtcod.h>

struct item_common
{
    char __placeholder;
};

enum base_item_type
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
    BASE_ITEM_QUARTERSTAFF,
    BASE_ITEM_SCIMITAR,
    BASE_ITEM_SHORTBOW,
    BASE_ITEM_SHORTSWORD,
    BASE_ITEM_SMALL_SHIELD,
    BASE_ITEM_SPEAR,
    BASE_ITEM_TOWER_SHIELD,
    BASE_ITEM_WARHAMMER,

    NUM_BASE_ITEM_TYPES
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

enum weapon_damage
{
    WEAPON_DAMAGE_BLUDGEONING,
    WEAPON_DAMAGE_PIERCING,
    WEAPON_DAMAGE_PIERCING_AND_SLASHING,
    WEAPON_DAMAGE_SLASHING
};

enum weapon_size
{
    WEAPON_SIZE_LARGE,
    WEAPON_SIZE_MEDIUM,
    WEAPON_SIZE_SMALL,
    WEAPON_SIZE_TINY
};

enum armor_class_type
{
    ARMOR_CLASS_DEFLECTION,
    ARMOR_CLASS_DODGE,
    ARMOR_CLASS_NATURAL,
    ARMOR_CLASS_SHIELD
};

struct base_item_info
{
    unsigned char glyph;
    TCOD_color_t color;
    int weight;
    enum equip_slot equip_slot;
    enum weapon_damage weapon_damage;
    enum weapon_size weapon_size;
    bool ranged;
    int num_dice;
    int die_to_roll;
    int crit_threat;
    int crit_mult;
    int base_cost;
    int stack;
    int base_armor_class;
    int armor_check_penalty;
    int arcane_spell_failure;
    int starting_charges;
};

enum item_type
{
    ITEM_BATTLEAXE,
    ITEM_BATTLEAXE_1,
    ITEM_CLUB,
    ITEM_CLUB_1,
    ITEM_DAGGER,
    ITEM_DAGGER_1,
    ITEM_GREATAXE,
    ITEM_GREATAXE_1,
    ITEM_GREATSWORD,
    ITEM_GREATSWORD_1,
    ITEM_HALBERD,
    ITEM_HALBERD_1,
    ITEM_HEAVY_CROSSBOW,
    ITEM_HEAVY_CROSSBOW_1,
    ITEM_KATANA,
    ITEM_KATANA_1,
    ITEM_LARGE_SHIELD,
    ITEM_LARGE_SHIELD_1,
    ITEM_LIGHT_CROSSBOW,
    ITEM_LIGHT_CROSSBOW_1,
    ITEM_LONGBOW,
    ITEM_LONGBOW_1,
    ITEM_LONGSWORD,
    ITEM_LONGSWORD_1,
    ITEM_MACE,
    ITEM_MACE_1,
    ITEM_QUARTERSTAFF,
    ITEM_QUARTERSTAFF_1,
    ITEM_SCIMITAR,
    ITEM_SCIMITAR_1,
    ITEM_SHORTBOW,
    ITEM_SHORTBOW_1,
    ITEM_SHORTSWORD,
    ITEM_SHORTSWORD_1,
    ITEM_SMALL_SHIELD,
    ITEM_SMALL_SHIELD_1,
    ITEM_SPEAR,
    ITEM_SPEAR_1,
    ITEM_TOWER_SHIELD,
    ITEM_TOWER_SHIELD_1,
    ITEM_WARHAMMER,
    ITEM_WARHAMMER_1,

    NUM_ITEM_TYPES
};

enum damage_element
{
    DAMAGE_ELEMENT_ACID,
    DAMAGE_ELEMENT_COLD,
    DAMAGE_ELEMENT_ELECTRICITY,
    DAMAGE_ELEMENT_FIRE,
    DAMAGE_ELEMENT_PHYSICAL,
    DAMAGE_ELEMENT_SONIC
};

struct item_info
{
    enum base_item_type base_type;
    const char *name;
    const char *description;
    int enhancement_bonus;
    enum armor_class_type armor_class_type;
    int armor_class_bonus;
};

struct item
{
    struct game *game;
    enum item_type type;
    int level;
    int x;
    int y;
};

struct item *item_create(enum item_type type, struct game *game, int level, int x, int y);
void item_destroy(struct item *item);

#endif
