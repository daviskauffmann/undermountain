#include "assets.h"

#include <libtcod.h>

struct tile_common tile_common;
struct tile_datum tile_data[NUM_TILE_TYPES];
struct object_common object_common;
struct object_datum object_data[NUM_OBJECT_TYPES];
struct light_datum light_data[NUM_LIGHT_TYPES];
struct actor_common actor_common;
struct race_datum race_data[NUM_RACES];
struct class_datum class_data[NUM_CLASSES];
struct actor_prototype monster_prototypes[NUM_MONSTERS];
struct corpse_common corpse_common;
struct item_common item_common;
struct equip_slot_datum equip_slot_data[NUM_EQUIP_SLOTS];
struct base_item_datum base_item_data[NUM_BASE_ITEM_TYPES];
struct item_datum item_data[NUM_ITEM_TYPES];
struct spell_datum spell_data[NUM_SPELL_TYPES];
struct projectile_datum projectile_data[NUM_PROJECTILE_TYPES];

void assets_load(void)
{
    // TODO: load from file

    tile_common = (struct tile_common){
        .ambient_light_color = TCOD_color_RGB(16, 16, 32),
        .ambient_light_intensity = 0.05f,
    };

    tile_data[TILE_TYPE_EMPTY] = (struct tile_datum){
        .name = "Empty",
        .glyph = ' ',
        .color = TCOD_white,
        .is_walkable = true,
        .is_transparent = true,
    };
    tile_data[TILE_TYPE_FLOOR] = (struct tile_datum){
        .name = "Floor",
        .glyph = '.',
        .color = TCOD_white,
        .is_walkable = true,
        .is_transparent = true,
    };
    tile_data[TILE_TYPE_GRASS] = (struct tile_datum){
        .name = "Grass",
        .glyph = '.',
        .color = TCOD_light_green,
        .is_walkable = true,
        .is_transparent = true,
    };
    tile_data[TILE_TYPE_WALL] = (struct tile_datum){
        .name = "Wall",
        .glyph = '#',
        .color = TCOD_white,
        .is_walkable = false,
        .is_transparent = false,
    };

    object_common = (struct object_common){
        .__placeholder = 0,
    };

    object_data[OBJECT_TYPE_ALTAR] = (struct object_datum){
        .name = "Altar",
        .glyph = '_',
        .is_walkable = true,
        .is_transparent = false,
    };
    object_data[OBJECT_TYPE_BRAZIER] = (struct object_datum){
        .name = "Brazier",
        .glyph = '*',
        .is_walkable = true,
        .is_transparent = false,
    };
    object_data[OBJECT_TYPE_CHEST] = (struct object_datum){
        .name = "Chest",
        .glyph = '~',
        .is_walkable = true,
        .is_transparent = false,
    };
    object_data[OBJECT_TYPE_DOOR_CLOSED] = (struct object_datum){
        .name = "Closed Door",
        .glyph = '+',
        .is_walkable = false,
        .is_transparent = false,
    };
    object_data[OBJECT_TYPE_DOOR_OPEN] = (struct object_datum){
        .name = "Open Door",
        .glyph = '-',
        .is_walkable = true,
        .is_transparent = true,
    };
    object_data[OBJECT_TYPE_FOUNTAIN] = (struct object_datum){
        .name = "Fountain",
        .glyph = '{',
        .is_walkable = true,
        .is_transparent = false,
    };
    object_data[OBJECT_TYPE_STAIR_DOWN] = (struct object_datum){
        .name = "Stair Down",
        .glyph = '>',
        .is_walkable = true,
        .is_transparent = true,
    };
    object_data[OBJECT_TYPE_STAIR_UP] = (struct object_datum){
        .name = "Stair Up",
        .glyph = '<',
        .is_walkable = true,
        .is_transparent = true,
    };
    object_data[OBJECT_TYPE_THRONE] = (struct object_datum){
        .name = "Throne",
        .glyph = '\\',
        .is_walkable = true,
        .is_transparent = false,
    };
    object_data[OBJECT_TYPE_TRAP] = (struct object_datum){
        .name = "Trap",
        .glyph = '^',
        .is_walkable = true,
        .is_transparent = true,
    };

    light_data[LIGHT_TYPE_NONE] = (struct light_datum){
        .radius = -1,
        .color = TCOD_black,
        .intensity = 0,
        .flicker = false,
    };
    light_data[LIGHT_TYPE_GLOW] = (struct light_datum){
        .radius = 5,
        .color = TCOD_white,
        .intensity = 0.1f,
        .flicker = false,
    };
    light_data[LIGHT_TYPE_TORCH] = (struct light_datum){
        .radius = 10,
        .color = TCOD_light_amber,
        .intensity = 0.25f,
        .flicker = true,
    };
    light_data[LIGHT_TYPE_FIREBALL] = (struct light_datum){
        .radius = 5,
        .color = TCOD_flame,
        .intensity = 0.5f,
        .flicker = true,
    };

    actor_common = (struct actor_common){
        .turns_to_chase = 10,
    };

    race_data[RACE_HUMAN] = (struct race_datum){
        .name = "Human",
        .glyph = '@',
        .speed = 1.0f,
    };
    race_data[RACE_DWARF] = (struct race_datum){
        .name = "Dwarf",
        .glyph = '@',
        .speed = 0.8f,
    };
    race_data[RACE_ELF] = (struct race_datum){
        .name = "Elf",
        .glyph = '@',
        .speed = 1.2f,
    };

    race_data[RACE_BUGBEAR] = (struct race_datum){
        .name = "Bugbear",
        .glyph = 'b',
        .speed = 0.5f,
    };
    race_data[RACE_DOG] = (struct race_datum){
        .name = "Dog",
        .glyph = 'd',
        .speed = 0.7f,
    };
    race_data[RACE_JACKAL] = (struct race_datum){
        .name = "Jackal",
        .glyph = 'j',
        .speed = 1.8f,
    };
    race_data[RACE_ORC] = (struct race_datum){
        .name = "Orc",
        .glyph = 'o',
        .speed = 0.5f,
    };
    race_data[RACE_RAT] = (struct race_datum){
        .name = "Rat",
        .glyph = 'r',
        .speed = 0.7f,
    };
    race_data[RACE_SLIME] = (struct race_datum){
        .name = "Slime",
        .glyph = 's',
        .speed = 0.3f,
    };

    class_data[CLASS_WARRIOR] = (struct class_datum){
        .name = "Warrior",
        .color = TCOD_brass,
    };
    class_data[CLASS_MAGE] = (struct class_datum){
        .name = "Wizard",
        .color = TCOD_azure,
    };
    class_data[CLASS_ROGUE] = (struct class_datum){
        .name = "Rogue",
        .color = TCOD_yellow,
    };

    class_data[CLASS_ANIMAL] = (struct class_datum){
        .name = "Animal",
        .color = TCOD_lightest_grey,
    };
    class_data[CLASS_SLIME] = (struct class_datum){
        .name = "Slime",
        .color = TCOD_light_green,
    };

    monster_prototypes[MONSTER_BUGBEAR] = (struct actor_prototype){
        .name = "Bugbear",
        .race = RACE_BUGBEAR,
        .class = CLASS_WARRIOR,
    };
    monster_prototypes[MONSTER_JACKAL] = (struct actor_prototype){
        .name = "Jackal",
        .race = RACE_JACKAL,
        .class = CLASS_ANIMAL,
    };
    monster_prototypes[MONSTER_ORC] = (struct actor_prototype){
        .name = "Orc",
        .race = RACE_ORC,
        .class = CLASS_WARRIOR,
    };
    monster_prototypes[MONSTER_RAT] = (struct actor_prototype){
        .name = "Rat",
        .race = RACE_RAT,
        .class = CLASS_ANIMAL,
    };
    monster_prototypes[MONSTER_SLIME] = (struct actor_prototype){
        .name = "Slime",
        .race = RACE_SLIME,
        .class = CLASS_SLIME,
    };

    corpse_common = (struct corpse_common){
        .glyph = '%',
        .color = TCOD_dark_red,
    };

    item_common = (struct item_common){
        .__placeholder = 0,
    };

    equip_slot_data[EQUIP_SLOT_NONE] = (struct equip_slot_datum){
        .name = "None",
    };
    equip_slot_data[EQUIP_SLOT_AMMUNITION] = (struct equip_slot_datum){
        .name = "Ammunition",
    };
    equip_slot_data[EQUIP_SLOT_ARMOR] = (struct equip_slot_datum){
        .name = "Armor",
    };
    equip_slot_data[EQUIP_SLOT_SHIELD] = (struct equip_slot_datum){
        .name = "Shield",
    };
    equip_slot_data[EQUIP_SLOT_WEAPON] = (struct equip_slot_datum){
        .name = "Weapon",
    };

    base_item_data[BASE_ITEM_TYPE_ARROW] = (struct base_item_datum){
        .name = "Arrow",
        .glyph = '`',

        .equip_slot = EQUIP_SLOT_AMMUNITION,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_ARROW,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 100,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_BOLT] = (struct base_item_datum){
        .name = "Bolt",
        .glyph = '`',

        .equip_slot = EQUIP_SLOT_AMMUNITION,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_BOLT,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 100,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_BULLET] = (struct base_item_datum){
        .name = "Bullet",
        .glyph = '`',

        .equip_slot = EQUIP_SLOT_AMMUNITION,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_BULLET,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 100,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_DAGGER] = (struct base_item_datum){
        .name = "Dagger",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .two_handed = true,

        .armor_class = 0,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .damage = "1d4",
        .threat_range = 19,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_FULL_PLATE] = (struct base_item_datum){
        .name = "Full Plate",
        .glyph = '[',

        .equip_slot = EQUIP_SLOT_ARMOR,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 8,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 1,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_GOLD] = (struct base_item_datum){
        .name = "Gold",
        .glyph = '$',

        .equip_slot = EQUIP_SLOT_NONE,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = INT32_MAX,

        .max_durability = 0,
    };
    base_item_data[BASE_ITEM_TYPE_GREATSWORD] = (struct base_item_datum){
        .name = "Greatsword",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .two_handed = true,

        .armor_class = 0,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .damage = "2d6",
        .threat_range = 19,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_CROSSBOW] = (struct base_item_datum){
        .name = "Crossbow",
        .glyph = 'T',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .two_handed = true,

        .armor_class = 0,

        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_BOLT,

        .damage = "1d10",
        .threat_range = 19,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_LARGE_SHIELD] = (struct base_item_datum){
        .name = "Large Shield",
        .glyph = ')',

        .equip_slot = EQUIP_SLOT_SHIELD,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 2,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 1,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_LONGBOW] = (struct base_item_datum){
        .name = "Longbow",
        .glyph = '}',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .two_handed = true,

        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_ARROW,

        .armor_class = 0,

        .damage = "1d8",
        .threat_range = 20,
        .critical_multiplier = 3,

        .max_stack = 1,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_LONGSWORD] = (struct base_item_datum){
        .name = "Longsword",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = "1d8",
        .threat_range = 19,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_MACE] = (struct base_item_datum){
        .name = "Mace",
        .glyph = '!',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = "1d6",
        .threat_range = 20,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_POTION] = (struct base_item_datum){
        .name = "Potion",
        .glyph = '!',

        .equip_slot = EQUIP_SLOT_NONE,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 10,

        .max_durability = 0,
    };
    base_item_data[BASE_ITEM_TYPE_SCROLL] = (struct base_item_datum){
        .name = "Scroll",
        .glyph = '%',

        .equip_slot = EQUIP_SLOT_NONE,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 10,

        .max_durability = 0,
    };
    base_item_data[BASE_ITEM_TYPE_SLING] = (struct base_item_datum){
        .name = "Sling",
        .glyph = '?',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .two_handed = false,

        .armor_class = 0,

        .ranged = true,
        .ammunition_type = AMMUNITION_TYPE_BULLET,

        .damage = "1d4",
        .threat_range = 20,
        .critical_multiplier = 2,

        .max_stack = 1,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_SMALL_SHIELD] = (struct base_item_datum){
        .name = "Small Shield",
        .glyph = ')',

        .equip_slot = EQUIP_SLOT_SHIELD,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 1,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 1,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_SPEAR] = (struct base_item_datum){
        .name = "Spear",
        .glyph = '|',

        .equip_slot = EQUIP_SLOT_WEAPON,
        .two_handed = true,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = "1d8",
        .threat_range = 20,
        .critical_multiplier = 3,

        .max_stack = 1,

        .max_durability = 100,
    };
    base_item_data[BASE_ITEM_TYPE_TOME] = (struct base_item_datum){
        .name = "Tome",
        .glyph = '=',

        .equip_slot = EQUIP_SLOT_NONE,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 0,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 1,

        .max_durability = 0,
    };
    base_item_data[BASE_ITEM_TYPE_TOWER_SHIELD] = (struct base_item_datum){
        .name = "Tower Shield",
        .glyph = ')',

        .equip_slot = EQUIP_SLOT_SHIELD,
        .two_handed = false,

        .ranged = false,
        .ammunition_type = AMMUNITION_TYPE_NONE,

        .armor_class = 3,

        .damage = NULL,
        .threat_range = 0,
        .critical_multiplier = 0,

        .max_stack = 1,

        .max_durability = 100,
    };

    item_data[ITEM_TYPE_ARROW] = (struct item_datum){
        .type = BASE_ITEM_TYPE_ARROW,

        .name = "Arrow",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_ARROW_1] = (struct item_datum){
        .type = BASE_ITEM_TYPE_ARROW,

        .name = "Arrow + 1",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_BOLT] = (struct item_datum){
        .type = BASE_ITEM_TYPE_BOLT,

        .name = "Bolt",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_BULLET] = (struct item_datum){
        .type = BASE_ITEM_TYPE_BULLET,

        .name = "Bullet",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_COLD_IRON_BLADE] = (struct item_datum){
        .type = BASE_ITEM_TYPE_LONGSWORD,

        .name = "Cold Iron Blade",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .unique = true,
        .spawned = false,
    };
    item_data[ITEM_TYPE_CROSSBOW] = (struct item_datum){
        .type = BASE_ITEM_TYPE_CROSSBOW,

        .name = "Crossbow",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_DAGGER] = (struct item_datum){
        .type = BASE_ITEM_TYPE_DAGGER,

        .name = "Dagger",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_FULL_PLATE] = (struct item_datum){
        .type = BASE_ITEM_TYPE_FULL_PLATE,

        .name = "Full Plate",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_FULL_PLATE_1] = (struct item_datum){
        .type = BASE_ITEM_TYPE_FULL_PLATE,

        .name = "Full Plate + 1",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_GOLD] = (struct item_datum){
        .type = BASE_ITEM_TYPE_GOLD,

        .name = "Gold",
        .description = "",
        .color = TCOD_gold,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_GREATSWORD] = (struct item_datum){
        .type = BASE_ITEM_TYPE_GREATSWORD,

        .name = "Greatsword",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_LARGE_SHIELD] = (struct item_datum){
        .type = BASE_ITEM_TYPE_LARGE_SHIELD,

        .name = "Large Shield",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_LONGBOW] = (struct item_datum){
        .type = BASE_ITEM_TYPE_LONGBOW,

        .name = "Longbow",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_LONGBOW_1] = (struct item_datum){
        .type = BASE_ITEM_TYPE_LONGBOW,

        .name = "Longbow + 1",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_LONGSWORD] = (struct item_datum){
        .type = BASE_ITEM_TYPE_LONGSWORD,

        .name = "Longsword",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_LONGSWORD_1] = (struct item_datum){
        .type = BASE_ITEM_TYPE_LONGSWORD,

        .name = "Longsword + 1",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_MACE] = (struct item_datum){
        .type = BASE_ITEM_TYPE_MACE,

        .name = "Mace",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_POTION_MINOR_HEAL] = (struct item_datum){
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Minor Heal",
        .description = "",
        .color = TCOD_red,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_MINOR_HEAL,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_SCEPTER_OF_UNITY] = (struct item_datum){
        .type = BASE_ITEM_TYPE_MACE,

        .name = "Scepter of Unity",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = true,
        .spawned = false,
    };
    item_data[ITEM_TYPE_SCROLL_LIGHTNING] = (struct item_datum){
        .type = BASE_ITEM_TYPE_SCROLL,

        .name = "Scroll of Lightning",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_LIGHTNING,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_SLING] = (struct item_datum){
        .type = BASE_ITEM_TYPE_SLING,

        .name = "Sling",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_SMALL_SHIELD] = (struct item_datum){
        .type = BASE_ITEM_TYPE_SMALL_SHIELD,

        .name = "Small Shield",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_SPEAR] = (struct item_datum){
        .type = BASE_ITEM_TYPE_SPEAR,

        .name = "Spear",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_SPIKED_SHIELD] = (struct item_datum){
        .type = BASE_ITEM_TYPE_LARGE_SHIELD,

        .name = "Spiked Shield",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = true,
        .spawned = false,
    };
    item_data[ITEM_TYPE_TOME_FIREBALL] = (struct item_datum){
        .type = BASE_ITEM_TYPE_TOME,

        .name = "Tome of Fireball",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_FIREBALL,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_TOWER_SHIELD] = (struct item_datum){
        .type = BASE_ITEM_TYPE_TOWER_SHIELD,

        .name = "Tower Shield",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 0,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };
    item_data[ITEM_TYPE_TOWER_SHIELD_1] = (struct item_datum){
        .type = BASE_ITEM_TYPE_TOWER_SHIELD,

        .name = "Tower Shield + 1",
        .description = "",
        .color = TCOD_white,

        .enhancement_bonus = 1,

        .spell_type = SPELL_TYPE_NONE,

        .unique = false,
        .spawned = false,
    };

    spell_data[SPELL_TYPE_MINOR_HEAL] = (struct spell_datum){
        .name = "Minor Heal",
        .range = SPELL_RANGE_SELF,
        .mana_cost = 40,
    };
    spell_data[SPELL_TYPE_LIGHTNING] = (struct spell_datum){
        .name = "Lightning",
        .range = SPELL_RANGE_TARGET,
        .mana_cost = 50,
    };
    spell_data[SPELL_TYPE_FIREBALL] = (struct spell_datum){
        .name = "Fireball",
        .range = SPELL_RANGE_TARGET,
        .mana_cost = 70,
    };

    projectile_data[PROJECTILE_TYPE_ARROW] = (struct projectile_datum){
        .glyph = '`',
        .color = TCOD_white,

        .light_type = LIGHT_TYPE_NONE,

        .speed = 50.0f,
    };
    projectile_data[PROJECTILE_TYPE_FIREBALL] = (struct projectile_datum){
        .glyph = '*',
        .color = TCOD_flame,

        .light_type = LIGHT_TYPE_FIREBALL,

        .speed = 30.0f,
    };
}
