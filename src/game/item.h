#ifndef UM_GAME_ITEM_H
#define UM_GAME_ITEM_H

#include "size.h"
#include "spell.h"
#include <libtcod.h>

enum equip_slot
{
    EQUIP_SLOT_NONE,

    EQUIP_SLOT_AMMUNITION,
    EQUIP_SLOT_ARMOR,
    EQUIP_SLOT_SHIELD,
    EQUIP_SLOT_WEAPON,

    NUM_EQUIP_SLOTS
};

enum weapon_proficiency
{
    WEAPON_PROFICIENCY_NONE,

    WEAPON_PROFICIENCY_ELF,
    WEAPON_PROFICIENCY_EXOTIC,
    WEAPON_PROFICIENCY_MARTIAL,
    WEAPON_PROFICIENCY_ROGUE,
    WEAPON_PROFICIENCY_SIMPLE,
    WEAPON_PROFICIENCY_WIZARD,

    NUM_WEAPON_PROFICIENCIES
};

enum armor_proficiency
{
    ARMOR_PROFICIENCY_NONE,

    ARMOR_PROFICIENCY_HEAVY,
    ARMOR_PROFICIENCY_LIGHT,
    ARMOR_PROFICIENCY_MEDIUM,
    ARMOR_PROFICIENCY_SHIELD,

    NUM_ARMOR_PROFICIENCIES
};

struct equip_slot_data
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
    BASE_ITEM_TYPE_BREASTPLATE,
    BASE_ITEM_TYPE_BOLT,
    BASE_ITEM_TYPE_BULLET,
    BASE_ITEM_TYPE_CLOTHING,
    BASE_ITEM_TYPE_DAGGER,
    BASE_ITEM_TYPE_FOOD,
    BASE_ITEM_TYPE_FULL_PLATE,
    BASE_ITEM_TYPE_GOLD,
    BASE_ITEM_TYPE_GREATSWORD,
    BASE_ITEM_TYPE_HALF_PLATE,
    BASE_ITEM_TYPE_HEAVY_CROSSBOW,
    BASE_ITEM_TYPE_HEAVY_MACE,
    BASE_ITEM_TYPE_HEAVY_SHIELD,
    BASE_ITEM_TYPE_JAVELIN,
    BASE_ITEM_TYPE_LEATHER_ARMOR,
    BASE_ITEM_TYPE_LIGHT_CROSSBOW,
    BASE_ITEM_TYPE_LIGHT_MACE,
    BASE_ITEM_TYPE_LIGHT_SHIELD,
    BASE_ITEM_TYPE_LONGBOW,
    BASE_ITEM_TYPE_LONGSWORD,
    BASE_ITEM_TYPE_MORNINGSTAR,
    BASE_ITEM_TYPE_POTION,
    BASE_ITEM_TYPE_SCIMITAR,
    BASE_ITEM_TYPE_SCROLL,
    BASE_ITEM_TYPE_SHORTSPEAR,
    BASE_ITEM_TYPE_SLING,
    BASE_ITEM_TYPE_SPEAR,
    BASE_ITEM_TYPE_STUDDED_LEATHER_ARMOR,
    BASE_ITEM_TYPE_TOME,
    BASE_ITEM_TYPE_TOWER_SHIELD,

    NUM_BASE_ITEM_TYPES
};

enum damage_type
{
    DAMAGE_TYPE_NONE,

    DAMAGE_TYPE_BLUDGEONING,
    DAMAGE_TYPE_PIERCING,
    DAMAGE_TYPE_PIERCING_SLASHING,
    DAMAGE_TYPE_SLASHING,

    NUM_DAMAGE_TYPES,
};

struct base_item_data
{
    const char *name;
    unsigned char glyph;

    enum equip_slot equip_slot;
    enum size size;
    float weight;
    int max_stack;
    int cost;

    const char *damage;
    enum damage_type damage_type;
    int threat_range;
    int critical_multiplier;
    bool finesse;
    bool ranged;
    enum ammunition_type ammunition_type;
    bool weapon_proficiencies[NUM_WEAPON_PROFICIENCIES];

    int armor_class;
    float arcane_spell_failure;
    int max_dexterity_bonus;
    enum armor_proficiency armor_proficiency;
};

enum item_type
{
    ITEM_TYPE_NONE,

    ITEM_TYPE_ADAMANTINE_BREASTPLATE,
    ITEM_TYPE_ARROW,
    ITEM_TYPE_BREASTPLATE,
    ITEM_TYPE_BOLT,
    ITEM_TYPE_BULLET,
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
    ITEM_TYPE_POTION_MINOR_HEAL,
    ITEM_TYPE_POTION_MINOR_MANA,
    ITEM_TYPE_SCIMITAR,
    ITEM_TYPE_SCROLL_LIGHTNING,
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

struct item *item_new(
    enum item_type type,
    int floor,
    int x,
    int y,
    int stack);
void item_delete(struct item *item);

#endif
