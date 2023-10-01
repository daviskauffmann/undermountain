#include "base_item.h"

#include "armor_proficiency.h"
#include "damage_type.h"
#include "equip_slot.h"
#include "size.h"

const struct base_item_data base_item_database[] = {
    [BASE_ITEM_TYPE_ARROW] = {
        .name = "Arrow",
        .glyph = '`',

        .equip_slot = EQUIP_SLOT_AMMUNITION,
        .size = SIZE_SMALL,
        .weight = 0.1f,
        .max_stack = 20,
        .cost = 1,

        .ammunition_type = AMMUNITION_TYPE_ARROW,
    },
    [BASE_ITEM_TYPE_BATTLEAXE] = {
        .name = "Battleaxe",
        .glyph = 'T',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_MEDIUM,
        .weight = 6,
        .max_stack = 1,
        .cost = 10,

        .damage = "1d8",
        .damage_type = DAMAGE_TYPE_SLASHING,
        .threat_range = 19,
        .critical_multiplier = 3,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_MARTIAL] = true,
        },
    },
    [BASE_ITEM_TYPE_BREASTPLATE] = {
        .name = "Breastplate",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_MEDIUM,
        .weight = 30,
        .max_stack = 1,
        .cost = 200,

        .armor_class = 5,
        .arcane_spell_failure = 0.25f,
        .max_dexterity_bonus = 3,
        .armor_proficiency = ARMOR_PROFICIENCY_MEDIUM,
    },
    [BASE_ITEM_TYPE_BOLT] = {
        .name = "Bolt",
        .glyph = '`',

        .equip_slot = EQUIP_SLOT_AMMUNITION,
        .size = SIZE_SMALL,
        .weight = 0.1f,
        .max_stack = 5,
        .cost = 1,

        .ammunition_type = AMMUNITION_TYPE_BOLT,
    },
    [BASE_ITEM_TYPE_BULLET] = {
        .name = "Bullet",
        .glyph = '`',

        .equip_slot = EQUIP_SLOT_AMMUNITION,
        .size = SIZE_TINY,
        .weight = 0.1f,
        .max_stack = 10,
        .cost = 1,

        .ammunition_type = AMMUNITION_TYPE_BULLET,
    },
    [BASE_ITEM_TYPE_CLOTHING] = {
        .name = "Armor",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_MEDIUM,
        .weight = 1,
        .max_stack = 1,
        .cost = 5,
    },
    [BASE_ITEM_TYPE_DAGGER] = {
        .name = "Dagger",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_TINY,
        .weight = 1,
        .max_stack = 1,
        .cost = 2,

        .damage = "1d4",
        .damage_type = DAMAGE_TYPE_PIERCING,
        .threat_range = 19,
        .critical_multiplier = 2,
        .finesssable = true,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_ROGUE] = true,
            [WEAPON_PROFICIENCY_SIMPLE] = true,
            [WEAPON_PROFICIENCY_WIZARD] = true,
        },
    },
    [BASE_ITEM_TYPE_FALCHION] = {
        .name = "Falchion",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,
        .weight = 8,
        .max_stack = 1,
        .cost = 75,

        .damage = "2d4",
        .damage_type = DAMAGE_TYPE_SLASHING,
        .threat_range = 18,
        .critical_multiplier = 2,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_ELF] = true,
            [WEAPON_PROFICIENCY_MARTIAL] = true,
        },
    },
    [BASE_ITEM_TYPE_FOOD] = {
        .name = "Food",
        .glyph = '#',

        .size = SIZE_TINY,
        .weight = 0.1f,
        .max_stack = INT32_MAX,
        .cost = 1,
    },
    [BASE_ITEM_TYPE_FULL_PLATE] = {
        .name = "Full Plate",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_LARGE,
        .weight = 50,
        .max_stack = 1,
        .cost = 1500,

        .armor_class = 8,
        .arcane_spell_failure = 0.35f,
        .max_dexterity_bonus = 1,
        .armor_proficiency = ARMOR_PROFICIENCY_HEAVY,
    },
    [BASE_ITEM_TYPE_GOLD] = {
        .name = "Gold",
        .glyph = '$',

        .size = SIZE_TINY,
        .weight = 0.1f,
        .max_stack = INT32_MAX,
        .cost = 1,
    },
    [BASE_ITEM_TYPE_GREATCLUB] = {
        .name = "Greatclub",
        .glyph = '!',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,
        .weight = 8,
        .max_stack = 1,
        .cost = 5,

        .damage = "1d10",
        .damage_type = DAMAGE_TYPE_BLUDGEONING,
        .threat_range = 20,
        .critical_multiplier = 2,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_MARTIAL] = true,
        },
    },
    [BASE_ITEM_TYPE_GREATSWORD] = {
        .name = "Greatsword",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,
        .weight = 8,
        .max_stack = 1,
        .cost = 50,

        .damage = "2d6",
        .damage_type = DAMAGE_TYPE_SLASHING,
        .threat_range = 19,
        .critical_multiplier = 2,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_MARTIAL] = true,
        },
    },
    [BASE_ITEM_TYPE_HALF_PLATE] = {
        .name = "Half Plate",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_LARGE,
        .weight = 50,
        .max_stack = 1,
        .cost = 600,

        .armor_class = 7,
        .arcane_spell_failure = 0.4f,
        .max_dexterity_bonus = 0,
        .armor_proficiency = ARMOR_PROFICIENCY_HEAVY,
    },
    [BASE_ITEM_TYPE_HEAVY_CROSSBOW] = {
        .name = "Heavy Crossbow",
        .glyph = 'T',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_MEDIUM,
        .weight = 8,
        .max_stack = 1,
        .cost = 50,

        .damage = "1d10",
        .damage_type = DAMAGE_TYPE_PIERCING,
        .threat_range = 19,
        .critical_multiplier = 2,
        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_BOLT,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_SIMPLE] = true,
            [WEAPON_PROFICIENCY_WIZARD] = true,
        },
    },
    [BASE_ITEM_TYPE_HEAVY_MACE] = {
        .name = "Heavy Mace",
        .glyph = '!',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_MEDIUM,
        .weight = 8,
        .max_stack = 1,
        .cost = 12,

        .damage = "1d8",
        .damage_type = DAMAGE_TYPE_BLUDGEONING,
        .threat_range = 20,
        .critical_multiplier = 2,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_SIMPLE] = true,
        },
    },
    [BASE_ITEM_TYPE_HEAVY_SHIELD] = {
        .name = "Heavy Shield",
        .glyph = ')',

        .equip_slot = EQUIP_SLOT_SHIELD,
        .size = SIZE_MEDIUM,
        .weight = 5,
        .max_stack = 1,
        .cost = 20,

        .armor_class = 2,
        .arcane_spell_failure = 0.15f,
        .armor_proficiency = ARMOR_PROFICIENCY_SHIELD,
    },
    [BASE_ITEM_TYPE_HIDE_ARMOR] = {
        .name = "Hide Armor",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_MEDIUM,
        .weight = 25,
        .max_stack = 1,
        .cost = 15,

        .armor_class = 3,
        .arcane_spell_failure = 0.20f,
        .max_dexterity_bonus = 4,
        .armor_proficiency = ARMOR_PROFICIENCY_MEDIUM,
    },
    [BASE_ITEM_TYPE_JAVELIN] = {
        .name = "Javelin",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_MEDIUM,
        .weight = 2,
        .max_stack = 5,
        .cost = 50,

        .damage = "1d6",
        .damage_type = DAMAGE_TYPE_PIERCING,
        .threat_range = 20,
        .critical_multiplier = 2,
        .ranged = true,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_SIMPLE] = true,
        },
    },
    [BASE_ITEM_TYPE_LEATHER_ARMOR] = {
        .name = "Leather Armor",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_MEDIUM,
        .weight = 15,
        .max_stack = 1,
        .cost = 10,

        .armor_class = 2,
        .arcane_spell_failure = 0.10f,
        .max_dexterity_bonus = 6,
        .armor_proficiency = ARMOR_PROFICIENCY_LIGHT,
    },
    [BASE_ITEM_TYPE_LIGHT_CROSSBOW] = {
        .name = "Light Crossbow",
        .glyph = 't',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_SMALL,
        .weight = 4,
        .max_stack = 1,
        .cost = 35,

        .damage = "1d8",
        .damage_type = DAMAGE_TYPE_PIERCING,
        .threat_range = 19,
        .critical_multiplier = 2,
        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_BOLT,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_SIMPLE] = true,
            [WEAPON_PROFICIENCY_WIZARD] = true,
        },
    },
    [BASE_ITEM_TYPE_LIGHT_MACE] = {
        .name = "Light Mace",
        .glyph = '!',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_SMALL,
        .weight = 4,
        .max_stack = 1,
        .cost = 5,

        .damage = "1d6",
        .damage_type = DAMAGE_TYPE_BLUDGEONING,
        .threat_range = 20,
        .critical_multiplier = 2,
        .finesssable = true,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_ROGUE] = true,
            [WEAPON_PROFICIENCY_SIMPLE] = true,
        },
    },
    [BASE_ITEM_TYPE_LIGHT_SHIELD] = {
        .name = "Light Shield",
        .glyph = ')',

        .equip_slot = EQUIP_SLOT_SHIELD,
        .size = SIZE_SMALL,
        .weight = 5,
        .max_stack = 1,
        .cost = 9,

        .armor_class = 1,
        .arcane_spell_failure = 0.05f,
        .armor_proficiency = ARMOR_PROFICIENCY_SHIELD,
    },
    [BASE_ITEM_TYPE_LONGBOW] = {
        .name = "Longbow",
        .glyph = '}',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,
        .weight = 3,
        .max_stack = 1,
        .cost = 75,

        .damage = "1d8",
        .damage_type = DAMAGE_TYPE_PIERCING,
        .threat_range = 20,
        .critical_multiplier = 3,
        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_ARROW,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_ELF] = true,
            [WEAPON_PROFICIENCY_MARTIAL] = true,
        },
    },
    [BASE_ITEM_TYPE_LONGSWORD] = {
        .name = "Longsword",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_MEDIUM,
        .weight = 4,
        .max_stack = 1,
        .cost = 15,

        .damage = "1d8",
        .damage_type = DAMAGE_TYPE_SLASHING,
        .threat_range = 19,
        .critical_multiplier = 2,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_ELF] = true,
            [WEAPON_PROFICIENCY_MARTIAL] = true,
        },
    },
    [BASE_ITEM_TYPE_MORNINGSTAR] = {
        .name = "Morningstar",
        .glyph = '!',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_MEDIUM,
        .weight = 6,
        .max_stack = 1,
        .cost = 8,

        .damage = "1d8",
        .damage_type = DAMAGE_TYPE_BLUDGEONING,
        .threat_range = 20,
        .critical_multiplier = 2,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_SIMPLE] = true,
        },
    },
    [BASE_ITEM_TYPE_POTION] = {
        .name = "Potion",
        .glyph = '!',

        .size = SIZE_SMALL,
        .weight = 0.5f,
        .max_stack = 10,
    },
    [BASE_ITEM_TYPE_SCIMITAR] = {
        .name = "Scimitar",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_MEDIUM,
        .weight = 4,
        .max_stack = 1,
        .cost = 15,

        .damage = "1d6",
        .damage_type = DAMAGE_TYPE_SLASHING,
        .threat_range = 18,
        .critical_multiplier = 2,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_MARTIAL] = true,
        },
    },
    [BASE_ITEM_TYPE_SCROLL] = {
        .name = "Scroll",
        .glyph = '%',

        .size = SIZE_SMALL,
        .weight = 0.5f,
        .max_stack = 10,
    },
    [BASE_ITEM_TYPE_SHORTBOW] = {
        .name = "Shortbow",
        .glyph = '}',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_MEDIUM,
        .weight = 2,
        .max_stack = 1,
        .cost = 30,

        .damage = "1d6",
        .damage_type = DAMAGE_TYPE_PIERCING,
        .threat_range = 20,
        .critical_multiplier = 3,
        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_ARROW,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_ELF] = true,
            [WEAPON_PROFICIENCY_MARTIAL] = true,
        },
    },
    [BASE_ITEM_TYPE_SHORTSPEAR] = {
        .name = "Shortspear",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_MEDIUM,
        .weight = 3,
        .max_stack = 1,
        .cost = 1,

        .damage = "1d6",
        .damage_type = DAMAGE_TYPE_PIERCING,
        .threat_range = 20,
        .critical_multiplier = 2,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_SIMPLE] = true,
        },
    },
    [BASE_ITEM_TYPE_SLING] = {
        .name = "Sling",
        .glyph = '?',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_SMALL,
        .weight = 0,
        .max_stack = 1,
        .cost = 0,

        .damage = "1d4",
        .damage_type = DAMAGE_TYPE_BLUDGEONING,
        .threat_range = 20,
        .critical_multiplier = 2,
        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_BULLET,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_ROGUE] = true,
            [WEAPON_PROFICIENCY_SIMPLE] = true,
        },
    },
    [BASE_ITEM_TYPE_SPEAR] = {
        .name = "Spear",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,
        .weight = 6,
        .max_stack = 1,
        .cost = 2,

        .damage = "1d8",
        .damage_type = DAMAGE_TYPE_PIERCING,
        .threat_range = 20,
        .critical_multiplier = 3,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_SIMPLE] = true,
        },
    },
    [BASE_ITEM_TYPE_STUDDED_LEATHER_ARMOR] = {
        .name = "Studded Leather Armor",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_MEDIUM,
        .weight = 20,
        .max_stack = 1,
        .cost = 25,

        .armor_class = 3,
        .arcane_spell_failure = 0.15f,
        .max_dexterity_bonus = 5,
        .armor_proficiency = ARMOR_PROFICIENCY_LIGHT,
    },
    [BASE_ITEM_TYPE_TOME] = {
        .name = "Tome",
        .glyph = '=',

        .size = SIZE_SMALL,
        .weight = 0.5f,
        .max_stack = 1,
    },
    [BASE_ITEM_TYPE_TOWER_SHIELD] = {
        .name = "Tower Shield",
        .glyph = ')',

        .equip_slot = EQUIP_SLOT_SHIELD,
        .size = SIZE_LARGE,
        .weight = 45,
        .max_stack = 1,
        .cost = 30,

        .armor_class = 3,
        .arcane_spell_failure = 0.5f,
        .armor_proficiency = ARMOR_PROFICIENCY_SHIELD,
    },
};
