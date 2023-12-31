#ifndef UM_GAME_BASE_ITEM_H
#define UM_GAME_BASE_ITEM_H

#include "ammunition_type.h"
#include "weapon_proficiency.h"
#include <libtcod.h>

enum base_item_type
{
    BASE_ITEM_TYPE_ARROW,
    BASE_ITEM_TYPE_BATTLEAXE,
    BASE_ITEM_TYPE_BREASTPLATE,
    BASE_ITEM_TYPE_BOLT,
    BASE_ITEM_TYPE_BULLET,
    BASE_ITEM_TYPE_CLOTHING,
    BASE_ITEM_TYPE_CLUB,
    BASE_ITEM_TYPE_COMPOSITE_BOW,
    BASE_ITEM_TYPE_DAGGER,
    BASE_ITEM_TYPE_FALCHION,
    BASE_ITEM_TYPE_FOOD,
    BASE_ITEM_TYPE_FULL_PLATE,
    BASE_ITEM_TYPE_GOLD,
    BASE_ITEM_TYPE_GREATCLUB,
    BASE_ITEM_TYPE_GREATSWORD,
    BASE_ITEM_TYPE_HALBERD,
    BASE_ITEM_TYPE_HALF_PLATE,
    BASE_ITEM_TYPE_HEAVY_CROSSBOW,
    BASE_ITEM_TYPE_HEAVY_MACE,
    BASE_ITEM_TYPE_HEAVY_SHIELD,
    BASE_ITEM_TYPE_HIDE_ARMOR,
    BASE_ITEM_TYPE_JAVELIN,
    BASE_ITEM_TYPE_LEATHER_ARMOR,
    BASE_ITEM_TYPE_LIGHT_CROSSBOW,
    BASE_ITEM_TYPE_LIGHT_MACE,
    BASE_ITEM_TYPE_LIGHT_SHIELD,
    BASE_ITEM_TYPE_LONGBOW,
    BASE_ITEM_TYPE_LONGSWORD,
    BASE_ITEM_TYPE_MORNINGSTAR,
    BASE_ITEM_TYPE_POTION,
    BASE_ITEM_TYPE_RAPIER,
    BASE_ITEM_TYPE_SCIMITAR,
    BASE_ITEM_TYPE_SCROLL,
    BASE_ITEM_TYPE_SHORTBOW,
    BASE_ITEM_TYPE_SHORTSPEAR,
    BASE_ITEM_TYPE_SLING,
    BASE_ITEM_TYPE_SPEAR,
    BASE_ITEM_TYPE_STUDDED_LEATHER_ARMOR,
    BASE_ITEM_TYPE_TOWER_SHIELD,

    NUM_BASE_ITEM_TYPES
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
    bool light;
    bool ranged;
    enum ammunition_type ammunition_type;
    bool weapon_proficiencies[NUM_WEAPON_PROFICIENCIES];

    int armor_class;
    float arcane_spell_failure;
    int max_dexterity_bonus;
    enum armor_proficiency armor_proficiency;
};

extern const struct base_item_data base_item_database[NUM_BASE_ITEM_TYPES];

#endif
