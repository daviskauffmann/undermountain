#include "monster.h"

#include "class.h"
#include "faction.h"
#include "race.h"
#include "size.h"

const struct monster_data monster_database[] = {
    [MONSTER_TYPE_BAT] = {
        .name = "Bat",

        .race = RACE_ANIMAL,
        .class = CLASS_BAT,
        .size = SIZE_DIMINUTIVE,
        .faction = FACTION_WILD_ANIMAL,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 1,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 2,
            [ABILITY_WISDOM] = 14,
            [ABILITY_CHARISMA] = 4,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },
    },
    [MONSTER_TYPE_BUGBEAR] = {
        .name = "Bugbear",

        .race = RACE_HUMANOID,
        .class = CLASS_BUGBEAR,
        .size = SIZE_MEDIUM,
        .faction = FACTION_GOBLINOID,

        .level = 3,

        .ability_scores = {
            [ABILITY_STRENGTH] = 15,
            [ABILITY_DEXTERITY] = 12,
            [ABILITY_CONSTITUTION] = 13,
            [ABILITY_INTELLIGENCE] = 10,
            [ABILITY_WISDOM] = 10,
            [ABILITY_CHARISMA] = 9,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },

        .feats = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = true,
            [FEAT_SHIELD_PROFICIENCY] = true,
            [FEAT_WEAPON_FOCUS] = true,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = true,
        },

        .equipment = {
            [EQUIP_SLOT_ARMOR] = {
                .type = ITEM_TYPE_LEATHER_ARMOR,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_SHIELD] = {
                .type = ITEM_TYPE_LIGHT_SHIELD,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_WEAPON] = {
                .type = ITEM_TYPE_MORNINGSTAR,

                .min_stack = 1,
                .max_stack = 1,
            },
        },

        .items = {
            [ITEM_TYPE_JAVELIN] = 1,
        },
    },
    [MONSTER_TYPE_CAT] = {
        .name = "Cat",

        .race = RACE_ANIMAL,
        .class = CLASS_CAT,
        .size = SIZE_TINY,
        .faction = FACTION_WILD_ANIMAL,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 3,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 2,
            [ABILITY_WISDOM] = 12,
            [ABILITY_CHARISMA] = 7,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .feats = {
            [FEAT_WEAPON_FINESSE] = true,
        },
    },
    [MONSTER_TYPE_DIRE_RAT] = {
        .name = "Dire Rat",

        .race = RACE_ANIMAL,
        .class = CLASS_DIRE_RAT,
        .size = SIZE_SMALL,
        .faction = FACTION_WILD_ANIMAL,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 10,
            [ABILITY_DEXTERITY] = 17,
            [ABILITY_CONSTITUTION] = 12,
            [ABILITY_INTELLIGENCE] = 1,
            [ABILITY_WISDOM] = 12,
            [ABILITY_CHARISMA] = 4,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .special_attacks = {
            [SPECIAL_ATTACK_DISEASE] = true,
        },

        .feats = {
            [FEAT_WEAPON_FINESSE] = true,
        },
    },
    [MONSTER_TYPE_DOG] = {
        .name = "Dog",

        .race = RACE_ANIMAL,
        .class = CLASS_DOG,
        .size = SIZE_SMALL,
        .faction = FACTION_WILD_ANIMAL,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 13,
            [ABILITY_DEXTERITY] = 17,
            [ABILITY_CONSTITUTION] = 15,
            [ABILITY_INTELLIGENCE] = 2,
            [ABILITY_WISDOM] = 12,
            [ABILITY_CHARISMA] = 6,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },
    },
    [MONSTER_TYPE_FIRE_BEETLE] = {
        .name = "Fire Beetle",

        .race = RACE_VERMIN,
        .class = CLASS_FIRE_BEETLE,
        .size = SIZE_SMALL,
        .faction = FACTION_VERMIN,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 10,
            [ABILITY_DEXTERITY] = 11,
            [ABILITY_CONSTITUTION] = 11,
            [ABILITY_INTELLIGENCE] = 0,
            [ABILITY_WISDOM] = 10,
            [ABILITY_CHARISMA] = 7,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },
    },
    [MONSTER_TYPE_GIANT_ANT] = {
        .name = "Giant Ant",

        .race = RACE_VERMIN,
        .class = CLASS_GIANT_ANT,
        .size = SIZE_MEDIUM,
        .faction = FACTION_VERMIN,

        .level = 2,

        .ability_scores = {
            [ABILITY_STRENGTH] = 10,
            [ABILITY_DEXTERITY] = 10,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 0,
            [ABILITY_WISDOM] = 11,
            [ABILITY_CHARISMA] = 9,
        },
    },
    [MONSTER_TYPE_GNOLL] = {
        .name = "Gnoll",

        .race = RACE_HUMANOID,
        .class = CLASS_GNOLL,
        .size = SIZE_MEDIUM,
        .faction = FACTION_GNOLL,

        .level = 2,

        .ability_scores = {
            [ABILITY_STRENGTH] = 15,
            [ABILITY_DEXTERITY] = 10,
            [ABILITY_CONSTITUTION] = 13,
            [ABILITY_INTELLIGENCE] = 8,
            [ABILITY_WISDOM] = 11,
            [ABILITY_CHARISMA] = 8,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },

        .feats = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = true,
            [FEAT_POWER_ATTACK] = true,
            [FEAT_SHIELD_PROFICIENCY] = true,
            [FEAT_WEAPON_PROFICIENCY_MARTIAL] = true,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = true,
        },

        .equipment = {
            [EQUIP_SLOT_ARMOR] = {
                .type = ITEM_TYPE_LEATHER_ARMOR,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_SHIELD] = {
                .type = ITEM_TYPE_HEAVY_SHIELD,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_WEAPON] = {
                .type = ITEM_TYPE_BATTLEAXE,

                .min_stack = 1,
                .max_stack = 1,
            },
        },

        .items = {
            [ITEM_TYPE_SHORTBOW] = {
                .min_stack = 0,
                .max_stack = 1,
            },
            [ITEM_TYPE_ARROW] = {
                .min_stack = 0,
                .max_stack = 10,
            },
        },
    },
    [MONSTER_TYPE_GOBLIN] = {
        .name = "Goblin",

        .race = RACE_HUMANOID,
        .class = CLASS_GOBLIN,
        .size = SIZE_SMALL,
        .faction = FACTION_GOBLINOID,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 11,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 12,
            [ABILITY_INTELLIGENCE] = 10,
            [ABILITY_WISDOM] = 9,
            [ABILITY_CHARISMA] = 6,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },

        .feats = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = true,
            [FEAT_SHIELD_PROFICIENCY] = true,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = true,
        },

        .equipment = {
            [EQUIP_SLOT_ARMOR] = {
                .type = ITEM_TYPE_LEATHER_ARMOR,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_SHIELD] = {
                .type = ITEM_TYPE_LIGHT_SHIELD,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_WEAPON] = {
                .type = ITEM_TYPE_MORNINGSTAR,

                .min_stack = 1,
                .max_stack = 1,
            },
        },

        .items = {
            [ITEM_TYPE_JAVELIN] = {
                .min_stack = 0,
                .max_stack = 3,
            },
        },
    },
    [MONSTER_TYPE_HOBGOBLIN] = {
        .name = "Hobgoblin",

        .race = RACE_HUMANOID,
        .class = CLASS_HOBGOBLIN,
        .size = SIZE_MEDIUM,
        .faction = FACTION_GOBLINOID,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 13,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 14,
            [ABILITY_INTELLIGENCE] = 10,
            [ABILITY_WISDOM] = 9,
            [ABILITY_CHARISMA] = 8,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },

        .feats = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = true,
            [FEAT_SHIELD_PROFICIENCY] = true,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = true,
            [FEAT_WEAPON_PROFICIENCY_MARTIAL] = true,
        },

        .equipment = {
            [EQUIP_SLOT_ARMOR] = {
                .type = ITEM_TYPE_STUDDED_LEATHER_ARMOR,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_SHIELD] = {
                .type = ITEM_TYPE_LIGHT_SHIELD,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_WEAPON] = {
                .type = ITEM_TYPE_LONGSWORD,

                .min_stack = 1,
                .max_stack = 1,
            },
        },

        .items = {
            [ITEM_TYPE_JAVELIN] = {
                .min_stack = 0,
                .max_stack = 3,
            },
        },
    },
    [MONSTER_TYPE_HYENA] = {
        .name = "Hyena",

        .race = RACE_ANIMAL,
        .class = CLASS_HYENA,
        .size = SIZE_MEDIUM,
        .faction = FACTION_GNOLL,

        .level = 2,

        .ability_scores = {
            [ABILITY_STRENGTH] = 14,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 15,
            [ABILITY_INTELLIGENCE] = 2,
            [ABILITY_WISDOM] = 13,
            [ABILITY_CHARISMA] = 6,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },
    },
    [MONSTER_TYPE_KOBOLD] = {
        .name = "Kobold",

        .race = RACE_HUMANOID,
        .class = CLASS_KOBOLD,
        .size = SIZE_SMALL,
        .faction = FACTION_KOBOLD,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 9,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 10,
            [ABILITY_WISDOM] = 9,
            [ABILITY_CHARISMA] = 8,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },

        .feats = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = true,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = true,
        },

        .equipment = {
            [EQUIP_SLOT_ARMOR] = {
                .type = ITEM_TYPE_LEATHER_ARMOR,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_WEAPON] = {
                .type = ITEM_TYPE_SHORTSPEAR,

                .min_stack = 1,
                .max_stack = 1,
            },
        },

        .items = {
            [ITEM_TYPE_SLING] = {
                .min_stack = 0,
                .max_stack = 1,
            },
            [ITEM_TYPE_BULLET] = {
                .min_stack = 0,
                .max_stack = 5,
            },
        },
    },
    [MONSTER_TYPE_OGRE] = {
        .name = "Ogre",

        .race = RACE_GIANT,
        .class = CLASS_OGRE,
        .size = SIZE_LARGE,
        .faction = FACTION_OGRE,

        .level = 4,

        .ability_scores = {
            [ABILITY_STRENGTH] = 21,
            [ABILITY_DEXTERITY] = 8,
            [ABILITY_CONSTITUTION] = 15,
            [ABILITY_INTELLIGENCE] = 6,
            [ABILITY_WISDOM] = 10,
            [ABILITY_CHARISMA] = 7,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .feats = {
            [FEAT_TOUGHNESS] = true,
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = true,
            [FEAT_ARMOR_PROFICIENCY_MEDIUM] = true,
            [FEAT_WEAPON_FOCUS] = true,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = true,
            [FEAT_WEAPON_PROFICIENCY_MARTIAL] = true,
        },

        .equipment = {
            [EQUIP_SLOT_ARMOR] = {
                .type = ITEM_TYPE_HIDE_ARMOR,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_WEAPON] = {
                .type = ITEM_TYPE_GREATCLUB,

                .min_stack = 1,
                .max_stack = 1,
            },
        },

        .items = {
            [ITEM_TYPE_JAVELIN] = {
                .min_stack = 0,
                .max_stack = 3,
            },
        },
    },
    [MONSTER_TYPE_ORC] = {
        .name = "Orc",

        .race = RACE_HUMANOID,
        .class = CLASS_ORC,
        .size = SIZE_MEDIUM,
        .faction = FACTION_ORC,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 17,
            [ABILITY_DEXTERITY] = 11,
            [ABILITY_CONSTITUTION] = 12,
            [ABILITY_INTELLIGENCE] = 8,
            [ABILITY_WISDOM] = 7,
            [ABILITY_CHARISMA] = 6,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },

        .feats = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = true,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = true,
            [FEAT_WEAPON_PROFICIENCY_MARTIAL] = true,
        },

        .equipment = {
            [EQUIP_SLOT_ARMOR] = {
                .type = ITEM_TYPE_STUDDED_LEATHER_ARMOR,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_WEAPON] = {
                .type = ITEM_TYPE_FALCHION,

                .min_stack = 1,
                .max_stack = 1,
            },
        },

        .items = {
            [ITEM_TYPE_JAVELIN] = {
                .min_stack = 0,
                .max_stack = 3,
            },
        },
    },
    [MONSTER_TYPE_OWL] = {
        .name = "Owl",

        .race = RACE_ANIMAL,
        .class = CLASS_OWL,
        .size = SIZE_TINY,
        .faction = FACTION_WILD_ANIMAL,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 4,
            [ABILITY_DEXTERITY] = 17,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 2,
            [ABILITY_WISDOM] = 14,
            [ABILITY_CHARISMA] = 4,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .feats = {
            [FEAT_WEAPON_FINESSE] = true,
        },
    },
    [MONSTER_TYPE_PONY] = {
        .name = "Pony",

        .race = RACE_ANIMAL,
        .class = CLASS_PONY,
        .size = SIZE_MEDIUM,
        .faction = FACTION_WILD_ANIMAL,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 4,
            [ABILITY_DEXTERITY] = 17,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 2,
            [ABILITY_WISDOM] = 14,
            [ABILITY_CHARISMA] = 4,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .feats = {
            [FEAT_WEAPON_FINESSE] = true,
        },
    },
    [MONSTER_TYPE_KRENSHAR] = {
        .name = "Krenshar",

        .race = RACE_MAGICAL_BEAST,
        .class = CLASS_KRENSHAR,
        .size = SIZE_MEDIUM,
        .faction = FACTION_MAGICAL_BEAST,

        .level = 2,

        .ability_scores = {
            [ABILITY_STRENGTH] = 11,
            [ABILITY_DEXTERITY] = 14,
            [ABILITY_CONSTITUTION] = 11,
            [ABILITY_INTELLIGENCE] = 6,
            [ABILITY_WISDOM] = 12,
            [ABILITY_CHARISMA] = 13,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .special_attacks = {
            [SPECIAL_ATTACK_SCARE] = true,
        },

        .feats = {
            [FEAT_MULTIATTACK] = true,
        },
    },
    [MONSTER_TYPE_RAKSHASA] = {
        .name = "Rakshasa",

        .race = RACE_OUTSIDER,
        .class = CLASS_RAKSHASA,
        .size = SIZE_MEDIUM,
        .faction = FACTION_OUTSIDER,

        .level = 7,

        .ability_scores = {
            [ABILITY_STRENGTH] = 12,
            [ABILITY_DEXTERITY] = 14,
            [ABILITY_CONSTITUTION] = 16,
            [ABILITY_INTELLIGENCE] = 13,
            [ABILITY_WISDOM] = 13,
            [ABILITY_CHARISMA] = 17,
        },

        .special_abilities = {
            // TODO: shapeshift
            // TODO: damage reduction
            // TODO: spell resistance
            [SPECIAL_ABILITY_DARKVISION] = true,
        },

        // TODO: spells

        .feats = {
            [FEAT_DODGE] = true,
        },
    },
    [MONSTER_TYPE_RAT] = {
        .name = "Rat",

        .race = RACE_ANIMAL,
        .class = CLASS_RAT,
        .size = SIZE_TINY,
        .faction = FACTION_WILD_ANIMAL,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 2,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 2,
            [ABILITY_WISDOM] = 12,
            [ABILITY_CHARISMA] = 2,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .feats = {
            [FEAT_WEAPON_FINESSE] = true,
        },
    },
    [MONSTER_TYPE_RED_DRAGON_WYRMLING] = {
        .name = "Wyrmling Red Dragon",

        .race = RACE_RED_DRAGON,
        .class = CLASS_RED_DRAGON_WYRMLING,
        .size = SIZE_MEDIUM,
        .faction = FACTION_RED_DRAGON,

        .level = 7,

        .ability_scores = {
            [ABILITY_STRENGTH] = 17,
            [ABILITY_DEXTERITY] = 10,
            [ABILITY_CONSTITUTION] = 15,
            [ABILITY_INTELLIGENCE] = 10,
            [ABILITY_WISDOM] = 11,
            [ABILITY_CHARISMA] = 10,
        },
    },
    [MONSTER_TYPE_RED_DRAGON_ADULT] = {
        .name = "Adult Red Dragon",

        .race = RACE_RED_DRAGON,
        .class = CLASS_RED_DRAGON_ADULT,
        .size = SIZE_HUGE,
        .faction = FACTION_RED_DRAGON,

        .level = 22,

        .ability_scores = {
            [ABILITY_STRENGTH] = 33,
            [ABILITY_DEXTERITY] = 10,
            [ABILITY_CONSTITUTION] = 21,
            [ABILITY_INTELLIGENCE] = 16,
            [ABILITY_WISDOM] = 19,
            [ABILITY_CHARISMA] = 16,
        },
    },
    [MONSTER_TYPE_RED_DRAGON_ANCIENT] = {
        .name = "Ancient Red Dragon",

        .race = RACE_RED_DRAGON,
        .class = CLASS_RED_DRAGON_ANCIENT,
        .size = SIZE_GARGANTUAN,
        .faction = FACTION_RED_DRAGON,

        .level = 34,

        .ability_scores = {
            [ABILITY_STRENGTH] = 39,
            [ABILITY_DEXTERITY] = 10,
            [ABILITY_CONSTITUTION] = 29,
            [ABILITY_INTELLIGENCE] = 24,
            [ABILITY_WISDOM] = 25,
            [ABILITY_CHARISMA] = 24,
        },
    },
    [MONSTER_TYPE_SKELETON_WARRIOR] = {
        .name = "Skeleton Warrior",

        .race = RACE_UNDEAD,
        .class = CLASS_SKELETON_WARRIOR,
        .size = SIZE_MEDIUM,
        .faction = FACTION_UNDEAD,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 13,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 0,
            [ABILITY_INTELLIGENCE] = 0,
            [ABILITY_WISDOM] = 10,
            [ABILITY_CHARISMA] = 1,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },

        .feats = {
            [FEAT_IMPROVED_INITIATIVE] = true,
            [FEAT_SHIELD_PROFICIENCY] = true,
            [FEAT_WEAPON_PROFICIENCY_MARTIAL] = true,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = true,
        },

        .equipment = {
            [EQUIP_SLOT_SHIELD] = {
                .type = ITEM_TYPE_HEAVY_SHIELD,

                .min_stack = 1,
                .max_stack = 1,
            },
            [EQUIP_SLOT_WEAPON] = {
                .type = ITEM_TYPE_SCIMITAR,

                .min_stack = 1,
                .max_stack = 1,
            },
        },
    },
    [MONSTER_TYPE_SNAKE] = {
        .name = "Snake",

        .race = RACE_ANIMAL,
        .class = CLASS_SNAKE,
        .size = SIZE_SMALL,
        .faction = FACTION_WILD_ANIMAL,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 6,
            [ABILITY_DEXTERITY] = 17,
            [ABILITY_CONSTITUTION] = 11,
            [ABILITY_INTELLIGENCE] = 1,
            [ABILITY_WISDOM] = 12,
            [ABILITY_CHARISMA] = 2,
        },

        .special_attacks = {
            [SPECIAL_ATTACK_POISON] = true,
        },

        .feats = {
            [FEAT_IMPROVED_INITIATIVE] = true,
            [FEAT_WEAPON_FINESSE] = true,
        },
    },
    [MONSTER_TYPE_SPIDER] = {
        .name = "Spider",

        .race = RACE_VERMIN,
        .class = CLASS_SPIDER,
        .size = SIZE_SMALL,
        .faction = FACTION_VERMIN,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 7,
            [ABILITY_DEXTERITY] = 17,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 0,
            [ABILITY_WISDOM] = 10,
            [ABILITY_CHARISMA] = 2,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },

        .special_attacks = {
            [SPECIAL_ATTACK_POISON] = true,
            [SPECIAL_ATTACK_WEB] = true,
        },

        .feats = {
            [FEAT_WEAPON_FINESSE] = true,
        },
    },
    [MONSTER_TYPE_TROLL] = {
        .name = "Troll",

        .race = RACE_GIANT,
        .class = CLASS_TROLL,
        .size = SIZE_LARGE,
        .faction = FACTION_TROLL,

        .level = 6,

        .ability_scores = {
            [ABILITY_STRENGTH] = 23,
            [ABILITY_DEXTERITY] = 14,
            [ABILITY_CONSTITUTION] = 23,
            [ABILITY_INTELLIGENCE] = 6,
            [ABILITY_WISDOM] = 9,
            [ABILITY_CHARISMA] = 6,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
            [SPECIAL_ABILITY_REGENERATION] = true,
        },

        .special_attacks = {
            [SPECIAL_ATTACK_REND] = true,
        },

        .feats = {
            [FEAT_IRON_WILL] = true,
        },
    },
    [MONSTER_TYPE_WOLF] = {
        .name = "Wolf",

        .race = RACE_ANIMAL,
        .class = CLASS_WOLF,
        .size = SIZE_MEDIUM,
        .faction = FACTION_WILD_ANIMAL,

        .level = 2,

        .ability_scores = {
            [ABILITY_STRENGTH] = 13,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 15,
            [ABILITY_INTELLIGENCE] = 2,
            [ABILITY_WISDOM] = 12,
            [ABILITY_CHARISMA] = 6,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .special_attacks = {
            [SPECIAL_ATTACK_TRIP] = true,
        },

        .feats = {
            [FEAT_WEAPON_FOCUS] = true,
        },
    },
    [MONSTER_TYPE_ZOMBIE] = {
        .name = "Zombie",

        .race = RACE_UNDEAD,
        .class = CLASS_ZOMBIE,
        .size = SIZE_MEDIUM,
        .faction = FACTION_UNDEAD,

        .level = 2,

        .ability_scores = {
            [ABILITY_STRENGTH] = 13,
            [ABILITY_DEXTERITY] = 8,
            [ABILITY_CONSTITUTION] = 0,
            [ABILITY_INTELLIGENCE] = 0,
            [ABILITY_WISDOM] = 10,
            [ABILITY_CHARISMA] = 1,
        },

        .feats = {
            [FEAT_TOUGHNESS] = true,
        },

        .equipment = {
            [EQUIP_SLOT_WEAPON] = {
                .type = ITEM_TYPE_CLUB,

                .min_stack = 0,
                .max_stack = 1,
            },
        },
    },
};
