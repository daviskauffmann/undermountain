#include "monster_prototype.h"

#include "faction.h"

const struct monster_prototype monster_prototypes[] = {
    [MONSTER_BAT] = {
        .name = "Bat",

        .race = RACE_ANIMAL_DIMINUTIVE,
        .class = CLASS_BAT,
        .faction = FACTION_WILD_ANIMAL,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 1,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 2,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .feats = {
            [FEAT_WEAPON_FINESSE] = true,
        },
    },
    [MONSTER_BUGBEAR] = {
        .name = "Bugbear",

        .race = RACE_HUMANOID_MEDIUM,
        .class = CLASS_BUGBEAR,
        .faction = FACTION_GOBLINOID,

        .level = 3,

        .ability_scores = {
            [ABILITY_STRENGTH] = 15,
            [ABILITY_DEXTERITY] = 12,
            [ABILITY_CONSTITUTION] = 13,
            [ABILITY_INTELLIGENCE] = 10,
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
            [ITEM_TYPE_JAVELIN] = 1,
        },
    },
    [MONSTER_DIRE_RAT] = {
        .name = "Dire Rat",

        .race = RACE_ANIMAL_SMALL,
        .class = CLASS_DIRE_RAT,
        .faction = FACTION_WILD_ANIMAL,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 10,
            [ABILITY_DEXTERITY] = 17,
            [ABILITY_CONSTITUTION] = 12,
            [ABILITY_INTELLIGENCE] = 1,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .feats = {
            [FEAT_WEAPON_FINESSE] = true,
        },
    },
    [MONSTER_GOBLIN] = {
        .name = "Goblin",

        .race = RACE_HUMANOID_SMALL,
        .class = CLASS_GOBLIN,
        .faction = FACTION_GOBLINOID,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 11,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 12,
            [ABILITY_INTELLIGENCE] = 10,
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
    [MONSTER_HOBGOBLIN] = {
        .name = "Hobgoblin",

        .race = RACE_HUMANOID_MEDIUM,
        .class = CLASS_HOBGOBLIN,
        .faction = FACTION_GOBLINOID,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 13,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 14,
            [ABILITY_INTELLIGENCE] = 10,
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
    [MONSTER_KOBOLD] = {
        .name = "Kobold",

        .race = RACE_HUMANOID_SMALL,
        .class = CLASS_KOBOLD,
        .faction = FACTION_KOBOLD,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 9,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 10,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },

        .feats = {
            [FEAT_ARMOR_PROFICIENCY_LIGHT] = true,
            [FEAT_WEAPON_PROFICIENCY_SIMPLE] = true,
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
                .min_stack = 1,
                .max_stack = 1,
            },
            [ITEM_TYPE_BULLET] = {
                .min_stack = 0,
                .max_stack = 5,
            },
        },
    },
    [MONSTER_RAT] = {
        .name = "Rat",

        .race = RACE_ANIMAL_TINY,
        .class = CLASS_RAT,
        .faction = FACTION_WILD_ANIMAL,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 2,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 2,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_LOW_LIGHT_VISION] = true,
        },

        .feats = {
            [FEAT_WEAPON_FINESSE] = true,
        },
    },
    [MONSTER_RED_DRAGON_WYRMLING] = {
        .name = "Wyrmling Red Dragon",

        .race = RACE_DRAGON_WYRMLING,
        .class = CLASS_RED_DRAGON_WYRMLING,
        .faction = FACTION_RED_DRAGON,

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
        .faction = FACTION_RED_DRAGON,

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
        .faction = FACTION_RED_DRAGON,

        .level = 34,

        .ability_scores = {
            [ABILITY_STRENGTH] = 39,
            [ABILITY_DEXTERITY] = 10,
            [ABILITY_CONSTITUTION] = 29,
            [ABILITY_INTELLIGENCE] = 24,
        },
    },
    [MONSTER_TROLL] = {
        .name = "Troll",

        .race = RACE_GIANT_LARGE,
        .class = CLASS_TROLL,
        .faction = FACTION_TROLL,

        .level = 6,

        .ability_scores = {
            [ABILITY_STRENGTH] = 23,
            [ABILITY_DEXTERITY] = 14,
            [ABILITY_CONSTITUTION] = 23,
            [ABILITY_INTELLIGENCE] = 6,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
        },
    },
    [MONSTER_SKELETON_WARRIOR] = {
        .name = "Skeleton Warrior",

        .race = RACE_UNDEAD,
        .class = CLASS_SKELETON_WARRIOR,
        .faction = FACTION_UNDEAD,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 13,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 0,
            [ABILITY_INTELLIGENCE] = 0,
        },

        .special_abilities = {
            [SPECIAL_ABILITY_DARKVISION] = true,
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
};
