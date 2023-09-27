#include "data.h"

#include "color.h"
#include <libtcod.h>

const struct tile_metadata tile_metadata = {
    .ambient_light_color = {16, 16, 32},
    .ambient_light_intensity = 0.05f,
};

const struct tile_data tile_database[] = {
    [TILE_TYPE_EMPTY] = {
        .name = "Empty",
        .glyph = ' ',
        .color = {COLOR_WHITE},
        .is_walkable = true,
        .is_transparent = true,
    },
    [TILE_TYPE_FLOOR] = {
        .name = "Floor",
        .glyph = '.',
        .color = {COLOR_WHITE},
        .is_walkable = true,
        .is_transparent = true,
    },
    [TILE_TYPE_GRASS] = {
        .name = "Grass",
        .glyph = '.',
        .color = {COLOR_LIGHT_GREEN},
        .is_walkable = true,
        .is_transparent = true,
    },
    [TILE_TYPE_WALL] = {
        .name = "Wall",
        .glyph = '#',
        .color = {COLOR_WHITE},
        .is_walkable = false,
        .is_transparent = false,
    },
};

const struct light_data light_database[] = {
    [LIGHT_TYPE_NONE] = {
        .radius = -1,
        .color = {COLOR_BLACK},
        .intensity = 0,
        .flicker = false,
    },
    [LIGHT_TYPE_ACID] = {
        .radius = 2,
        .color = {COLOR_LIME},
        .intensity = 0.1f,
        .flicker = true,
    },
    [LIGHT_TYPE_ACID_SPLASH] = {
        .radius = 3,
        .color = {COLOR_LIME},
        .intensity = 0.5f,
        .flicker = true,
    },
    [LIGHT_TYPE_ALTAR] = {
        .radius = 3,
        .color = {COLOR_WHITE},
        .intensity = 0.1f,
        .flicker = false,
    },
    [LIGHT_TYPE_BRAZIER] = {
        .radius = 10,
        .color = {COLOR_LIGHT_AMBER},
        .intensity = 0.25f,
        .flicker = true,
    },
    [LIGHT_TYPE_FIRE] = {
        .radius = 2,
        .color = {COLOR_FLAME},
        .intensity = 0.1f,
        .flicker = true,
    },
    [LIGHT_TYPE_FIREBALL] = {
        .radius = 5,
        .color = {COLOR_FLAME},
        .intensity = 0.5f,
        .flicker = true,
    },
    [LIGHT_TYPE_GLOW] = {
        .radius = 5,
        .color = {COLOR_WHITE},
        .intensity = 0.1f,
        .flicker = false,
    },
    [LIGHT_TYPE_TORCH] = {
        .radius = 10,
        .color = {COLOR_LIGHT_AMBER},
        .intensity = 0.25f,
        .flicker = true,
    },
};

const struct object_data object_database[] = {
    [OBJECT_TYPE_ALTAR] = {
        .name = "Altar",
        .glyph = '_',
        .color = {COLOR_WHITE},
        .light_type = LIGHT_TYPE_ALTAR,

        .is_walkable = true,
        .is_transparent = false,
    },
    [OBJECT_TYPE_BRAZIER] = {
        .name = "Brazier",
        .glyph = '*',
        .color = {COLOR_LIGHT_AMBER},
        .light_type = LIGHT_TYPE_BRAZIER,

        .is_walkable = false,
        .is_transparent = true,
    },
    [OBJECT_TYPE_CHEST] = {
        .name = "Chest",
        .glyph = '~',
        .color = {COLOR_SEPIA},
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_DOOR_CLOSED] = {
        .name = "Closed Door",
        .glyph = '+',
        .color = {COLOR_WHITE},
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = false,
        .is_transparent = false,
    },
    [OBJECT_TYPE_DOOR_OPEN] = {
        .name = "Open Door",
        .glyph = '-',
        .color = {COLOR_WHITE},
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_FOUNTAIN] = {
        .name = "Fountain",
        .glyph = '{',
        .color = {COLOR_LIGHT_AZURE},
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_STAIR_DOWN] = {
        .name = "Stair Down",
        .glyph = '>',
        .color = {COLOR_WHITE},
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_STAIR_UP] = {
        .name = "Stair Up",
        .glyph = '<',
        .color = {COLOR_WHITE},
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_THRONE] = {
        .name = "Throne",
        .glyph = '\\',
        .color = {COLOR_GOLD},
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = true,
    },
    [OBJECT_TYPE_TRAP] = {
        .name = "Trap",
        .glyph = '^',
        .color = {COLOR_WHITE},
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = true,
    },
};

const struct actor_metadata actor_metadata = {
    .turns_to_chase = 10,
};

const struct size_data size_database[] = {
    [SIZE_FINE] = {
        .name = "Fine",

        .modifier = 8,
        .speed = 0.1f,
    },
    [SIZE_DIMINUTIVE] = {
        .name = "Diminutive",

        .modifier = 4,
        .speed = 0.25f,
    },
    [SIZE_TINY] = {
        .name = "Tiny",

        .modifier = 2,
        .speed = 0.5f,
    },
    [SIZE_SMALL] = {
        .name = "Small",

        .modifier = 1,
        .speed = 0.75f,
    },
    [SIZE_MEDIUM] = {
        .name = "Medium",

        .modifier = 0,
        .speed = 1,
    },
    [SIZE_LARGE] = {
        .name = "Large",

        .modifier = -1,
        .speed = 0.75f,
    },
    [SIZE_HUGE] = {
        .name = "Huge",

        .modifier = -2,
        .speed = 0.5f,
    },
    [SIZE_GARGANTUAN] = {
        .name = "Gargantuan",

        .modifier = -4,
        .speed = 0.25f,
    },
    [SIZE_COLOSSAL] = {
        .name = "Colossal",

        .modifier = -8,
        .speed = 0.1f,
    },
};

const struct race_data race_database[] = {
    // player races
    [RACE_DWARF] = {
        .name = "Dwarf",

        .size = SIZE_SMALL,
    },
    [RACE_ELF] = {
        .name = "Elf",

        .size = SIZE_MEDIUM,

        .feats = {
            [FEAT_LOW_LIGHT_VISION] = true,
            [FEAT_WEAPON_PROFICIENCY_ELF] = true,
        },
    },
    [RACE_HUMAN] = {
        .name = "Human",

        .size = SIZE_MEDIUM,

        .feats = {
            [FEAT_QUICK_TO_MASTER] = true,
        },
    },

    // monster races
    [RACE_ANIMAL_DIMINUTIVE] = {
        .name = "Diminutive Animal",

        .size = SIZE_DIMINUTIVE,
    },
    [RACE_ANIMAL_TINY] = {
        .name = "Small Animal",

        .size = SIZE_SMALL,
    },
    [RACE_ANIMAL_SMALL] = {
        .name = "Small Animal",

        .size = SIZE_TINY,
    },
    [RACE_DRAGON_ADULT] = {
        .name = "Wyrmling Dragon",

        .size = SIZE_MEDIUM,
    },
    [RACE_DRAGON_ADULT] = {
        .name = "Adult Dragon",

        .size = SIZE_HUGE,
    },
    [RACE_DRAGON_ADULT] = {
        .name = "Ancient Dragon",

        .size = SIZE_COLOSSAL,
    },
    [RACE_HUMANOID_SMALL] = {
        .name = "Small Humanoid",

        .size = SIZE_SMALL,
    },
    [RACE_HUMANOID_MEDIUM] = {
        .name = "Medium Humanoid",

        .size = SIZE_MEDIUM,
    },
    [RACE_UNDEAD] = {
        .name = "Undead",

        .size = SIZE_MEDIUM,
    },
};

const struct class_data class_database[] = {
    // player classes
    [CLASS_FIGHTER] = {
        .name = "Fighter",
        .color = {COLOR_BRASS},
        .glyph = '@',

        .hit_die = "1d10",
        .mana_die = "1d4",

        .base_attack_bonus_progression = BASE_ATTACK_BONUS_PROGRESSION_COMBAT,

        .default_ability_scores = {
            [ABILITY_STRENGTH] = 15,
            [ABILITY_DEXTERITY] = 12,
            [ABILITY_CONSTITUTION] = 14,
            [ABILITY_INTELLIGENCE] = 10,
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
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_BREASTPLATE,
            [EQUIP_SLOT_SHIELD] = ITEM_TYPE_LARGE_SHIELD,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_LONGSWORD,
        },
    },
    [CLASS_ROGUE] = {
        .name = "Rogue",
        .color = {COLOR_YELLOW},
        .glyph = '@',

        .hit_die = "1d6",
        .mana_die = "1d4",

        .base_attack_bonus_progression = BASE_ATTACK_BONUS_PROGRESSION_MIDDLE,

        .default_ability_scores = {
            [ABILITY_STRENGTH] = 14,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 12,
            [ABILITY_INTELLIGENCE] = 10,
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
        },
    },
    [CLASS_WIZARD] = {
        .name = "Wizard",
        .color = {COLOR_AZURE},
        .glyph = '@',

        .hit_die = "1d4",
        .mana_die = "1d10",

        .base_attack_bonus_progression = BASE_ATTACK_BONUS_PROGRESSION_NON_COMBAT,

        .default_ability_scores = {
            [ABILITY_STRENGTH] = 10,
            [ABILITY_DEXTERITY] = 14,
            [ABILITY_CONSTITUTION] = 12,
            [ABILITY_INTELLIGENCE] = 15,
        },

        .feat_progression = {
            [FEAT_WEAPON_PROFICIENCY_WIZARD] = 1,
        },

        .spell_progression = {
            [SPELL_TYPE_ACID_SPLASH] = 1,
            [SPELL_TYPE_FIREBALL] = 1,
            [SPELL_TYPE_SUMMON_FAMILIAR] = 1,
            [SPELL_TYPE_LIGHTNING] = 2,
        },

        .starting_equipment = {
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_WIZARDS_ROBE,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_DAGGER,
        },
    },

    // monster classes
    [CLASS_BAT] = {
        .name = "Bat",
        .color = {COLOR_GRAY},
        .glyph = 'b',

        .hit_die = "1d2",
        .mana_die = "0d0",
    },
    [CLASS_BUGBEAR] = {
        .name = "Bugbear",
        .color = {COLOR_BRASS},
        .glyph = 'b',

        .hit_die = "1d8",
        .mana_die = "0d0",

        .natural_armor_bonus = 3,

        .base_attack_bonus = 2,
    },
    [CLASS_DOG] = {
        .name = "Dog",
        .color = {COLOR_LIGHTEST_GRAY},
        .glyph = 'd',

        .hit_die = "1d8",
        .mana_die = "0d0",

        .natural_armor_bonus = 1,
    },
    [CLASS_GOBLIN] = {
        .name = "Goblin",
        .color = {COLOR_DARK_GREEN},
        .glyph = 'g',

        .hit_die = "1d5",
        .mana_die = "0d0",

        .base_attack_bonus = 1,
    },
    [CLASS_KOBOLD] = {
        .name = "Kobold",
        .color = {COLOR_LIGHT_GRAY},
        .glyph = 'k',

        .hit_die = "1d8",
        .mana_die = "0d0",

        .base_attack_bonus = 1,

        .natural_armor_bonus = 1,
    },
    [CLASS_RAT] = {
        .name = "Rat",
        .color = {COLOR_LIGHTEST_GRAY},
        .glyph = 'r',

        .hit_die = "1d2",
        .mana_die = "0d0",

        .natural_armor_bonus = 1,
    },
    [CLASS_RED_DRAGON_WYRMLING] = {
        .name = "Wyrmling Red Dragon",
        .color = {COLOR_LIGHT_RED},
        .glyph = 'D',

        .hit_die = "1d12",
        .mana_die = "0d0",

        .natural_armor_bonus = 6,

        .base_attack_bonus = 7,
    },
    [CLASS_RED_DRAGON_ADULT] = {
        .name = "Adult Red Dragon",
        .color = {COLOR_RED},
        .glyph = 'D',

        .hit_die = "1d12",
        .mana_die = "0d0",

        .natural_armor_bonus = 21,

        .base_attack_bonus = 22,
    },
    [CLASS_RED_DRAGON_ANCIENT] = {
        .name = "Ancient Red Dragon",
        .color = {COLOR_DARK_RED},
        .glyph = 'D',

        .hit_die = "1d12",
        .mana_die = "0d0",

        .natural_armor_bonus = 33,

        .base_attack_bonus = 34,
    },
    [CLASS_SKELETON_WARRIOR] = {
        .name = "Skeleton Warrior",
        .color = {COLOR_WHITE},
        .glyph = 's',

        .hit_die = "1d12",
        .mana_die = "0d0",
    },
};

const struct base_attack_bonus_progression_data base_attack_bonus_progression_database[] = {
    [BASE_ATTACK_BONUS_PROGRESSION_COMBAT] = {
        .name = "Combat",
        .multiplier = 1,
    },
    [BASE_ATTACK_BONUS_PROGRESSION_MIDDLE] = {
        .name = "Middle",
        .multiplier = 0.75f,
    },
    [BASE_ATTACK_BONUS_PROGRESSION_NON_COMBAT] = {
        .name = "Non-Combat",
        .multiplier = 0.5f,
    },
};

const struct ability_data ability_database[] = {
    [ABILITY_STRENGTH] = {
        .name = "Strength",
        .description = "Strength measures the muscle and physical power of a character.",
    },
    [ABILITY_DEXTERITY] = {
        .name = "Dexterity",
        .description = "Dexterity measures agility, reflexes, and balance.",
    },
    [ABILITY_CONSTITUTION] = {
        .name = "Constitution",
        .description = "Constitution represents the health and stamina of a character.",
    },
    [ABILITY_INTELLIGENCE] = {
        .name = "Intelligence",
        .description = "Intelligence determines how well a character learns and reasons.",
    },
};

const struct feat_data feat_database[] = {
    [FEAT_ARMOR_PROFICIENCY_LIGHT] = {
        .name = "Armor Proficiency: Light",
        .description = "A character with this feat can equip light armor.",
    },
    [FEAT_ARMOR_PROFICIENCY_MEDIUM] = {
        .name = "Armor Proficiency: Medium",
        .description = "A character with this feat can equip medium armor.",
    },
    [FEAT_ARMOR_PROFICIENCY_HEAVY] = {
        .name = "Armor Proficiency: Heavy",
        .description = "A character with this feat can equip heavy armor.",
    },
    [FEAT_LOW_LIGHT_VISION] = {
        .name = "Low-light vision",
        .description = "Grants the ability to see in the dark, but not as far as darkvision.",
    },
    [FEAT_QUICK_TO_MASTER] = {
        .name = "Quick to Master",
        .description = "A character with this feat gains an additional feat at level 1.",

        .prerequisites = {
            .requires_race = true,
            .race = RACE_HUMAN,
        },
    },
    [FEAT_RAPID_RELOAD] = {
        .name = "Rapid Reload",
        .description = "A character gets the same number of attacks with any crossbow as a normal bow.",

        .prerequisites = {
            .base_attack_bonus = 2,
        },
    },
    [FEAT_SHIELD_PROFICIENCY] = {
        .name = "Shield Proficiency",
        .description = "A character with this feat can equip shields.",
    },
    [FEAT_SNEAK_ATTACK] = {
        .name = "Sneak Attack",
        .description = "A character with this feat deals extra damage when undetected.",

        .prerequisites = {
            .requires_class = true,
            .class = CLASS_ROGUE,
        },
    },
    [FEAT_STILL_SPELL] = {
        .name = "Still Spell",
        .description = "A character with this feat ignores arcane spell failure.",
    },
    [FEAT_WEAPON_FINESSE] = {
        .name = "Weapon Finesse",
        .description = "A character with this feat is adept at using light weapons subtly and effectively, allowing him to make melee attack rolls with his dexterity modifier instead of strength (if his dexterity is higher than his strength).",

        .prerequisites = {
            .base_attack_bonus = 1,
        },
    },
    [FEAT_WEAPON_PROFICIENCY_ELF] = {
        .name = "Weapon Proficiency: Elf",
        .description = "A character with this feat is proficient with elf weapons.",

        .prerequisites = {
            .requires_race = true,
            .race = RACE_ELF,
        },
    },
    [FEAT_WEAPON_PROFICIENCY_EXOTIC] = {
        .name = "Weapon Proficiency: Exotic",
        .description = "A character with this feat is proficient with exotic weapons.",
    },
    [FEAT_WEAPON_PROFICIENCY_MARTIAL] = {
        .name = "Weapon Proficiency: Martial",
        .description = "A character with this feat is proficient with martial weapons.",
    },
    [FEAT_WEAPON_PROFICIENCY_ROGUE] = {
        .name = "Weapon Proficiency: Rogue",
        .description = "A character with this feat is proficient with rogue weapons.",

        .prerequisites = {
            .requires_class = true,
            .class = CLASS_ROGUE,
        },
    },
    [FEAT_WEAPON_PROFICIENCY_SIMPLE] = {
        .name = "Weapon Proficiency: Simple",
        .description = "A character with this feat is proficient with simple weapons.",
    },
    [FEAT_WEAPON_PROFICIENCY_WIZARD] = {
        .name = "Weapon Proficiency: Wizard",
        .description = "A character with this feat is proficient with wizard weapons.",

        .prerequisites = {
            .requires_class = true,
            .class = CLASS_WIZARD,
        },
    },
};

const struct actor_prototype monster_prototypes[] = {
    [MONSTER_BAT] = {
        .name = "Bat",
        .race = RACE_ANIMAL_DIMINUTIVE,
        .class = CLASS_BAT,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 1,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 2,
        },
    },
    [MONSTER_BUGBEAR] = {
        .name = "Bugbear",
        .race = RACE_HUMANOID_MEDIUM,
        .class = CLASS_BUGBEAR,

        .level = 3,

        .ability_scores = {
            [ABILITY_STRENGTH] = 15,
            [ABILITY_DEXTERITY] = 12,
            [ABILITY_CONSTITUTION] = 13,
            [ABILITY_INTELLIGENCE] = 10,
        },

        .equipment = {
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_LEATHER_ARMOR,
            [EQUIP_SLOT_SHIELD] = ITEM_TYPE_SMALL_SHIELD,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_LONGBOW,
            [EQUIP_SLOT_AMMUNITION] = ITEM_TYPE_ARROW,
        },

        .items = {
            [ITEM_TYPE_MACE] = 1,
        },
    },
    [MONSTER_GOBLIN] = {
        .name = "Goblin",
        .race = RACE_HUMANOID_SMALL,
        .class = CLASS_GOBLIN,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 11,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 12,
            [ABILITY_INTELLIGENCE] = 10,
        },

        .equipment = {
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_LEATHER_ARMOR,
            [EQUIP_SLOT_SHIELD] = ITEM_TYPE_SMALL_SHIELD,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_MACE,
        },
    },
    [MONSTER_KOBOLD] = {
        .name = "Kobold",
        .race = RACE_HUMANOID_SMALL,
        .class = CLASS_KOBOLD,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 9,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 10,
        },

        .equipment = {
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_LEATHER_ARMOR,
            [EQUIP_SLOT_SHIELD] = ITEM_TYPE_SPEAR,
        },
    },
    [MONSTER_RAT] = {
        .name = "Rat",
        .race = RACE_ANIMAL_TINY,
        .class = CLASS_RAT,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 2,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 2,
        },

        .equipment = {
            [EQUIP_SLOT_ARMOR] = ITEM_TYPE_LEATHER_ARMOR,
            [EQUIP_SLOT_SHIELD] = ITEM_TYPE_SMALL_SHIELD,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_MACE,
        },
    },
    [MONSTER_RED_DRAGON_WYRMLING] = {
        .name = "Wyrmling Red Dragon",
        .race = RACE_DRAGON_WYRMLING,
        .class = CLASS_RED_DRAGON_WYRMLING,

        .level = 7,

        .ability_scores = {
            [ABILITY_STRENGTH] = 17,
            [ABILITY_DEXTERITY] = 10,
            [ABILITY_CONSTITUTION] = 15,
            [ABILITY_INTELLIGENCE] = 10,
        },
    },
    [MONSTER_RED_DRAGON_ADULT] = {
        .name = "Adult Red Dragon",
        .race = RACE_DRAGON_ADULT,
        .class = CLASS_RED_DRAGON_ADULT,

        .level = 22,

        .ability_scores = {
            [ABILITY_STRENGTH] = 33,
            [ABILITY_DEXTERITY] = 10,
            [ABILITY_CONSTITUTION] = 21,
            [ABILITY_INTELLIGENCE] = 16,
        },
    },
    [MONSTER_RED_DRAGON_ANCIENT] = {
        .name = "Ancient Red Dragon",
        .race = RACE_DRAGON_ANCIENT,
        .class = CLASS_RED_DRAGON_ANCIENT,

        .level = 34,

        .ability_scores = {
            [ABILITY_STRENGTH] = 39,
            [ABILITY_DEXTERITY] = 10,
            [ABILITY_CONSTITUTION] = 29,
            [ABILITY_INTELLIGENCE] = 24,
        },
    },
    [MONSTER_SKELETON_WARRIOR] = {
        .name = "Skeleton Warrior",
        .race = RACE_UNDEAD,
        .class = CLASS_SKELETON_WARRIOR,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 13,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 0,
            [ABILITY_INTELLIGENCE] = 0,
        },

        .equipment = {
            [EQUIP_SLOT_SHIELD] = ITEM_TYPE_LARGE_SHIELD,
            [EQUIP_SLOT_WEAPON] = ITEM_TYPE_LONGSWORD,
        },
    },
};

const struct corpse_metadata corpse_metadata = {
    .glyph = '%',
    .color = {COLOR_DARK_RED},
};

const struct equip_slot_data equip_slot_database[] = {
    [EQUIP_SLOT_AMMUNITION] = {
        .name = "Ammunition",
    },
    [EQUIP_SLOT_ARMOR] = {
        .name = "Armor",
    },
    [EQUIP_SLOT_SHIELD] = {
        .name = "Shield",
    },
    [EQUIP_SLOT_WEAPON] = {
        .name = "Weapon",
    },
};

const struct base_item_data base_item_database[] = {
    [BASE_ITEM_TYPE_ARROW] = {
        .name = "Arrow",
        .glyph = '`',

        .equip_slot = EQUIP_SLOT_AMMUNITION,
        .size = SIZE_SMALL,
        .weight = 0.1f,
        .max_stack = 100,

        .ammunition_type = AMMUNITION_TYPE_ARROW,
    },
    [BASE_ITEM_TYPE_BREASTPLATE] = {
        .name = "Breatplate",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_LARGE,
        .weight = 40,
        .max_stack = 1,

        .armor_class = 5,
        .arcane_spell_failure = 0.3f,
        .max_dexterity_bonus = 2,
        .armor_proficiency = ARMOR_PROFICIENCY_MEDIUM,
    },
    [BASE_ITEM_TYPE_BOLT] = {
        .name = "Bolt",
        .glyph = '`',

        .equip_slot = EQUIP_SLOT_AMMUNITION,
        .size = SIZE_SMALL,
        .weight = 0.1f,
        .max_stack = 100,

        .ammunition_type = AMMUNITION_TYPE_BOLT,
    },
    [BASE_ITEM_TYPE_BULLET] = {
        .name = "Bullet",
        .glyph = '`',

        .equip_slot = EQUIP_SLOT_AMMUNITION,
        .size = SIZE_TINY,
        .weight = 0.1f,
        .max_stack = 100,

        .ammunition_type = AMMUNITION_TYPE_BULLET,
    },
    [BASE_ITEM_TYPE_CLOTHING] = {
        .name = "Armor",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_MEDIUM,
        .weight = 1,
        .max_stack = 1,
    },
    [BASE_ITEM_TYPE_DAGGER] = {
        .name = "Dagger",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_TINY,
        .weight = 1,
        .max_stack = 1,

        .damage = "1d4",
        .threat_range = 19,
        .critical_multiplier = 2,
        .finesse = true,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_SIMPLE] = true,
            [WEAPON_PROFICIENCY_WIZARD] = true,
        },
    },
    [BASE_ITEM_TYPE_FULL_PLATE] = {
        .name = "Full Plate",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_LARGE,
        .weight = 50,
        .max_stack = 1,

        .armor_class = 8,
        .arcane_spell_failure = 0.45f,
        .max_dexterity_bonus = 1,
        .armor_proficiency = ARMOR_PROFICIENCY_HEAVY,
    },
    [BASE_ITEM_TYPE_GOLD] = {
        .name = "Gold",
        .glyph = '$',

        .size = SIZE_TINY,
        .weight = 0.1f,
        .max_stack = INT32_MAX,
    },
    [BASE_ITEM_TYPE_GREATSWORD] = {
        .name = "Greatsword",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,
        .weight = 15,
        .max_stack = 1,

        .damage = "2d6",
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

        .armor_class = 7,
        .arcane_spell_failure = 0.4f,
        .max_dexterity_bonus = 1,
        .armor_proficiency = ARMOR_PROFICIENCY_HEAVY,
    },
    [BASE_ITEM_TYPE_HEAVY_CROSSBOW] = {
        .name = "Heavy Crossbow",
        .glyph = 'T',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_MEDIUM,
        .weight = 9,
        .max_stack = 1,

        .damage = "1d10",
        .threat_range = 19,
        .critical_multiplier = 2,
        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_BOLT,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_SIMPLE] = true,
            [WEAPON_PROFICIENCY_WIZARD] = true,
        },
    },
    [BASE_ITEM_TYPE_LARGE_SHIELD] = {
        .name = "Large Shield",
        .glyph = ')',

        .equip_slot = EQUIP_SLOT_SHIELD,
        .size = SIZE_MEDIUM,
        .weight = 15,
        .max_stack = 1,

        .armor_class = 2,
        .arcane_spell_failure = 0.15f,
        .armor_proficiency = ARMOR_PROFICIENCY_SHIELD,
    },
    [BASE_ITEM_TYPE_LEATHER_ARMOR] = {
        .name = "Leather Armor",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_MEDIUM,
        .weight = 10,
        .max_stack = 1,

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
        .weight = 6,
        .max_stack = 1,

        .damage = "1d8",
        .threat_range = 19,
        .critical_multiplier = 2,
        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_BOLT,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_SIMPLE] = true,
            [WEAPON_PROFICIENCY_WIZARD] = true,
        },
    },
    [BASE_ITEM_TYPE_LONGBOW] = {
        .name = "Longbow",
        .glyph = '}',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,
        .weight = 3,
        .max_stack = 1,

        .damage = "1d8",
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

        .damage = "1d8",
        .threat_range = 19,
        .critical_multiplier = 2,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_ELF] = true,
            [WEAPON_PROFICIENCY_MARTIAL] = true,
        },
    },
    [BASE_ITEM_TYPE_MACE] = {
        .name = "Mace",
        .glyph = '!',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_SMALL,
        .weight = 6,
        .max_stack = 1,

        .damage = "1d6",
        .threat_range = 20,
        .critical_multiplier = 2,
        .finesse = true,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_ROGUE] = true,
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
    [BASE_ITEM_TYPE_SCROLL] = {
        .name = "Scroll",
        .glyph = '%',

        .size = SIZE_SMALL,
        .weight = 0.5f,
        .max_stack = 10,
    },
    [BASE_ITEM_TYPE_SLING] = {
        .name = "Sling",
        .glyph = '?',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_SMALL,
        .weight = 1,
        .max_stack = 1,

        .damage = "1d4",
        .threat_range = 20,
        .critical_multiplier = 2,
        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_BULLET,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_ROGUE] = true,
            [WEAPON_PROFICIENCY_SIMPLE] = true,
        },
    },
    [BASE_ITEM_TYPE_SMALL_SHIELD] = {
        .name = "Small Shield",
        .glyph = ')',

        .equip_slot = EQUIP_SLOT_SHIELD,
        .size = SIZE_SMALL,
        .weight = 6,
        .max_stack = 1,

        .armor_class = 1,
        .arcane_spell_failure = 0.05f,
        .armor_proficiency = ARMOR_PROFICIENCY_SHIELD,
    },
    [BASE_ITEM_TYPE_SPEAR] = {
        .name = "Spear",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,
        .weight = 3,
        .max_stack = 1,

        .damage = "1d8",
        .threat_range = 20,
        .critical_multiplier = 3,
        .weapon_proficiencies = {
            [WEAPON_PROFICIENCY_SIMPLE] = true,
        },
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

        .armor_class = 3,
        .arcane_spell_failure = 0.5f,
        .armor_proficiency = ARMOR_PROFICIENCY_SHIELD,
    },
};

const struct item_data item_database[] = {
    [ITEM_TYPE_ADAMANTINE_BREASTPLATE] = {
        .type = BASE_ITEM_TYPE_BREASTPLATE,

        .name = "Adamantine Breastplate",
        .description = "",
        .color = {COLOR_DARK_AZURE},

        .level = 1,

        .unique = true,
    },
    [ITEM_TYPE_ARROW] = {
        .type = BASE_ITEM_TYPE_ARROW,

        .name = "Arrow",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_BREASTPLATE] = {
        .type = BASE_ITEM_TYPE_BREASTPLATE,

        .name = "Breastplate",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_BOLT] = {
        .type = BASE_ITEM_TYPE_BOLT,

        .name = "Bolt",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_BULLET] = {
        .type = BASE_ITEM_TYPE_BULLET,

        .name = "Bullet",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_DAGGER] = {
        .type = BASE_ITEM_TYPE_DAGGER,

        .name = "Dagger",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_FULL_PLATE] = {
        .type = BASE_ITEM_TYPE_FULL_PLATE,

        .name = "Full Plate",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_GOLD] = {
        .type = BASE_ITEM_TYPE_GOLD,

        .name = "Gold",
        .description = "",
        .color = {COLOR_GOLD},

        .level = 1,
    },
    [ITEM_TYPE_GREATSWORD] = {
        .type = BASE_ITEM_TYPE_GREATSWORD,

        .name = "Greatsword",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_HALF_PLATE] = {
        .type = BASE_ITEM_TYPE_HALF_PLATE,

        .name = "Half Plate",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_HEAVY_CROSSBOW] = {
        .type = BASE_ITEM_TYPE_HEAVY_CROSSBOW,

        .name = "Heavy Crossbow",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_LARGE_SHIELD] = {
        .type = BASE_ITEM_TYPE_LARGE_SHIELD,

        .name = "Large Shield",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_LEATHER_ARMOR] = {
        .type = BASE_ITEM_TYPE_LEATHER_ARMOR,

        .name = "Leather Armor",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_LIGHT_CROSSBOW] = {
        .type = BASE_ITEM_TYPE_LIGHT_CROSSBOW,

        .name = "Light Crossbow",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_LONGBOW] = {
        .type = BASE_ITEM_TYPE_LONGBOW,

        .name = "Longbow",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_LONGSWORD] = {
        .type = BASE_ITEM_TYPE_LONGSWORD,

        .name = "Longsword",
        .description = "",
        .color = {COLOR_WHITE},
        .level = 1,
    },
    [ITEM_TYPE_MACE] = {
        .type = BASE_ITEM_TYPE_MACE,

        .name = "Mace",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_POTION_MINOR_HEAL] = {
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Minor Healing",
        .description = "",
        .color = {COLOR_RED},

        .spell_type = SPELL_TYPE_MINOR_HEAL,

        .level = 1,
    },
    [ITEM_TYPE_POTION_MINOR_MANA] = {
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Minor Mana",
        .description = "",
        .color = {COLOR_BLUE},

        .spell_type = SPELL_TYPE_MINOR_MANA,

        .level = 1,
    },
    [ITEM_TYPE_SCROLL_LIGHTNING] = {
        .type = BASE_ITEM_TYPE_SCROLL,

        .name = "Scroll of Lightning",
        .description = "",
        .color = {COLOR_WHITE},

        .spell_type = SPELL_TYPE_LIGHTNING,

        .level = 1,
    },
    [ITEM_TYPE_SLING] = {
        .type = BASE_ITEM_TYPE_SLING,

        .name = "Sling",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_SMALL_SHIELD] = {
        .type = BASE_ITEM_TYPE_SMALL_SHIELD,

        .name = "Small Shield",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_SPEAR] = {
        .type = BASE_ITEM_TYPE_SPEAR,

        .name = "Spear",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_TOME_FIREBALL] = {
        .type = BASE_ITEM_TYPE_TOME,

        .name = "Tome of Fireball",
        .description = "",
        .color = {COLOR_WHITE},

        .spell_type = SPELL_TYPE_FIREBALL,

        .level = 1,
    },
    [ITEM_TYPE_TOWER_SHIELD] = {
        .type = BASE_ITEM_TYPE_TOWER_SHIELD,

        .name = "Tower Shield",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_WIZARDS_ROBE] = {
        .type = BASE_ITEM_TYPE_CLOTHING,

        .name = "Wizard's Robe",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
};

const struct spell_data spell_database[] = {
    [SPELL_TYPE_ACID_SPLASH] = {
        .name = "Acid Splash",
        .range = SPELL_RANGE_TOUCH,
        .mana_cost = 0,
    },
    [SPELL_TYPE_MINOR_HEAL] = {
        .name = "Minor Heal",
        .range = SPELL_RANGE_TOUCH,
        .mana_cost = 1,
    },
    [SPELL_TYPE_MINOR_MANA] = {
        .name = "Minor Mana",
        .range = SPELL_RANGE_TOUCH,
        .mana_cost = 0,
    },
    [SPELL_TYPE_LIGHTNING] = {
        .name = "Lightning",
        .range = SPELL_RANGE_TOUCH,
        .mana_cost = 2,
    },
    [SPELL_TYPE_FIREBALL] = {
        .name = "Fireball",
        .range = SPELL_RANGE_TOUCH,
        .mana_cost = 0,
    },
};

const struct projectile_data projectile_database[] = {
    [PROJECTILE_TYPE_ACID_SPLASH] = {
        .glyph = '*',
        .color = {COLOR_LIME},

        .light_type = LIGHT_TYPE_ACID_SPLASH,

        .speed = 30,
    },
    [PROJECTILE_TYPE_ARROW] = {
        .glyph = '`',
        .color = {COLOR_WHITE},

        .light_type = LIGHT_TYPE_NONE,

        .speed = 50,
    },
    [PROJECTILE_TYPE_FIREBALL] = {
        .glyph = '*',
        .color = {COLOR_FLAME},

        .light_type = LIGHT_TYPE_FIREBALL,

        .speed = 30,
    },
};

const struct surface_data surface_database[] = {
    [SURFACE_TYPE_ACID] = {
        .glyph = '\0',
        .color = {COLOR_LIME},

        .duration = 20,

        .light_type = LIGHT_TYPE_ACID,
    },
    [SURFACE_TYPE_FIRE] = {
        .glyph = '\0',
        .color = {COLOR_FLAME},

        .duration = 20,

        .light_type = LIGHT_TYPE_FIRE,
    },
    [SURFACE_TYPE_WATER] = {
        .glyph = '~',
        .color = {COLOR_AZURE},

        .duration = 0,

        .light_type = LIGHT_TYPE_NONE,
    },
};
