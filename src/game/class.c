#include "class.h"

#include "base_attack_bonus.h"
#include "color.h"
#include "monster.h"
#include "natural_weapon.h"

const struct class_data class_database[] = {
    // player classes
    [CLASS_CLERIC] = {
        .name = "Cleric",
        .color = {COLOR_AMBER},
        .glyph = '@',

        .hit_die = "1d8",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .default_ability_scores = {
            [ABILITY_STRENGTH] = 14,
            [ABILITY_DEXTERITY] = 8,
            [ABILITY_CONSTITUTION] = 14,
            [ABILITY_INTELLIGENCE] = 10,
            [ABILITY_WISDOM] = 16,
            [ABILITY_CHARISMA] = 14,
        },

        .feat_progression = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = 1,
            [FEAT_ARMOR_PROFICIENCY_MEDIUM] = 1,
            [FEAT_ARMOR_PROFICIENCY_HEAVY] = 1,
            [FEAT_SHIELD_PROFICIENCY] = 1,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = 1,
        },

        .spellcasting_ability = ABILITY_WISDOM,
        .spell_progression = {
            [SPELL_TYPE_CURE_MINOR_WOUNDS] = 1,
            [SPELL_TYPE_CURE_LIGHT_WOUNDS] = 1,
            [SPELL_TYPE_CURE_MODERATE_WOUNDS] = 2,
            [SPELL_TYPE_CURE_SERIOUS_WOUNDS] = 3,
            [SPELL_TYPE_CURE_CRITICAL_WOUNDS] = 4,
            [SPELL_TYPE_HARM] = 6,
            [SPELL_TYPE_HEAL] = 6,
            [SPELL_TYPE_INFLICT_MINOR_WOUNDS] = 1,
            [SPELL_TYPE_INFLICT_LIGHT_WOUNDS] = 1,
            [SPELL_TYPE_INFLICT_MODERATE_WOUNDS] = 2,
            [SPELL_TYPE_INFLICT_SERIOUS_WOUNDS] = 3,
            [SPELL_TYPE_INFLICT_CRITICAL_WOUNDS] = 4,
        },

        .starting_equipment = {
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_BREASTPLATE,
            [EQUIP_SLOT_SHIELD] = ITEM_TYPE_LIGHT_SHIELD,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_HEAVY_MACE,
        },

        .starting_items = {
            [ITEM_TYPE_FOOD] = 10,
            [ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS] = 10,
            [ITEM_TYPE_POTION_RESTORE_MANA] = 10,
        },

        .companion = MONSTER_TYPE_DOG,
    },
    [CLASS_FIGHTER] = {
        .name = "Fighter",
        .color = {COLOR_BRASS},
        .glyph = '@',

        .hit_die = "1d10",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_GOOD,

        .default_ability_scores = {
            [ABILITY_STRENGTH] = 16,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 16,
            [ABILITY_INTELLIGENCE] = 10,
            [ABILITY_WISDOM] = 10,
            [ABILITY_CHARISMA] = 9,
        },

        .feat_progression = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = 1,
            [FEAT_ARMOR_PROFICIENCY_MEDIUM] = 1,
            [FEAT_ARMOR_PROFICIENCY_HEAVY] = 1,
            [FEAT_SHIELD_PROFICIENCY] = 1,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = 1,
            [FEAT_WEAPON_PROFICIENCY_MARTIAL] = 1,
        },

        .starting_equipment = {
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_FULL_PLATE,
            [EQUIP_SLOT_SHIELD] = ITEM_TYPE_HEAVY_SHIELD,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_LONGSWORD,
        },

        .starting_items = {
            [ITEM_TYPE_FOOD] = 10,
            [ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS] = 10,
        },

        .companion = MONSTER_TYPE_DOG,
    },
    [CLASS_PALADIN] = {
        .name = "Paladin",
        .color = {COLOR_LIGHTEST_PINK},
        .glyph = '@',

        .hit_die = "1d10",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_GOOD,

        .default_ability_scores = {
            [ABILITY_STRENGTH] = 15,
            [ABILITY_DEXTERITY] = 9,
            [ABILITY_CONSTITUTION] = 14,
            [ABILITY_INTELLIGENCE] = 10,
            [ABILITY_WISDOM] = 13,
            [ABILITY_CHARISMA] = 15,
        },

        .feat_progression = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = 1,
            [FEAT_ARMOR_PROFICIENCY_MEDIUM] = 1,
            [FEAT_ARMOR_PROFICIENCY_HEAVY] = 1,
            [FEAT_SHIELD_PROFICIENCY] = 1,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = 1,
            [FEAT_WEAPON_PROFICIENCY_MARTIAL] = 1,
        },

        .spellcasting_ability = ABILITY_WISDOM,
        .spell_progression = {
            [SPELL_TYPE_CURE_MINOR_WOUNDS] = 1,
            [SPELL_TYPE_CURE_LIGHT_WOUNDS] = 1,
            [SPELL_TYPE_CURE_MODERATE_WOUNDS] = 2,
            [SPELL_TYPE_CURE_SERIOUS_WOUNDS] = 3,
        },

        .starting_equipment = {
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_FULL_PLATE,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_GREATSWORD,
        },

        .starting_items = {
            [ITEM_TYPE_FOOD] = 10,
            [ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS] = 10,
            [ITEM_TYPE_POTION_RESTORE_MANA] = 10,
        },

        .companion = MONSTER_TYPE_PONY,
    },
    [CLASS_ROGUE] = {
        .name = "Rogue",
        .color = {COLOR_YELLOW},
        .glyph = '@',

        .hit_die = "1d6",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .default_ability_scores = {
            [ABILITY_STRENGTH] = 12,
            [ABILITY_DEXTERITY] = 16,
            [ABILITY_CONSTITUTION] = 14,
            [ABILITY_INTELLIGENCE] = 14,
            [ABILITY_WISDOM] = 8,
            [ABILITY_CHARISMA] = 12,
        },

        .feat_progression = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = 1,
            [FEAT_SNEAK_ATTACK] = 1,
            [FEAT_WEAPON_PROFICIENCY_ROGUE] = 1,
        },

        .starting_equipment = {
            [EQUIP_SLOT_AMMUNITION] = ITEM_TYPE_ARROW,
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_LEATHER_ARMOR,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_LONGBOW,
        },

        .starting_items = {
            [ITEM_TYPE_DAGGER] = 1,
            [ITEM_TYPE_FOOD] = 10,
            [ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS] = 10,
        },

        .companion = MONSTER_TYPE_OWL,
    },
    [CLASS_WIZARD] = {
        .name = "Wizard",
        .color = {COLOR_AZURE},
        .glyph = '@',

        .hit_die = "1d4",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_POOR,

        .default_ability_scores = {
            [ABILITY_STRENGTH] = 10,
            [ABILITY_DEXTERITY] = 14,
            [ABILITY_CONSTITUTION] = 14,
            [ABILITY_INTELLIGENCE] = 16,
            [ABILITY_WISDOM] = 12,
            [ABILITY_CHARISMA] = 10,
        },

        .feat_progression = {
            [FEAT_WEAPON_PROFICIENCY_WIZARD] = 1,
        },

        .spellcasting_ability = ABILITY_INTELLIGENCE,
        .spell_progression = {
            [SPELL_TYPE_ACID_SPLASH] = 1,
            [SPELL_TYPE_CHAIN_LIGHTNING] = 6,
            [SPELL_TYPE_DAZE] = 1,
            [SPELL_TYPE_MAGIC_MISSILE] = 1,
            [SPELL_TYPE_RAY_OF_FROST] = 1,
            [SPELL_TYPE_FIREBALL] = 3,
            [SPELL_TYPE_SUMMON_FAMILIAR] = 1,
        },

        .starting_equipment = {
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_WIZARDS_ROBE,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_DAGGER,
        },

        .starting_items = {
            [ITEM_TYPE_FOOD] = 10,
            [ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS] = 10,
            [ITEM_TYPE_POTION_RESTORE_MANA] = 10,
        },

        .companion = MONSTER_TYPE_CAT,
    },

    // monster classes
    [CLASS_BAT] = {
        .name = "Bat",
        .color = {COLOR_GRAY},
        .glyph = 'b',

        .hit_die = "1d2",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_POOR,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_BUGBEAR] = {
        .name = "Bugbear",
        .color = {COLOR_BRASS},
        .glyph = 'b',

        .hit_die = "1d8",

        .natural_armor_bonus = 3,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,
    },
    [CLASS_CAT] = {
        .name = "Cat",
        .color = {COLOR_LIGHT_ORANGE},
        .glyph = 'c',

        .hit_die = "1d4",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_POOR,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_CLAW,
    },
    [CLASS_DIRE_RAT] = {
        .name = "Dire Rat",
        .color = {COLOR_GRAY},
        .glyph = 'R',

        .hit_die = "1d8",

        .natural_armor_bonus = 1,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_DOG] = {
        .name = "Dog",
        .color = {COLOR_LIGHTEST_GRAY},
        .glyph = 'd',

        .hit_die = "1d8",

        .natural_armor_bonus = 1,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_FIRE_BEETLE] = {
        .name = "Fire Beetle",
        .color = {COLOR_FLAME},
        .glyph = 'b',

        .hit_die = "1d8",

        .natural_armor_bonus = 5,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_GIANT_ANT] = {
        .name = "Giant Ant",
        .color = {COLOR_DARK_FLAME},
        .glyph = 'a',

        .hit_die = "1d8",

        .natural_armor_bonus = 7,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_GNOLL] = {
        .name = "Gnoll",
        .color = {COLOR_DARK_AMBER},
        .glyph = 'g',

        .hit_die = "1d8",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_armor_bonus = 1,
    },
    [CLASS_GOBLIN] = {
        .name = "Goblin",
        .color = {COLOR_DARK_GREEN},
        .glyph = 'g',

        .hit_die = "1d8",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_GOOD,
    },
    [CLASS_HOBGOBLIN] = {
        .name = "Hobgoblin",
        .color = {COLOR_LIGHT_CRIMSON},
        .glyph = 'h',

        .hit_die = "1d8",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_GOOD,
    },
    [CLASS_HYENA] = {
        .name = "Hyena",
        .color = {COLOR_DARK_AMBER},
        .glyph = 'h',

        .hit_die = "1d8",

        .natural_armor_bonus = 1,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_KOBOLD] = {
        .name = "Kobold",
        .color = {COLOR_LIGHT_GRAY},
        .glyph = 'k',

        .hit_die = "1d8",

        .natural_armor_bonus = 1,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_GOOD,
    },
    [CLASS_KRENSHAR] = {
        .name = "Krenshar",
        .color = {COLOR_LIGHT_SEPIA},
        .glyph = 'k',

        .hit_die = "1d10",

        .natural_armor_bonus = 3,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_GOOD,
    },
    [CLASS_OGRE] = {
        .name = "Ogre",
        .color = {COLOR_LIGHTEST_AMBER},
        .glyph = 'O',

        .hit_die = "1d8",

        .natural_armor_bonus = 5,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,
    },
    [CLASS_ORC] = {
        .name = "Orc",
        .color = {COLOR_DARK_GREEN},
        .glyph = 'o',

        .hit_die = "1d8",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_GOOD,
    },
    [CLASS_OWL] = {
        .name = "Owl",
        .color = {COLOR_LIGHTEST_GRAY},
        .glyph = 'o',

        .hit_die = "1d8",

        .natural_armor_bonus = 2,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_TALONS,
    },
    [CLASS_PONY] = {
        .name = "Pony",
        .color = {COLOR_LIGHT_AMBER},
        .glyph = 'p',

        .hit_die = "1d8",

        .natural_armor_bonus = 2,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_HOOF,
    },
    [CLASS_RAKSHASA] = {
        .name = "Rakshasa",
        .color = {COLOR_LIGHT_SEPIA},
        .glyph = 'R',

        .hit_die = "1d8",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_GOOD,

        .natural_armor_bonus = 2,
    },
    [CLASS_RAT] = {
        .name = "Rat",
        .color = {COLOR_LIGHTEST_GRAY},
        .glyph = 'r',

        .hit_die = "1d2",

        .natural_armor_bonus = 1,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_POOR,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_RED_DRAGON_WYRMLING] = {
        .name = "Wyrmling Red Dragon",
        .color = {COLOR_LIGHT_RED},
        .glyph = 'D',

        .hit_die = "1d12",

        .natural_armor_bonus = 6,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_GOOD,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_CLAW,
    },
    [CLASS_RED_DRAGON_ADULT] = {
        .name = "Adult Red Dragon",
        .color = {COLOR_RED},
        .glyph = 'D',

        .hit_die = "1d12",

        .natural_armor_bonus = 21,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_GOOD,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_CLAW,
    },
    [CLASS_RED_DRAGON_ANCIENT] = {
        .name = "Ancient Red Dragon",
        .color = {COLOR_DARK_RED},
        .glyph = 'D',

        .hit_die = "1d12",

        .natural_armor_bonus = 33,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_GOOD,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_CLAW,
    },
    [CLASS_SKELETON_WARRIOR] = {
        .name = "Skeleton Warrior",
        .color = {COLOR_WHITE},
        .glyph = 's',

        .hit_die = "1d12",

        .natural_armor_bonus = 2,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,
    },
    [CLASS_SNAKE] = {
        .name = "Snake",
        .color = {COLOR_LIGHT_CRIMSON},
        .glyph = 's',

        .hit_die = "1d8",

        .natural_armor_bonus = 3,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_SPIDER] = {
        .name = "Spider",
        .color = {COLOR_DARK_GRAY},
        .glyph = 's',

        .hit_die = "1d8",

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
    [CLASS_TROLL] = {
        .name = "Rat",
        .color = {COLOR_GREEN},
        .glyph = 'T',

        .hit_die = "1d8",

        .natural_armor_bonus = 5,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_CLAW,
    },
    [CLASS_WOLF] = {
        .name = "Wolf",
        .color = {COLOR_LIGHT_GRAY},
        .glyph = 'w',

        .hit_die = "1d8",

        .natural_armor_bonus = 2,

        .base_attack_bonus_type = BASE_ATTACK_BONUS_TYPE_AVERAGE,

        .natural_weapon_type = NATURAL_WEAPON_TYPE_BITE,
    },
};
