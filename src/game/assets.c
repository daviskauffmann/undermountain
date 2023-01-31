#include "assets.h"

#include "color.h"
#include <libtcod.h>

const struct tile_common tile_common = {
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

        .is_walkable = true,
        .is_transparent = false,
    },
    [OBJECT_TYPE_CHEST] = {
        .name = "Chest",
        .glyph = '~',
        .color = {COLOR_SEPIA},
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = false,
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
        .is_transparent = false,
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
        .is_transparent = false,
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

const struct actor_common actor_common = {
    .turns_to_chase = 10,
};

const struct size_data size_database[] = {
    [SIZE_TINY] = {
        .name = "Tiny",

        .modifier = 2,
    },
    [SIZE_SMALL] = {
        .name = "Small",

        .modifier = 1,
    },
    [SIZE_MEDIUM] = {
        .name = "Medium",

        .modifier = 0,
    },
    [SIZE_LARGE] = {
        .name = "Large",

        .modifier = -1,
    },
    [SIZE_HUGE] = {
        .name = "Huge",

        .modifier = -2,
    },
};

const struct race_data race_database[] = {
    // player races
    [RACE_DWARF] = {
        .name = "Dwarf",
        .glyph = '@',

        .size = SIZE_SMALL,
        .speed = 0.8f,
    },
    [RACE_ELF] = {
        .name = "Elf",
        .glyph = '@',

        .size = SIZE_MEDIUM,
        .speed = 1.2f,
    },
    [RACE_HUMAN] = {
        .name = "Human",
        .glyph = '@',

        .size = SIZE_MEDIUM,
        .speed = 1.0f,
    },

    // monster races
    [RACE_BUGBEAR] = {
        .name = "Bugbear",
        .glyph = 'b',

        .size = SIZE_MEDIUM,
        .speed = 0.5f,
    },
    [RACE_DOG] = {
        .name = "Dog",
        .glyph = 'd',

        .size = SIZE_SMALL,
        .speed = 0.7f,
    },
    [RACE_GOBLIN] = {
        .name = "Goblin",
        .glyph = 'g',

        .size = SIZE_SMALL,
        .speed = 0.7f,
    },
    [RACE_JACKAL] = {
        .name = "Jackal",
        .glyph = 'j',

        .size = SIZE_SMALL,
        .speed = 1.5f,
    },
    [RACE_KOBOLD] = {
        .name = "Kobold",
        .glyph = 'k',

        .size = SIZE_SMALL,
        .speed = 0.7f,
    },
    [RACE_ORC] = {
        .name = "Orc",
        .glyph = 'o',

        .size = SIZE_MEDIUM,
        .speed = 0.5f,
    },
    [RACE_RAT] = {
        .name = "Rat",
        .glyph = 'r',

        .size = SIZE_TINY,
        .speed = 0.7f,
    },
    [RACE_SLIME] = {
        .name = "Slime",
        .glyph = 's',

        .size = SIZE_SMALL,
        .speed = 0.3f,
    },
};

const struct class_data class_database[] = {
    // player classes
    [CLASS_FIGHTER] = {
        .name = "Fighter",
        .color = {COLOR_BRASS},

        .hit_die = "1d10",
        .mana_die = "1d4",
    },
    [CLASS_WIZARD] = {
        .name = "Wizard",
        .color = {COLOR_AZURE},

        .hit_die = "1d4",
        .mana_die = "1d10",
    },
    [CLASS_ROGUE] = {
        .name = "Rogue",
        .color = {COLOR_YELLOW},

        .hit_die = "1d6",
        .mana_die = "1d4",
    },

    // monster classes
    [CLASS_DOG] = {
        .name = "Dog",
        .color = {COLOR_LIGHTEST_GRAY},

        .hit_die = "1d2",
        .mana_die = "0d0",
    },
    [CLASS_GOBLIN] = {
        .name = "Goblin",
        .color = {COLOR_LIGHTEST_GRAY},

        .hit_die = "1d1",
        .mana_die = "0d0",
    },
    [CLASS_JACKAL] = {
        .name = "Jackal",
        .color = {COLOR_LIGHTEST_GRAY},

        .hit_die = "1d2",
        .mana_die = "0d0",
    },
    [CLASS_KOBOLD] = {
        .name = "Kobold",
        .color = {COLOR_LIGHTEST_GRAY},

        .hit_die = "1d1",
        .mana_die = "0d0",
    },
    [CLASS_RAT] = {
        .name = "Jackal",
        .color = {COLOR_LIGHTEST_GRAY},

        .hit_die = "1d1",
        .mana_die = "0d0",
    },
    [CLASS_SLIME] = {
        .name = "Slime",
        .color = {COLOR_LIGHT_GREEN},

        .hit_die = "1d10",
        .mana_die = "0d0",
    },
};

const struct actor_prototype monster_prototypes[] = {
    [MONSTER_BUGBEAR] = {
        .name = "Bugbear",
        .race = RACE_BUGBEAR,
        .class = CLASS_FIGHTER,

        .level = 5,

        .ability_scores = {
            [ABILITY_STRENGTH] = 15,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 13,
            [ABILITY_INTELLIGENCE] = 10,
        },
    },
    [MONSTER_GOBLIN] = {
        .name = "Goblin",
        .race = RACE_GOBLIN,
        .class = CLASS_GOBLIN,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 8,
            [ABILITY_DEXTERITY] = 8,
            [ABILITY_CONSTITUTION] = 11,
            [ABILITY_INTELLIGENCE] = 10,
        },
    },
    [MONSTER_JACKAL] = {
        .name = "Jackal",
        .race = RACE_JACKAL,
        .class = CLASS_JACKAL,

        .level = 3,

        .ability_scores = {
            [ABILITY_STRENGTH] = 13,
            [ABILITY_DEXTERITY] = 11,
            [ABILITY_CONSTITUTION] = 15,
            [ABILITY_INTELLIGENCE] = 3,
        },
    },
    [MONSTER_KOBOLD] = {
        .name = "Kobold",
        .race = RACE_KOBOLD,
        .class = CLASS_KOBOLD,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 6,
            [ABILITY_DEXTERITY] = 13,
            [ABILITY_CONSTITUTION] = 11,
            [ABILITY_INTELLIGENCE] = 10,
        },
    },
    [MONSTER_ORC] = {
        .name = "Orc",
        .race = RACE_ORC,
        .class = CLASS_FIGHTER,

        .level = 3,

        .ability_scores = {
            [ABILITY_STRENGTH] = 15,
            [ABILITY_DEXTERITY] = 10,
            [ABILITY_CONSTITUTION] = 11,
            [ABILITY_INTELLIGENCE] = 9,
        },
    },
    [MONSTER_RAT] = {
        .name = "Rat",
        .race = RACE_RAT,
        .class = CLASS_RAT,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 3,
            [ABILITY_DEXTERITY] = 15,
            [ABILITY_CONSTITUTION] = 10,
            [ABILITY_INTELLIGENCE] = 3,
        },
    },
    [MONSTER_SLIME] = {
        .name = "Slime",
        .race = RACE_SLIME,
        .class = CLASS_SLIME,

        .level = 1,

        .ability_scores = {
            [ABILITY_STRENGTH] = 12,
            [ABILITY_DEXTERITY] = 3,
            [ABILITY_CONSTITUTION] = 11,
            [ABILITY_INTELLIGENCE] = 3,
        },
    },
};

const struct ability_data ability_database[] = {
    [ABILITY_STRENGTH] = {
        .name = "Strength",
    },
    [ABILITY_DEXTERITY] = {
        .name = "Dexterity",
    },
    [ABILITY_CONSTITUTION] = {
        .name = "Constitution",
    },
    [ABILITY_INTELLIGENCE] = {
        .name = "Intelligence",
    },
};

const struct corpse_common corpse_common = {
    .glyph = '%',
    .color = {COLOR_DARK_RED},
};

const struct equip_slot_data equip_slot_database[] = {
    [EQUIP_SLOT_NONE] = {
        .name = "None",
    },
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

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_ARROW,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 100,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_BOLT] = {
        .name = "Bolt",
        .glyph = '`',

        .equip_slot = EQUIP_SLOT_AMMUNITION,
        .size = SIZE_SMALL,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_BOLT,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 100,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_BULLET] = {
        .name = "Bullet",
        .glyph = '`',

        .equip_slot = EQUIP_SLOT_AMMUNITION,
        .size = SIZE_TINY,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_BULLET,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 100,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_DAGGER] = {
        .name = "Dagger",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_TINY,

        .armor_class = 0,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .damage = "1d4",
        .threat_range = 19,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_FULL_PLATE] = {
        .name = "Full Plate",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .size = SIZE_LARGE,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 8,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 1,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_GOLD] = {
        .name = "Gold",
        .glyph = '$',

        .equip_slot = EQUIP_SLOT_NONE,
        .size = SIZE_TINY,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = INT32_MAX,

        .max_durability = 0,
    },
    [BASE_ITEM_TYPE_GREATSWORD] = {
        .name = "Greatsword",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,

        .armor_class = 0,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .damage = "2d6",
        .threat_range = 19,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_CROSSBOW] = {
        .name = "Crossbow",
        .glyph = 'T',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,

        .armor_class = 0,

        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_BOLT,

        .damage = "1d10",
        .threat_range = 19,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_LARGE_SHIELD] = {
        .name = "Large Shield",
        .glyph = ')',

        .equip_slot = EQUIP_SLOT_SHIELD,
        .size = SIZE_MEDIUM,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 2,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 1,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_LONGBOW] = {
        .name = "Longbow",
        .glyph = '}',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,

        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_ARROW,

        .armor_class = 0,

        .damage = "1d8",
        .threat_range = 20,
        .critical_multiplier = 3,

        .max_stack = 1,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_LONGSWORD] = {
        .name = "Longsword",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_MEDIUM,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = "1d8",
        .threat_range = 19,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_MACE] = {
        .name = "Mace",
        .glyph = '!',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_SMALL,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = "1d6",
        .threat_range = 20,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_POTION] = {
        .name = "Potion",
        .glyph = '!',

        .equip_slot = EQUIP_SLOT_NONE,
        .size = SIZE_SMALL,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 10,

        .max_durability = 0,
    },
    [BASE_ITEM_TYPE_SCROLL] = {
        .name = "Scroll",
        .glyph = '%',

        .equip_slot = EQUIP_SLOT_NONE,
        .size = SIZE_SMALL,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 10,

        .max_durability = 0,
    },
    [BASE_ITEM_TYPE_SLING] = {
        .name = "Sling",
        .glyph = '?',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_SMALL,

        .armor_class = 0,

        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_BULLET,

        .damage = "1d4",
        .threat_range = 20,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_SMALL_SHIELD] = {
        .name = "Small Shield",
        .glyph = ')',

        .equip_slot = EQUIP_SLOT_SHIELD,
        .size = SIZE_SMALL,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 1,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 1,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_SPEAR] = {
        .name = "Spear",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .size = SIZE_LARGE,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = "1d8",
        .threat_range = 20,
        .critical_multiplier = 3,

        .max_stack = 1,

        .max_durability = 100,
    },
    [BASE_ITEM_TYPE_TOME] = {
        .name = "Tome",
        .glyph = '=',

        .equip_slot = EQUIP_SLOT_NONE,
        .size = SIZE_SMALL,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 1,

        .max_durability = 0,
    },
    [BASE_ITEM_TYPE_TOWER_SHIELD] = {
        .name = "Tower Shield",
        .glyph = ')',

        .equip_slot = EQUIP_SLOT_SHIELD,
        .size = SIZE_LARGE,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 3,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 1,

        .max_durability = 100,
    },
};

const struct item_data item_database[] = {
    [ITEM_TYPE_ARROW] = {
        .type = BASE_ITEM_TYPE_ARROW,

        .name = "Arrow",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_ARROW_1] = {
        .type = BASE_ITEM_TYPE_ARROW,

        .name = "Arrow + 1",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 1,

        .level = 3,

        .unique = false,
    },
    [ITEM_TYPE_BOLT] = {
        .type = BASE_ITEM_TYPE_BOLT,

        .name = "Bolt",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_BULLET] = {
        .type = BASE_ITEM_TYPE_BULLET,

        .name = "Bullet",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_COLD_IRON_BLADE] = {
        .type = BASE_ITEM_TYPE_LONGSWORD,

        .name = "Cold Iron Blade",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .level = 5,

        .unique = true,
    },
    [ITEM_TYPE_CROSSBOW] = {
        .type = BASE_ITEM_TYPE_CROSSBOW,

        .name = "Crossbow",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_DAGGER] = {
        .type = BASE_ITEM_TYPE_DAGGER,

        .name = "Dagger",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_FULL_PLATE] = {
        .type = BASE_ITEM_TYPE_FULL_PLATE,

        .name = "Full Plate",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_FULL_PLATE_1] = {
        .type = BASE_ITEM_TYPE_FULL_PLATE,

        .name = "Full Plate + 1",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .level = 3,

        .unique = false,
    },
    [ITEM_TYPE_GOLD] = {
        .type = BASE_ITEM_TYPE_GOLD,

        .name = "Gold",
        .description = "",
        .color = {COLOR_GOLD},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_GREATSWORD] = {
        .type = BASE_ITEM_TYPE_GREATSWORD,

        .name = "Greatsword",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_LARGE_SHIELD] = {
        .type = BASE_ITEM_TYPE_LARGE_SHIELD,

        .name = "Large Shield",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_LONGBOW] = {
        .type = BASE_ITEM_TYPE_LONGBOW,

        .name = "Longbow",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_LONGBOW_1] = {
        .type = BASE_ITEM_TYPE_LONGBOW,

        .name = "Longbow + 1",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .level = 3,

        .unique = false,
    },
    [ITEM_TYPE_LONGSWORD] = {
        .type = BASE_ITEM_TYPE_LONGSWORD,

        .name = "Longsword",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_LONGSWORD_1] = {
        .type = BASE_ITEM_TYPE_LONGSWORD,

        .name = "Longsword + 1",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .level = 3,

        .unique = false,
    },
    [ITEM_TYPE_MACE] = {
        .type = BASE_ITEM_TYPE_MACE,

        .name = "Mace",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_POTION_MINOR_HEAL] = {
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Minor Healing",
        .description = "",
        .color = {COLOR_RED},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_MINOR_HEAL,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_POTION_MINOR_MANA] = {
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Minor Mana",
        .description = "",
        .color = {COLOR_BLUE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_MINOR_MANA,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_SCEPTER_OF_UNITY] = {
        .type = BASE_ITEM_TYPE_MACE,

        .name = "Scepter of Unity",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 10,

        .unique = true,
    },
    [ITEM_TYPE_SCROLL_LIGHTNING] = {
        .type = BASE_ITEM_TYPE_SCROLL,

        .name = "Scroll of Lightning",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_LIGHTNING,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_SLING] = {
        .type = BASE_ITEM_TYPE_SLING,

        .name = "Sling",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_SMALL_SHIELD] = {
        .type = BASE_ITEM_TYPE_SMALL_SHIELD,

        .name = "Small Shield",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_SPEAR] = {
        .type = BASE_ITEM_TYPE_SPEAR,

        .name = "Spear",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_SPIKED_SHIELD] = {
        .type = BASE_ITEM_TYPE_LARGE_SHIELD,

        .name = "Spiked Shield",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = true,
    },
    [ITEM_TYPE_TOME_FIREBALL] = {
        .type = BASE_ITEM_TYPE_TOME,

        .name = "Tome of Fireball",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_FIREBALL,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_TOWER_SHIELD] = {
        .type = BASE_ITEM_TYPE_TOWER_SHIELD,

        .name = "Tower Shield",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
    },
    [ITEM_TYPE_TOWER_SHIELD_1] = {
        .type = BASE_ITEM_TYPE_TOWER_SHIELD,

        .name = "Tower Shield + 1",
        .description = "",
        .color = {COLOR_WHITE},

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .level = 3,

        .unique = false,
    },
};

const struct spell_data spell_database[] = {
    [SPELL_TYPE_MINOR_HEAL] = {
        .name = "Minor Heal",
        .range = SPELL_RANGE_SELF,
        .mana_cost = 1,
    },
    [SPELL_TYPE_MINOR_MANA] = {
        .name = "Minor Mana",
        .range = SPELL_RANGE_SELF,
        .mana_cost = 0,
    },
    [SPELL_TYPE_LIGHTNING] = {
        .name = "Lightning",
        .range = SPELL_RANGE_TARGET,
        .mana_cost = 2,
    },
    [SPELL_TYPE_FIREBALL] = {
        .name = "Fireball",
        .range = SPELL_RANGE_TARGET,
        .mana_cost = 5,
    },
};

const struct projectile_data projectile_database[] = {
    [PROJECTILE_TYPE_ARROW] = {
        .glyph = '`',
        .color = {COLOR_WHITE},

        .light_type = LIGHT_TYPE_NONE,

        .speed = 50.0f,
    },
    [PROJECTILE_TYPE_FIREBALL] = {
        .glyph = '*',
        .color = {COLOR_FLAME},

        .light_type = LIGHT_TYPE_FIREBALL,

        .speed = 30.0f,
    },
};
