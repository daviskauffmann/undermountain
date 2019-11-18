#include "assets.h"

#include <libtcod.h>

#include "util.h"

struct tile_common tile_common;
struct tile_data tile_datum[NUM_TILE_TYPES];
struct object_common object_common;
struct object_data object_datum[NUM_OBJECT_TYPES];
struct actor_common actor_common;
struct race_data race_datum[NUM_RACES];
struct class_data class_datum[NUM_CLASSES];
struct actor_prototype monster_prototypes[NUM_MONSTERS];
struct item_common item_common;
struct equip_slot_data equip_slot_datum[NUM_EQUIP_SLOTS];
struct item_data item_datum[NUM_ITEM_TYPES];

#define TILE_COMMON(_ambient_color, _ambient_intensity) \
    tile_common.ambient_color = _ambient_color;         \
    tile_common.ambient_intensity = _ambient_intensity;

#define TILE_DATA(_type, _name, _glyph, _color, _is_transparent, _is_walkable) \
    tile_datum[_type].name = _name;                                            \
    tile_datum[_type].glyph = _glyph;                                          \
    tile_datum[_type].color = _color;                                          \
    tile_datum[_type].is_transparent = _is_transparent;                        \
    tile_datum[_type].is_walkable = _is_walkable;

#define OBJECT_COMMON(_placeholder) \
    object_common.__placeholder = _placeholder;

#define OBJECT_DATA(_type, _name, _glyph, _is_transparent, _is_walkable) \
    object_datum[_type].name = _name;                                    \
    object_datum[_type].glyph = _glyph;                                  \
    object_datum[_type].is_transparent = _is_transparent;                \
    object_datum[_type].is_walkable = _is_walkable;

#define ACTOR_COMMON(_turns_to_chase, _glow_radius, _glow_color, _glow_intensity, _torch_radius, _torch_color, _torch_intensity) \
    actor_common.turns_to_chase = _turns_to_chase;                                                                               \
    actor_common.glow_radius = _glow_radius;                                                                                     \
    actor_common.glow_color = _glow_color;                                                                                       \
    actor_common.glow_intensity = _glow_intensity;                                                                               \
    actor_common.torch_radius = _torch_radius;                                                                                   \
    actor_common.torch_color = _torch_color;                                                                                     \
    actor_common.torch_intensity = _torch_intensity;

#define RACE_DATA(_race, _name, _glyph) \
    race_datum[_race].name = _name;     \
    race_datum[_race].glyph = _glyph;

#define CLASS_DATA(_class, _name, _color) \
    class_datum[_class].name = _name;     \
    class_datum[_class].color = _color;

#define MONSTER_PROTOTYPE(_monster, _name, _race, _class) \
    monster_prototypes[_monster].name = _name;            \
    monster_prototypes[_monster].race = _race;            \
    monster_prototypes[_monster].class = _class;

#define ABILITY_DATA(_ability, _name, _abbreviation, _description) \
    ability_datum[_ability].name = _name;                          \
    ability_datum[_ability].abbreviation = _abbreviation;          \
    ability_datum[_ability].description = _description;

#define ITEM_COMMON(_placeholder) \
    item_common.__placeholder = _placeholder;

#define EQUIP_SLOT_DATA(_type, _name, _label) \
    equip_slot_datum[_type].name = _name;     \
    equip_slot_datum[_type].label = _label;

#define BASE_ITEM_DATA(_type, _glyph, _color, _equip_slot, _ranged, _two_handed) \
    base_item_datum[_type].glyph = _glyph;                                       \
    base_item_datum[_type].color = _color;                                       \
    base_item_datum[_type].equip_slot = _equip_slot;                             \
    base_item_datum[_type].ranged = _ranged;                                     \
    base_item_datum[_type].two_handed = _two_handed;

#define ITEM_DATA(_type, _name, _description, _glyph, _color, _equip_slot, _two_handed, _armor, _block_chance, _min_damage, _max_damage, _ranged, _max_durability, _quaffable) \
    item_datum[_type].name = _name;                                                                                                                                            \
    item_datum[_type].description = _description;                                                                                                                              \
    item_datum[_type].glyph = _glyph;                                                                                                                                          \
    item_datum[_type].color = _color;                                                                                                                                          \
    item_datum[_type].equip_slot = _equip_slot;                                                                                                                                \
    item_datum[_type].two_handed = _two_handed;                                                                                                                                \
    item_datum[_type].armor = _armor;                                                                                                                                          \
    item_datum[_type].block_chance = _block_chance;                                                                                                                            \
    item_datum[_type].min_damage = _min_damage;                                                                                                                                \
    item_datum[_type].max_damage = _max_damage;                                                                                                                                \
    item_datum[_type].ranged = _ranged;                                                                                                                                        \
    item_datum[_type].max_durability = _max_durability;                                                                                                                        \
    item_datum[_type].quaffable = _quaffable;

void assets_load(void)
{
    TCOD_namegen_parse("assets/namegen.cfg", NULL);

    // TODO: load from file
    // see content.json

    TILE_COMMON(TCOD_color_RGB(16, 16, 32), 0.05f);

    TILE_DATA(TILE_TYPE_EMPTY, "Empty", ' ', TCOD_white, true, true);
    TILE_DATA(TILE_TYPE_FLOOR, "Floor", '.', TCOD_white, true, true);
    TILE_DATA(TILE_TYPE_WALL, "Wall", '#', TCOD_white, false, false);

    OBJECT_COMMON(0);

    OBJECT_DATA(OBJECT_TYPE_ALTAR, "Altar", '_', true, false);
    OBJECT_DATA(OBJECT_TYPE_BRAZIER, "Brazier", '*', true, false);
    OBJECT_DATA(OBJECT_TYPE_CHEST, "Chest", '~', true, false);
    OBJECT_DATA(OBJECT_TYPE_DOOR_CLOSED, "Closed Door", '+', false, false);
    OBJECT_DATA(OBJECT_TYPE_DOOR_OPEN, "Open Door", '-', true, true);
    OBJECT_DATA(OBJECT_TYPE_FOUNTAIN, "Fountain", '{', true, false);
    OBJECT_DATA(OBJECT_TYPE_STAIR_DOWN, "Stair Down", '>', true, true);
    OBJECT_DATA(OBJECT_TYPE_STAIR_UP, "Stair Up", '<', true, true);
    OBJECT_DATA(OBJECT_TYPE_THRONE, "Throne", '\\', true, false);

    ACTOR_COMMON(10, 5, TCOD_white, 0.05f, 10, TCOD_light_amber, 0.25f);

    RACE_DATA(RACE_HUMAN, "Human", '@');
    RACE_DATA(RACE_DWARF, "Dwarf", '@');
    RACE_DATA(RACE_ELF, "Elf", '@');

    RACE_DATA(RACE_ANIMAL, "Animal", 'a');
    RACE_DATA(RACE_BUGBEAR, "Bugbear", 'b');
    RACE_DATA(RACE_ORC, "Orc", 'o');
    RACE_DATA(RACE_SLIME, "Slime", 's');

    CLASS_DATA(CLASS_WARRIOR, "Warrior", TCOD_brass);
    CLASS_DATA(CLASS_MAGE, "Wizard", TCOD_azure);
    CLASS_DATA(CLASS_ROGUE, "Rogue", TCOD_yellow);

    CLASS_DATA(CLASS_ANIMAL, "Animal", TCOD_lightest_grey);
    CLASS_DATA(CLASS_SLIME, "Slime", TCOD_lightest_grey);

    MONSTER_PROTOTYPE(MONSTER_BUGBEAR, "Bugbear", RACE_BUGBEAR, CLASS_WARRIOR);
    MONSTER_PROTOTYPE(MONSTER_ORC, "Orc", RACE_ORC, CLASS_WARRIOR);
    MONSTER_PROTOTYPE(MONSTER_RAT, "Rat", RACE_ANIMAL, CLASS_ANIMAL);
    MONSTER_PROTOTYPE(MONSTER_SLIME, "Slime", RACE_SLIME, CLASS_SLIME);

    ITEM_COMMON(0);

    EQUIP_SLOT_DATA(EQUIP_SLOT_ARMOR, "Armor", "Armor ");
    EQUIP_SLOT_DATA(EQUIP_SLOT_MAIN_HAND, "Main Hand", "M-Hand");
    EQUIP_SLOT_DATA(EQUIP_SLOT_OFF_HAND, "Off Hand", "O-Hand");

    ITEM_DATA(ITEM_TYPE_IRON_ARMOR, "Iron Armor", "A set of iron armor.", '[', TCOD_white, EQUIP_SLOT_ARMOR, false, 3, 0, 0, 0, false, 100, false);
    ITEM_DATA(ITEM_TYPE_GREATSWORD, "Greatsword", "A two-handed straight sword.", '|', TCOD_white, EQUIP_SLOT_MAIN_HAND, true, 0, 0, 4, 12, false, 100, false);
    ITEM_DATA(ITEM_TYPE_LONGSWORD, "Longsword", "A straight sword.", '|', TCOD_white, EQUIP_SLOT_MAIN_HAND, false, 0, 0, 1, 8, false, 100, false);
    ITEM_DATA(ITEM_TYPE_LONGBOW, "Longbow", "A tall war bow.", '}', TCOD_white, EQUIP_SLOT_MAIN_HAND, true, 0, 0, 1, 8, true, 100, false);
    ITEM_DATA(ITEM_TYPE_KITE_SHIELD, "Kite Shield", "A medium-sized shield.", '|', TCOD_white, EQUIP_SLOT_OFF_HAND, false, 0, 0.3f, 1, 8, false, 100, false);
    ITEM_DATA(ITEM_TYPE_HEALING_POTION, "Healing Potion", "Restores full health.", '!', TCOD_red, EQUIP_SLOT_NONE, false, 0, 0, 0, 0, false, 0, true);
}

void assets_unload(void)
{
    TCOD_namegen_destroy();
}
