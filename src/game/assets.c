#include "assets.h"

#include <libtcod.h>

struct tile_common tile_common;
struct tile_data tile_database[NUM_TILE_TYPES];
struct light_data light_database[NUM_LIGHT_TYPES];
struct object_data object_database[NUM_OBJECT_TYPES];
struct actor_common actor_common;
struct size_data size_database[NUM_SIZES];
struct race_data race_database[NUM_RACES];
struct class_data class_database[NUM_CLASSES];
struct actor_prototype monster_prototypes[NUM_MONSTERS];
struct ability_data ability_database[NUM_ABILITIES];
struct corpse_common corpse_common;
struct equip_slot_data equip_slot_database[NUM_EQUIP_SLOTS];
struct base_item_data base_item_database[NUM_BASE_ITEM_TYPES];
struct item_data item_database[NUM_ITEM_TYPES];
struct spell_data spell_database[NUM_SPELL_TYPES];
struct projectile_data projectile_database[NUM_PROJECTILE_TYPES];

void assets_load(void)
{
    // TODO: load from file

    tile_common = (struct tile_common){
        .ambient_light_color = TCOD_color_RGB(16, 16, 32),
        .ambient_light_intensity = 0.05f,
    };

    tile_database[TILE_TYPE_EMPTY] = (struct tile_data){
        .name = "Empty",
        .glyph = ' ',
        .color = TCOD_white,
        .is_walkable = true,
        .is_transparent = true,
    };
    tile_database[TILE_TYPE_FLOOR] = (struct tile_data){
        .name = "Floor",
        .glyph = '.',
        .color = TCOD_white,
        .is_walkable = true,
        .is_transparent = true,
    };
    tile_database[TILE_TYPE_GRASS] = (struct tile_data){
        .name = "Grass",
        .glyph = '.',
        .color = TCOD_light_green,
        .is_walkable = true,
        .is_transparent = true,
    };
    tile_database[TILE_TYPE_WALL] = (struct tile_data){
        .name = "Wall",
        .glyph = '#',
        .color = TCOD_white,
        .is_walkable = false,
        .is_transparent = false,
    };

    light_database[LIGHT_TYPE_NONE] = (struct light_data){
        .radius = -1,
        .color = TCOD_black,
        .intensity = 0,
        .flicker = false,
    };
    light_database[LIGHT_TYPE_ALTAR] = (struct light_data){
        .radius = 3,
        .color = TCOD_white,
        .intensity = 0.1f,
        .flicker = false,
    };
    light_database[LIGHT_TYPE_BRAZIER] = (struct light_data){
        .radius = 10,
        .color = TCOD_light_amber,
        .intensity = 0.25f,
        .flicker = true,
    };
    light_database[LIGHT_TYPE_FIREBALL] = (struct light_data){
        .radius = 5,
        .color = TCOD_flame,
        .intensity = 0.5f,
        .flicker = true,
    };
    light_database[LIGHT_TYPE_GLOW] = (struct light_data){
        .radius = 5,
        .color = TCOD_white,
        .intensity = 0.1f,
        .flicker = false,
    };
    light_database[LIGHT_TYPE_TORCH] = (struct light_data){
        .radius = 10,
        .color = TCOD_light_amber,
        .intensity = 0.25f,
        .flicker = true,
    };

    object_database[OBJECT_TYPE_ALTAR] = (struct object_data){
        .name = "Altar",
        .glyph = '_',
        .color = TCOD_white,
        .light_type = LIGHT_TYPE_ALTAR,

        .is_walkable = true,
        .is_transparent = false,
    };
    object_database[OBJECT_TYPE_BRAZIER] = (struct object_data){
        .name = "Brazier",
        .glyph = '*',
        .color = TCOD_light_amber,
        .light_type = LIGHT_TYPE_BRAZIER,

        .is_walkable = true,
        .is_transparent = false,
    };
    object_database[OBJECT_TYPE_CHEST] = (struct object_data){
        .name = "Chest",
        .glyph = '~',
        .color = TCOD_sepia,
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = false,
    };
    object_database[OBJECT_TYPE_DOOR_CLOSED] = (struct object_data){
        .name = "Closed Door",
        .glyph = '+',
        .color = TCOD_white,
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = false,
        .is_transparent = false,
    };
    object_database[OBJECT_TYPE_DOOR_OPEN] = (struct object_data){
        .name = "Open Door",
        .glyph = '-',
        .color = TCOD_white,
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = true,
    };
    object_database[OBJECT_TYPE_FOUNTAIN] = (struct object_data){
        .name = "Fountain",
        .glyph = '{',
        .color = TCOD_light_azure,
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = false,
    };
    object_database[OBJECT_TYPE_STAIR_DOWN] = (struct object_data){
        .name = "Stair Down",
        .glyph = '>',
        .color = TCOD_white,
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = true,
    };
    object_database[OBJECT_TYPE_STAIR_UP] = (struct object_data){
        .name = "Stair Up",
        .glyph = '<',
        .color = TCOD_white,
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = true,
    };
    object_database[OBJECT_TYPE_THRONE] = (struct object_data){
        .name = "Throne",
        .glyph = '\\',
        .color = TCOD_gold,
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = false,
    };
    object_database[OBJECT_TYPE_TRAP] = (struct object_data){
        .name = "Trap",
        .glyph = '^',
        .color = TCOD_white,
        .light_type = LIGHT_TYPE_NONE,

        .is_walkable = true,
        .is_transparent = true,
    };

    actor_common = (struct actor_common){
        .turns_to_chase = 10,
    };

    size_database[SIZE_TINY] = (struct size_data){
        .name = "Tiny",

        .modifier = 2,
    };
    size_database[SIZE_SMALL] = (struct size_data){
        .name = "Small",

        .modifier = 1,
    };
    size_database[SIZE_MEDIUM] = (struct size_data){
        .name = "Medium",

        .modifier = 0,
    };
    size_database[SIZE_LARGE] = (struct size_data){
        .name = "Large",

        .modifier = -1,
    };
    size_database[SIZE_HUGE] = (struct size_data){
        .name = "Huge",

        .modifier = -2,
    };

    race_database[RACE_HUMAN] = (struct race_data){
        .name = "Human",
        .glyph = '@',

        .size = SIZE_MEDIUM,
        .speed = 1.0f,
    };
    race_database[RACE_DWARF] = (struct race_data){
        .name = "Dwarf",
        .glyph = '@',

        .size = SIZE_SMALL,
        .speed = 0.8f,
    };
    race_database[RACE_ELF] = (struct race_data){
        .name = "Elf",
        .glyph = '@',

        .size = SIZE_MEDIUM,
        .speed = 1.2f,
    };

    race_database[RACE_BUGBEAR] = (struct race_data){
        .name = "Bugbear",
        .glyph = 'b',

        .size = SIZE_MEDIUM,
        .speed = 0.5f,
    };
    race_database[RACE_DOG] = (struct race_data){
        .name = "Dog",
        .glyph = 'd',

        .size = SIZE_SMALL,
        .speed = 0.7f,
    };
    race_database[RACE_GOBLIN] = (struct race_data){
        .name = "Goblin",
        .glyph = 'g',

        .size = SIZE_SMALL,
        .speed = 0.7f,
    };
    race_database[RACE_JACKAL] = (struct race_data){
        .name = "Jackal",
        .glyph = 'j',

        .size = SIZE_SMALL,
        .speed = 1.5f,
    };
    race_database[RACE_KOBOLD] = (struct race_data){
        .name = "Kobold",
        .glyph = 'k',

        .size = SIZE_SMALL,
        .speed = 0.7f,
    };
    race_database[RACE_ORC] = (struct race_data){
        .name = "Orc",
        .glyph = 'o',

        .size = SIZE_MEDIUM,
        .speed = 0.5f,
    };
    race_database[RACE_RAT] = (struct race_data){
        .name = "Rat",
        .glyph = 'r',

        .size = SIZE_TINY,
        .speed = 0.7f,
    };
    race_database[RACE_SLIME] = (struct race_data){
        .name = "Slime",
        .glyph = 's',

        .size = SIZE_SMALL,
        .speed = 0.3f,
    };

    class_database[CLASS_FIGHTER] = (struct class_data){
        .name = "Fighter",
        .color = TCOD_brass,

        .hit_die = "1d10",
        .mana_die = "1d4",
    };
    class_database[CLASS_WIZARD] = (struct class_data){
        .name = "Wizard",
        .color = TCOD_azure,

        .hit_die = "1d4",
        .mana_die = "1d10",
    };
    class_database[CLASS_ROGUE] = (struct class_data){
        .name = "Rogue",
        .color = TCOD_yellow,

        .hit_die = "1d6",
        .mana_die = "1d4",
    };

    class_database[CLASS_DOG] = (struct class_data){
        .name = "Dog",
        .color = TCOD_lightest_grey,

        .hit_die = "1d2",
        .mana_die = "0d0",
    };
    class_database[CLASS_GOBLIN] = (struct class_data){
        .name = "Goblin",
        .color = TCOD_lightest_grey,

        .hit_die = "1d1",
        .mana_die = "0d0",
    };
    class_database[CLASS_JACKAL] = (struct class_data){
        .name = "Jackal",
        .color = TCOD_lightest_grey,

        .hit_die = "1d2",
        .mana_die = "0d0",
    };
    class_database[CLASS_KOBOLD] = (struct class_data){
        .name = "Kobold",
        .color = TCOD_lightest_grey,

        .hit_die = "1d1",
        .mana_die = "0d0",
    };
    class_database[CLASS_RAT] = (struct class_data){
        .name = "Jackal",
        .color = TCOD_lightest_grey,

        .hit_die = "1d1",
        .mana_die = "0d0",
    };
    class_database[CLASS_SLIME] = (struct class_data){
        .name = "Slime",
        .color = TCOD_light_green,

        .hit_die = "1d10",
        .mana_die = "0d0",
    };

    monster_prototypes[MONSTER_BUGBEAR] = (struct actor_prototype){
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
    };
    monster_prototypes[MONSTER_GOBLIN] = (struct actor_prototype){
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
    };
    monster_prototypes[MONSTER_JACKAL] = (struct actor_prototype){
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
    };
    monster_prototypes[MONSTER_KOBOLD] = (struct actor_prototype){
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
    };
    monster_prototypes[MONSTER_ORC] = (struct actor_prototype){
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
    };
    monster_prototypes[MONSTER_RAT] = (struct actor_prototype){
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
    };
    monster_prototypes[MONSTER_SLIME] = (struct actor_prototype){
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
    };

    ability_database[ABILITY_STRENGTH] = (struct ability_data){
        .name = "Strength",
    };
    ability_database[ABILITY_DEXTERITY] = (struct ability_data){
        .name = "Dexterity",
    };
    ability_database[ABILITY_CONSTITUTION] = (struct ability_data){
        .name = "Constitution",
    };
    ability_database[ABILITY_INTELLIGENCE] = (struct ability_data){
        .name = "Intelligence",
    };

    corpse_common = (struct corpse_common){
        .glyph = '%',
        .color = TCOD_dark_red,
    };

    equip_slot_database[EQUIP_SLOT_NONE] = (struct equip_slot_data){
        .name = "None",
    };
    equip_slot_database[EQUIP_SLOT_AMMUNITION] = (struct equip_slot_data){
        .name = "Ammunition",
    };
    equip_slot_database[EQUIP_SLOT_ARMOR] = (struct equip_slot_data){
        .name = "Armor",
    };
    equip_slot_database[EQUIP_SLOT_SHIELD] = (struct equip_slot_data){
        .name = "Shield",
    };
    equip_slot_database[EQUIP_SLOT_WEAPON] = (struct equip_slot_data){
        .name = "Weapon",
    };

    base_item_database[BASE_ITEM_TYPE_ARROW] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_BOLT] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_BULLET] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_DAGGER] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_FULL_PLATE] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_GOLD] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_GREATSWORD] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_CROSSBOW] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_LARGE_SHIELD] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_LONGBOW] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_LONGSWORD] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_MACE] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_POTION] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_SCROLL] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_SLING] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_SMALL_SHIELD] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_SPEAR] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_TOME] = (struct base_item_data){
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
    };
    base_item_database[BASE_ITEM_TYPE_TOWER_SHIELD] = (struct base_item_data){
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
    };

    item_database[ITEM_TYPE_ARROW] = (struct item_data){
        .type = BASE_ITEM_TYPE_ARROW,

        .name = "Arrow",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_ARROW_1] = (struct item_data){
        .type = BASE_ITEM_TYPE_ARROW,

        .name = "Arrow + 1",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .level = 3,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_BOLT] = (struct item_data){
        .type = BASE_ITEM_TYPE_BOLT,

        .name = "Bolt",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_BULLET] = (struct item_data){
        .type = BASE_ITEM_TYPE_BULLET,

        .name = "Bullet",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_COLD_IRON_BLADE] = (struct item_data){
        .type = BASE_ITEM_TYPE_LONGSWORD,

        .name = "Cold Iron Blade",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .level = 5,

        .unique = true,
        .spawned = false,
    };
    item_database[ITEM_TYPE_CROSSBOW] = (struct item_data){
        .type = BASE_ITEM_TYPE_CROSSBOW,

        .name = "Crossbow",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_DAGGER] = (struct item_data){
        .type = BASE_ITEM_TYPE_DAGGER,

        .name = "Dagger",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_FULL_PLATE] = (struct item_data){
        .type = BASE_ITEM_TYPE_FULL_PLATE,

        .name = "Full Plate",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_FULL_PLATE_1] = (struct item_data){
        .type = BASE_ITEM_TYPE_FULL_PLATE,

        .name = "Full Plate + 1",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .level = 3,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_GOLD] = (struct item_data){
        .type = BASE_ITEM_TYPE_GOLD,

        .name = "Gold",
        .description = "",
        .color = TCOD_gold,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_GREATSWORD] = (struct item_data){
        .type = BASE_ITEM_TYPE_GREATSWORD,

        .name = "Greatsword",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_LARGE_SHIELD] = (struct item_data){
        .type = BASE_ITEM_TYPE_LARGE_SHIELD,

        .name = "Large Shield",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_LONGBOW] = (struct item_data){
        .type = BASE_ITEM_TYPE_LONGBOW,

        .name = "Longbow",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_LONGBOW_1] = (struct item_data){
        .type = BASE_ITEM_TYPE_LONGBOW,

        .name = "Longbow + 1",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .level = 3,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_LONGSWORD] = (struct item_data){
        .type = BASE_ITEM_TYPE_LONGSWORD,

        .name = "Longsword",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_LONGSWORD_1] = (struct item_data){
        .type = BASE_ITEM_TYPE_LONGSWORD,

        .name = "Longsword + 1",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .level = 3,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_MACE] = (struct item_data){
        .type = BASE_ITEM_TYPE_MACE,

        .name = "Mace",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_POTION_MINOR_HEAL] = (struct item_data){
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Minor Healing",
        .description = "",
        .color = TCOD_red,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_MINOR_HEAL,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_POTION_MINOR_MANA] = (struct item_data){
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Minor Mana",
        .description = "",
        .color = TCOD_blue,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_MINOR_MANA,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_SCEPTER_OF_UNITY] = (struct item_data){
        .type = BASE_ITEM_TYPE_MACE,

        .name = "Scepter of Unity",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 10,

        .unique = true,
        .spawned = false,
    };
    item_database[ITEM_TYPE_SCROLL_LIGHTNING] = (struct item_data){
        .type = BASE_ITEM_TYPE_SCROLL,

        .name = "Scroll of Lightning",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_LIGHTNING,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_SLING] = (struct item_data){
        .type = BASE_ITEM_TYPE_SLING,

        .name = "Sling",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_SMALL_SHIELD] = (struct item_data){
        .type = BASE_ITEM_TYPE_SMALL_SHIELD,

        .name = "Small Shield",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_SPEAR] = (struct item_data){
        .type = BASE_ITEM_TYPE_SPEAR,

        .name = "Spear",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_SPIKED_SHIELD] = (struct item_data){
        .type = BASE_ITEM_TYPE_LARGE_SHIELD,

        .name = "Spiked Shield",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = true,
        .spawned = false,
    };
    item_database[ITEM_TYPE_TOME_FIREBALL] = (struct item_data){
        .type = BASE_ITEM_TYPE_TOME,

        .name = "Tome of Fireball",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_FIREBALL,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_TOWER_SHIELD] = (struct item_data){
        .type = BASE_ITEM_TYPE_TOWER_SHIELD,

        .name = "Tower Shield",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .level = 1,

        .unique = false,
        .spawned = false,
    };
    item_database[ITEM_TYPE_TOWER_SHIELD_1] = (struct item_data){
        .type = BASE_ITEM_TYPE_TOWER_SHIELD,

        .name = "Tower Shield + 1",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .level = 3,

        .unique = false,
        .spawned = false,
    };

    spell_database[SPELL_TYPE_MINOR_HEAL] = (struct spell_data){
        .name = "Minor Heal",
        .range = SPELL_RANGE_SELF,
        .mana_cost = 1,
    };
    spell_database[SPELL_TYPE_MINOR_MANA] = (struct spell_data){
        .name = "Minor Mana",
        .range = SPELL_RANGE_SELF,
        .mana_cost = 0,
    };
    spell_database[SPELL_TYPE_LIGHTNING] = (struct spell_data){
        .name = "Lightning",
        .range = SPELL_RANGE_TARGET,
        .mana_cost = 2,
    };
    spell_database[SPELL_TYPE_FIREBALL] = (struct spell_data){
        .name = "Fireball",
        .range = SPELL_RANGE_TARGET,
        .mana_cost = 5,
    };

    projectile_database[PROJECTILE_TYPE_ARROW] = (struct projectile_data){
        .glyph = '`',
        .color = TCOD_white,

        .light_type = LIGHT_TYPE_NONE,

        .speed = 50.0f,
    };
    projectile_database[PROJECTILE_TYPE_FIREBALL] = (struct projectile_data){
        .glyph = '*',
        .color = TCOD_flame,

        .light_type = LIGHT_TYPE_FIREBALL,

        .speed = 30.0f,
    };
}
