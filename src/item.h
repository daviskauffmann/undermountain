#ifndef ITEM_H
#define ITEM_H

#include <libtcod/libtcod.h>

struct item_common
{
    char __placeholder;
};

enum base_item_type
{
    BASE_ITEM_LONGSWORD,

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
    WEAPON_DAMAGE_BLUGEONING,
    WEAPON_DAMAGE_PIERCING,
    WEAPON_DAMAGE_SLASHING
};

enum weapon_size
{
    WEAPON_SIZE_LARGE,
    WEAPON_SIZE_MEDIUM,
    WEAPON_SIZE_SMALL,
    WEAPON_SIZE_TINY
};

enum armor_class
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
    enum armor_class armor_class;
    int base_ac;
    int armor_check_penalty;
    int arcane_spell_failure;
};

enum item_type
{
    ITEM_COLD_IRON_LONGSWORD,
    ITEM_LONGSWORD,

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
    int num_dice_bonus;
    int die_to_roll_bonus;
    enum damage_element damage_element;
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
