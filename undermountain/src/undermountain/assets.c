#include <undermountain/undermountain.h>

// TODO: load from file?

struct tile_common tile_common;
struct tile_info tile_info[NUM_TILE_TYPES];
struct object_common object_common;
struct object_info object_info[NUM_OBJECT_TYPES];
struct actor_common actor_common;
struct race_info race_info[NUM_RACES];
struct class_info class_info[NUM_CLASSES];
struct prototype monster_prototype[NUM_MONSTERS];
struct ability_info ability_info[NUM_ABILITIES];
struct item_common item_common;
struct equip_slot_info equip_slot_info[NUM_EQUIP_SLOTS];
struct base_item_info base_item_info[NUM_BASE_ITEMS];
struct item_info item_info[NUM_ITEM_TYPES];

#define TILE_COMMON(_shadow_color) tile_common.shadow_color = _shadow_color;

#define TILE_INFO(_type, _name, _glyph, _color, _is_transparent, _is_walkable) \
    tile_info[_type].name = _name;                                             \
    tile_info[_type].glyph = _glyph;                                           \
    tile_info[_type].color = _color;                                           \
    tile_info[_type].is_transparent = _is_transparent;                         \
    tile_info[_type].is_walkable = _is_walkable;

#define OBJECT_COMMON(_placeholder) \
    object_common.__placeholder = _placeholder;

#define OBJECT_INFO(_type, _name, _glyph, _is_transparent, _is_walkable) \
    object_info[_type].name = _name;                                     \
    object_info[_type].glyph = _glyph;                                   \
    object_info[_type].is_transparent = _is_transparent;                 \
    object_info[_type].is_walkable = _is_walkable;

#define ACTOR_COMMON(_turns_to_chase, _glow_radius, _glow_color, _torch_radius, _torch_color) \
    actor_common.turns_to_chase = _turns_to_chase;                                            \
    actor_common.glow_radius = _glow_radius;                                                  \
    actor_common.glow_color = _glow_color;                                                    \
    actor_common.torch_radius = _torch_radius;                                                \
    actor_common.torch_color = _torch_color;

#define RACE_INFO(_race, _name, _glyph, _size) \
    race_info[_race].name = _name;             \
    race_info[_race].glyph = _glyph;           \
    race_info[_race].size = _size;

#define CLASS_INFO(_class, _name, _color, _hit_die) \
    class_info[_class].name = _name;                \
    class_info[_class].color = _color;              \
    class_info[_class].hit_die = _hit_die;

#define MONSTER_PROTOTYPE(_monster, _name, _race, _class) \
    monster_prototype[_monster].name = _name;             \
    monster_prototype[_monster].race = _race;             \
    monster_prototype[_monster].class = _class;

#define CLASS_INFO(_class, _name, _color, _hit_die) \
    class_info[_class].name = _name;                \
    class_info[_class].color = _color;              \
    class_info[_class].hit_die = _hit_die;

#define MONSTER_PROTOTYPE(_monster, _name, _race, _class) \
    monster_prototype[_monster].name = _name;             \
    monster_prototype[_monster].race = _race;             \
    monster_prototype[_monster].class = _class;

#define ABILITY_INFO(_ability, _name, _abbreviation, _description) \
    ability_info[_ability].name = _name;                           \
    ability_info[_ability].abbreviation = _abbreviation;           \
    ability_info[_ability].description = _description;

#define ITEM_COMMON(_placeholder) \
    item_common.__placeholder = _placeholder;

#define EQUIP_SLOT_INFO(_type, _name, _label) \
    equip_slot_info[_type].name = _name;      \
    equip_slot_info[_type].label = _label;

#define BASE_ITEM_INFO(_type, _glyph, _color, _weight, _equip_slot, _damage_type, _weapon_size, _ranged, _num_dice, _die_to_roll, _crit_threat, _crit_mult, _base_cost, _stack, _base_ac, _armor_check_penalty, _arcane_spell_failure) \
    base_item_info[_type].glyph = _glyph;                                                                                                                                                                                              \
    base_item_info[_type].color = _color;                                                                                                                                                                                              \
    base_item_info[_type].weight = _weight;                                                                                                                                                                                            \
    base_item_info[_type].equip_slot = _equip_slot;                                                                                                                                                                                    \
    base_item_info[_type].damage_type = _damage_type;                                                                                                                                                                                  \
    base_item_info[_type].weapon_size = _weapon_size;                                                                                                                                                                                  \
    base_item_info[_type].ranged = _ranged;                                                                                                                                                                                            \
    base_item_info[_type].num_dice = _num_dice;                                                                                                                                                                                        \
    base_item_info[_type].die_to_roll = _die_to_roll;                                                                                                                                                                                  \
    base_item_info[_type].crit_threat = _crit_threat;                                                                                                                                                                                  \
    base_item_info[_type].crit_mult = _crit_mult;                                                                                                                                                                                      \
    base_item_info[_type].base_cost = _base_cost;                                                                                                                                                                                      \
    base_item_info[_type].stack = _stack;                                                                                                                                                                                              \
    base_item_info[_type].base_ac = _base_ac;                                                                                                                                                                                          \
    base_item_info[_type].armor_check_penalty = _armor_check_penalty;                                                                                                                                                                  \
    base_item_info[_type].arcane_spell_failure = _arcane_spell_failure;

#define ITEM_INFO(_type, _base_item, _name, _description) \
    item_info[_type].base_item = _base_item;              \
    item_info[_type].name = _name;                        \
    item_info[_type].description = _description;          \
    item_info[_type].item_properties = TCOD_list_new();

void assets_load(void)
{
    TILE_COMMON(TCOD_color_RGB(16, 16, 32));

    TILE_INFO(TILE_TYPE_EMPTY, "Empty", ' ', TCOD_white, true, true);
    TILE_INFO(TILE_TYPE_FLOOR, "Floor", '.', TCOD_white, true, true);
    TILE_INFO(TILE_TYPE_WALL, "Wall", '#', TCOD_white, false, false);

    OBJECT_COMMON(0);

    OBJECT_INFO(OBJECT_TYPE_ALTAR, "Altar", '_', true, false);
    OBJECT_INFO(OBJECT_TYPE_BRAZIER, "Brazier", '*', true, false);
    OBJECT_INFO(OBJECT_TYPE_CHEST, "Chest", '~', true, false);
    OBJECT_INFO(OBJECT_TYPE_DOOR_CLOSED, "Closed Door", '+', false, false);
    OBJECT_INFO(OBJECT_TYPE_DOOR_OPEN, "Open Door", '-', true, true);
    OBJECT_INFO(OBJECT_TYPE_FOUNTAIN, "Fountain", '{', true, false);
    OBJECT_INFO(OBJECT_TYPE_STAIR_DOWN, "Stair Down", '>', true, true);
    OBJECT_INFO(OBJECT_TYPE_STAIR_UP, "Stair Up", '<', true, true);
    OBJECT_INFO(OBJECT_TYPE_THRONE, "Throne", '\\', true, false);

    ACTOR_COMMON(10, 5, TCOD_white, 10, TCOD_light_amber);

    RACE_INFO(RACE_DWARF, "Dwarf", '@', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_ELF, "Elf", '@', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_GNOME, "Gnome", '@', RACE_SIZE_SMALL);
    RACE_INFO(RACE_HALF_ELF, "Half-Elf", '@', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_HALF_ORC, "Half-Orc", '@', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_HALFLING, "Halfling", '@', RACE_SIZE_SMALL);
    RACE_INFO(RACE_HUMAN, "Human", '@', RACE_SIZE_MEDIUM);

    RACE_INFO(RACE_ABBERATION, "Abberation", 'A', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_ANIMAL, "Animal", 'a', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_BEAST, "Beast", 'b', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_CONSTRUCT, "Construct", 'c', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_DRAGON, "Dragon", 'D', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_ELEMENTAL, "Elemental", 'e', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_FEY, "Fey", 'f', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_GIANT, "Giant", 'G', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_GOBLINOID, "Goblinoid", 'g', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_MAGICAL_BEAST, "Magical Beast", 'M', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_MONSTROUS_HUMANOID, "Monstrous Humanoid", 'm', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_OOZE, "Ooze", 'O', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_ORC, "Orc", 'o', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_OUTSIDER, "Outsider", 'z', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_REPTILLIAN, "Reptillian", 'r', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_SHAPECHANGER, "Shapechanger", 's', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_UNDEAD, "Undead", 'u', RACE_SIZE_MEDIUM);
    RACE_INFO(RACE_VERMIN, "Vermin", 'v', RACE_SIZE_MEDIUM);

    CLASS_INFO(CLASS_BARBARIAN, "Barbarian", TCOD_amber, 12);
    CLASS_INFO(CLASS_BARD, "Bard", TCOD_purple, 6);
    CLASS_INFO(CLASS_CLERIC, "Cleric", TCOD_white, 8);
    CLASS_INFO(CLASS_DRUID, "Druid", TCOD_orange, 8);
    CLASS_INFO(CLASS_FIGHTER, "Fighter", TCOD_brass, 10);
    CLASS_INFO(CLASS_MONK, "Monk", TCOD_sea, 8);
    CLASS_INFO(CLASS_PALADIN, "Paladin", TCOD_pink, 10);
    CLASS_INFO(CLASS_RANGER, "Ranger", TCOD_green, 10);
    CLASS_INFO(CLASS_ROGUE, "Rogue", TCOD_yellow, 6);
    CLASS_INFO(CLASS_SORCERER, "Sorcerer", TCOD_flame, 4);
    CLASS_INFO(CLASS_WIZARD, "Wizard", TCOD_azure, 4);

    CLASS_INFO(CLASS_ABBERATION, "Abberation", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_ANIMAL, "Animal", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_BEAST, "Beast", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_COMMONER, "Commonder", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_CONSTRUCT, "Construct", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_DRAGON, "Dragon", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_ELEMENTAL, "Elemental", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_FEY, "Fey", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_GIANT, "Giant", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_HUMANOID, "Humanoid", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_MAGICAL_BEAST, "Magical Beast", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_MONSTROUS, "Monstrous", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_OOZE, "Ooze", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_OUTSIDER, "Outsider", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_SHAPESHIFTER, "Shapeshifter", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_UNDEAD, "Undead", TCOD_lightest_grey, 10);
    CLASS_INFO(CLASS_VERMIN, "Vermin", TCOD_lightest_grey, 10);

    MONSTER_PROTOTYPE(MONSTER_BASILISK, "Basilisk", RACE_BEAST, CLASS_BEAST);
    MONSTER_PROTOTYPE(MONSTER_BEHOLDER, "Beholder", RACE_ABBERATION, CLASS_ABBERATION);
    MONSTER_PROTOTYPE(MONSTER_BUGBEAR, "Bugbear", RACE_GOBLINOID, CLASS_HUMANOID);
    MONSTER_PROTOTYPE(MONSTER_FIRE_ELEMENTAL, "Fire Elemental", RACE_ELEMENTAL, CLASS_ELEMENTAL);
    MONSTER_PROTOTYPE(MONSTER_GELATINOUS_CUBE, "Gelatinous Cube", RACE_OOZE, CLASS_OOZE);
    MONSTER_PROTOTYPE(MONSTER_GOBLIN, "Goblin", RACE_GOBLINOID, CLASS_HUMANOID);
    MONSTER_PROTOTYPE(MONSTER_IRON_GOLEM, "Iron Golem", RACE_CONSTRUCT, CLASS_CONSTRUCT);
    MONSTER_PROTOTYPE(MONSTER_OGRE, "Ogre", RACE_GIANT, CLASS_GIANT);
    MONSTER_PROTOTYPE(MONSTER_ORC_CLERIC, "Orc Cleric", RACE_ORC, CLASS_CLERIC);
    MONSTER_PROTOTYPE(MONSTER_ORC_FIGHTER, "Orc Fighter", RACE_ORC, CLASS_FIGHTER);
    MONSTER_PROTOTYPE(MONSTER_ORC_RANGER, "Orc Ranger", RACE_ORC, CLASS_RANGER);
    MONSTER_PROTOTYPE(MONSTER_PIXIE, "Pixie", RACE_FEY, CLASS_FEY);
    MONSTER_PROTOTYPE(MONSTER_RAKSHASA, "Rakshasa", RACE_OUTSIDER, CLASS_SHAPESHIFTER);
    MONSTER_PROTOTYPE(MONSTER_RAT, "Rat", RACE_VERMIN, CLASS_VERMIN);
    MONSTER_PROTOTYPE(MONSTER_RED_SLAAD, "Red Slaad", RACE_OUTSIDER, CLASS_OUTSIDER);
    MONSTER_PROTOTYPE(MONSTER_WOLF, "Wolf", RACE_ANIMAL, CLASS_ANIMAL);
    MONSTER_PROTOTYPE(MONSTER_WYRMLING, "Wyrmling", RACE_DRAGON, CLASS_DRAGON);
    MONSTER_PROTOTYPE(MONSTER_ZOMBIE, "Zombie", RACE_UNDEAD, CLASS_UNDEAD);

    ABILITY_INFO(ABILITY_STRENGTH, "Strength", "STR", "Measures a character's muscle, endurance, and stamina.");
    ABILITY_INFO(ABILITY_DEXTERITY, "Dexterity", "DEX", "Encompasses several physical attributes including hand-eye coordination, agility, reaction speed, reflexes, and balance.");
    ABILITY_INFO(ABILITY_CONSTITUTION, "Constitution", "CON", "Encompasses his physique, fitness, health, and physical resistance to hardship, injury, and disease.");
    ABILITY_INFO(ABILITY_INTELLIGENCE, "Intelligence", "INT", "Represents a character's memory, reasoning, and learning ability, including areas outside those measured by the written word.");
    ABILITY_INFO(ABILITY_WISDOM, "Wisdom", "WIS", "Describes a composite of the character's enlightenment, judgment, guile, willpower, common sense, and intuition.");
    ABILITY_INFO(ABILITY_CHARISMA, "Charisma", "CHA", "Measures a character's persuasiveness, personal magnetism, and ability to lead.");

    ITEM_COMMON(0);

    EQUIP_SLOT_INFO(EQUIP_SLOT_ARMOR, "Armor", "Armor ");
    EQUIP_SLOT_INFO(EQUIP_SLOT_BELT, "Belt", "Belt  ");
    EQUIP_SLOT_INFO(EQUIP_SLOT_BOOTS, "Boots", "Boots ");
    EQUIP_SLOT_INFO(EQUIP_SLOT_CLOAK, "Cloak", "Cloak ");
    EQUIP_SLOT_INFO(EQUIP_SLOT_GLOVES, "Gloves", "Gloves");
    EQUIP_SLOT_INFO(EQUIP_SLOT_HELMET, "Helmet", "Helmet");
    EQUIP_SLOT_INFO(EQUIP_SLOT_MAIN_HAND, "Main Hand", "M-Hand");
    EQUIP_SLOT_INFO(EQUIP_SLOT_OFF_HAND, "Off Hand", "O-Hand");

    BASE_ITEM_INFO(BASE_ITEM_BATTLEAXE, 'T', TCOD_white, 7, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_SLASHING, WEAPON_SIZE_MEDIUM, false, 1, 8, 20, 3, 10, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_CLUB, '!', TCOD_white, 1, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_BLUDGEONING, WEAPON_SIZE_MEDIUM, false, 1, 6, 20, 2, 3, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_DAGGER, '-', TCOD_white, 1, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_PIERCING, WEAPON_SIZE_TINY, false, 1, 4, 19, 2, 4, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_GREATAXE, 'T', TCOD_white, 20, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_SLASHING, WEAPON_SIZE_LARGE, false, 1, 12, 20, 3, 20, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_GREATSWORD, '|', TCOD_white, 15, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_SLASHING, WEAPON_SIZE_LARGE, false, 2, 6, 19, 2, 50, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_HALBERD, 'P', TCOD_white, 15, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_PIERCING, WEAPON_SIZE_LARGE, false, 1, 10, 20, 3, 10, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_HEAVY_CROSSBOW, 't', TCOD_white, 9, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_PIERCING, WEAPON_SIZE_MEDIUM, true, 1, 10, 19, 2, 50, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_KATANA, '(', TCOD_white, 10, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_SLASHING, WEAPON_SIZE_MEDIUM, false, 1, 10, 19, 2, 40, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_LARGE_SHIELD, ')', TCOD_white, 15, EQUIP_SLOT_OFF_HAND, 0, 0, false, 0, 0, 0, 0, 50, 1, 2, -2, 15);
    BASE_ITEM_INFO(BASE_ITEM_LIGHT_CROSSBOW, 't', TCOD_white, 6, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_PIERCING, WEAPON_SIZE_SMALL, true, 1, 8, 19, 2, 35, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_LONGBOW, '}', TCOD_white, 3, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_PIERCING, WEAPON_SIZE_LARGE, true, 1, 8, 20, 3, 75, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_LONGSWORD, '|', TCOD_white, 4, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_SLASHING, WEAPON_SIZE_MEDIUM, false, 1, 8, 19, 2, 15, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_MACE, '!', TCOD_white, 6, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_BLUDGEONING, WEAPON_SIZE_SMALL, false, 1, 6, 19, 2, 5, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_POTION, '!', TCOD_white, 0.5f, EQUIP_SLOT_NONE, 0, 0, false, 0, 0, 0, 0, 30, 10, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_QUARTERSTAFF, '/', TCOD_white, 4, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_BLUDGEONING, WEAPON_SIZE_LARGE, false, 1, 6, 20, 2, 1, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_SCIMITAR, '(', TCOD_white, 4, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_SLASHING, WEAPON_SIZE_MEDIUM, false, 1, 6, 18, 2, 15, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_SHORTBOW, '}', TCOD_white, 2, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_PIERCING, WEAPON_SIZE_MEDIUM, true, 1, 6, 20, 3, 30, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_SHORTSWORD, '-', TCOD_white, 3, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_PIERCING, WEAPON_SIZE_SMALL, false, 1, 6, 19, 2, 10, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_SMALL_SHIELD, ')', TCOD_white, 6, EQUIP_SLOT_OFF_HAND, 0, 0, false, 0, 0, 0, 0, 9, 1, 1, -1, 5);
    BASE_ITEM_INFO(BASE_ITEM_SPEAR, '/', TCOD_white, 3, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_PIERCING, WEAPON_SIZE_LARGE, false, 1, 8, 20, 3, 1, 1, 0, 0, 0);
    BASE_ITEM_INFO(BASE_ITEM_TOWER_SHIELD, ')', TCOD_white, 45, EQUIP_SLOT_OFF_HAND, 0, 0, false, 0, 0, 0, 0, 100, 1, 3, -10, 50);
    BASE_ITEM_INFO(BASE_ITEM_WARHAMMER, '!', TCOD_white, 8, EQUIP_SLOT_MAIN_HAND, DAMAGE_TYPE_BLUDGEONING, WEAPON_SIZE_MEDIUM, false, 1, 8, 20, 3, 12, 1, 0, 0, 0);

    ITEM_INFO(ITEM_TYPE_BATTLEAXE, BASE_ITEM_BATTLEAXE, "Battleaxe", "A generic battleaxe");
    ITEM_INFO(ITEM_TYPE_BATTLEAXE_1, BASE_ITEM_BATTLEAXE, "Battleaxe + 1", "A special battleaxe");
    TCOD_list_push(item_info[ITEM_TYPE_BATTLEAXE_1].item_properties, enhancement_bonus_create(1));
    ITEM_INFO(ITEM_TYPE_CLUB, BASE_ITEM_CLUB, "Club", "A generic club");
    ITEM_INFO(ITEM_TYPE_CLUB_1, BASE_ITEM_CLUB, "Club + 1", "A special club");
    TCOD_list_push(item_info[ITEM_TYPE_CLUB_1].item_properties, enhancement_bonus_create(1));
    ITEM_INFO(ITEM_TYPE_DAGGER, BASE_ITEM_DAGGER, "Dagger", "A generic dagger");
    ITEM_INFO(ITEM_TYPE_DAGGER_1, BASE_ITEM_DAGGER, "Dagger + 1", "A special Dagger");
    TCOD_list_push(item_info[ITEM_TYPE_DAGGER_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_GREATAXE].base_item = BASE_ITEM_GREATAXE;
    item_info[ITEM_TYPE_GREATAXE].name = "Greataxe";
    item_info[ITEM_TYPE_GREATAXE].description = "A generic greataxe.";
    item_info[ITEM_TYPE_GREATAXE].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_GREATAXE_1].base_item = BASE_ITEM_GREATAXE;
    item_info[ITEM_TYPE_GREATAXE_1].name = "Greataxe + 1";
    item_info[ITEM_TYPE_GREATAXE_1].description = "A special greataxe.";
    item_info[ITEM_TYPE_GREATAXE_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_GREATAXE_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_GREATSWORD].base_item = BASE_ITEM_GREATSWORD;
    item_info[ITEM_TYPE_GREATSWORD].name = "Greatsword";
    item_info[ITEM_TYPE_GREATSWORD].description = "A generic greatsword.";
    item_info[ITEM_TYPE_GREATSWORD].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_GREATSWORD_1].base_item = BASE_ITEM_GREATSWORD;
    item_info[ITEM_TYPE_GREATSWORD_1].name = "Greatsword + 1";
    item_info[ITEM_TYPE_GREATSWORD_1].description = "A special greatsword.";
    item_info[ITEM_TYPE_GREATSWORD_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_GREATSWORD_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_HALBERD].base_item = BASE_ITEM_HALBERD;
    item_info[ITEM_TYPE_HALBERD].name = "Halberd";
    item_info[ITEM_TYPE_HALBERD].description = "A generic halberd.";
    item_info[ITEM_TYPE_HALBERD].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_HALBERD_1].base_item = BASE_ITEM_HALBERD;
    item_info[ITEM_TYPE_HALBERD_1].name = "Halberd + 1";
    item_info[ITEM_TYPE_HALBERD_1].description = "A special halberd.";
    item_info[ITEM_TYPE_HALBERD_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_HALBERD_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_HEAVY_CROSSBOW].base_item = BASE_ITEM_HEAVY_CROSSBOW;
    item_info[ITEM_TYPE_HEAVY_CROSSBOW].name = "Heavy Crossbow";
    item_info[ITEM_TYPE_HEAVY_CROSSBOW].description = "A generic heavy crossbow.";
    item_info[ITEM_TYPE_HEAVY_CROSSBOW].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_HEAVY_CROSSBOW_1].base_item = BASE_ITEM_HEAVY_CROSSBOW;
    item_info[ITEM_TYPE_HEAVY_CROSSBOW_1].name = "Heavy Crossbow + 1";
    item_info[ITEM_TYPE_HEAVY_CROSSBOW_1].description = "A special heavy crossbow.";
    item_info[ITEM_TYPE_HEAVY_CROSSBOW_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_HEAVY_CROSSBOW_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_KATANA].base_item = BASE_ITEM_KATANA;
    item_info[ITEM_TYPE_KATANA].name = "Katana";
    item_info[ITEM_TYPE_KATANA].description = "A generic katana.";
    item_info[ITEM_TYPE_KATANA].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_KATANA_1].base_item = BASE_ITEM_KATANA;
    item_info[ITEM_TYPE_KATANA_1].name = "Katana + 1";
    item_info[ITEM_TYPE_KATANA_1].description = "A special katana.";
    item_info[ITEM_TYPE_KATANA_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_KATANA_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_LARGE_SHIELD].base_item = BASE_ITEM_LARGE_SHIELD;
    item_info[ITEM_TYPE_LARGE_SHIELD].name = "Large Shield";
    item_info[ITEM_TYPE_LARGE_SHIELD].description = "A generic large shield.";
    item_info[ITEM_TYPE_LARGE_SHIELD].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_LARGE_SHIELD_1].base_item = BASE_ITEM_LARGE_SHIELD;
    item_info[ITEM_TYPE_LARGE_SHIELD_1].name = "Large Shield + 1";
    item_info[ITEM_TYPE_LARGE_SHIELD_1].description = "A special large shield.";
    item_info[ITEM_TYPE_LARGE_SHIELD_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_LARGE_SHIELD_1].item_properties, ac_bonus_create(AC_SHIELD, 1));
    item_info[ITEM_TYPE_LIGHT_CROSSBOW].base_item = BASE_ITEM_LIGHT_CROSSBOW;
    item_info[ITEM_TYPE_LIGHT_CROSSBOW].name = "Light Crossbow";
    item_info[ITEM_TYPE_LIGHT_CROSSBOW].description = "A generic light crossbow.";
    item_info[ITEM_TYPE_LIGHT_CROSSBOW].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_LIGHT_CROSSBOW_1].base_item = BASE_ITEM_LIGHT_CROSSBOW;
    item_info[ITEM_TYPE_LIGHT_CROSSBOW_1].name = "Light Crossbow + 1";
    item_info[ITEM_TYPE_LIGHT_CROSSBOW_1].description = "A special light crossbow.";
    item_info[ITEM_TYPE_LIGHT_CROSSBOW_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_LIGHT_CROSSBOW_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_LONGBOW].base_item = BASE_ITEM_LONGBOW;
    item_info[ITEM_TYPE_LONGBOW].name = "Longbow";
    item_info[ITEM_TYPE_LONGBOW].description = "A generic longbow.";
    item_info[ITEM_TYPE_LONGBOW].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_LONGBOW_1].base_item = BASE_ITEM_LONGBOW;
    item_info[ITEM_TYPE_LONGBOW_1].name = "Longbow + 1";
    item_info[ITEM_TYPE_LONGBOW_1].description = "A special longbow.";
    item_info[ITEM_TYPE_LONGBOW_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_LONGBOW_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_LONGSWORD].base_item = BASE_ITEM_LONGSWORD;
    item_info[ITEM_TYPE_LONGSWORD].name = "Longsword";
    item_info[ITEM_TYPE_LONGSWORD].description = "A generic longsword.";
    item_info[ITEM_TYPE_LONGSWORD].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_LONGSWORD_1].base_item = BASE_ITEM_LONGSWORD;
    item_info[ITEM_TYPE_LONGSWORD_1].name = "Longsword + 1";
    item_info[ITEM_TYPE_LONGSWORD_1].description = "A special longsword.";
    item_info[ITEM_TYPE_LONGSWORD_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_LONGSWORD_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_MACE].base_item = BASE_ITEM_MACE;
    item_info[ITEM_TYPE_MACE].name = "Mace";
    item_info[ITEM_TYPE_MACE].description = "A generic mace.";
    item_info[ITEM_TYPE_MACE].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_MACE_1].base_item = BASE_ITEM_MACE;
    item_info[ITEM_TYPE_MACE_1].name = "Mace + 1";
    item_info[ITEM_TYPE_MACE_1].description = "A special mace.";
    item_info[ITEM_TYPE_MACE_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_MACE_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS].base_item = BASE_ITEM_POTION;
    item_info[ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS].name = "Potion of Cure Light Wounds";
    item_info[ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS].description = "A healing potion.";
    item_info[ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_QUARTERSTAFF].base_item = BASE_ITEM_QUARTERSTAFF;
    item_info[ITEM_TYPE_QUARTERSTAFF].name = "Quarterstaff";
    item_info[ITEM_TYPE_QUARTERSTAFF].description = "A generic quarterstaff.";
    item_info[ITEM_TYPE_QUARTERSTAFF].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_QUARTERSTAFF_1].base_item = BASE_ITEM_QUARTERSTAFF;
    item_info[ITEM_TYPE_QUARTERSTAFF_1].name = "Quarterstaff + 1";
    item_info[ITEM_TYPE_QUARTERSTAFF_1].description = "A special quarterstaff.";
    item_info[ITEM_TYPE_QUARTERSTAFF_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_QUARTERSTAFF_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_SCIMITAR].base_item = BASE_ITEM_SCIMITAR;
    item_info[ITEM_TYPE_SCIMITAR].name = "Scimitar";
    item_info[ITEM_TYPE_SCIMITAR].description = "A generic scimitar.";
    item_info[ITEM_TYPE_SCIMITAR].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_SCIMITAR_1].base_item = BASE_ITEM_SCIMITAR;
    item_info[ITEM_TYPE_SCIMITAR_1].name = "Scimitar + 1";
    item_info[ITEM_TYPE_SCIMITAR_1].description = "A special scimitar.";
    item_info[ITEM_TYPE_SCIMITAR_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_SCIMITAR_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_SHORTBOW].base_item = BASE_ITEM_SHORTBOW;
    item_info[ITEM_TYPE_SHORTBOW].name = "Shortbow";
    item_info[ITEM_TYPE_SHORTBOW].description = "A generic shortbow.";
    item_info[ITEM_TYPE_SHORTBOW].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_SHORTBOW_1].base_item = BASE_ITEM_SHORTBOW;
    item_info[ITEM_TYPE_SHORTBOW_1].name = "Shortbow + 1";
    item_info[ITEM_TYPE_SHORTBOW_1].description = "A special shortbow.";
    item_info[ITEM_TYPE_SHORTBOW_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_SHORTBOW_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_SHORTSWORD].base_item = BASE_ITEM_SHORTSWORD;
    item_info[ITEM_TYPE_SHORTSWORD].name = "Shortsword";
    item_info[ITEM_TYPE_SHORTSWORD].description = "A generic shortsword.";
    item_info[ITEM_TYPE_SHORTSWORD].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_SHORTSWORD_1].base_item = BASE_ITEM_SHORTSWORD;
    item_info[ITEM_TYPE_SHORTSWORD_1].name = "Shortsword + 1";
    item_info[ITEM_TYPE_SHORTSWORD_1].description = "A special shortsword.";
    item_info[ITEM_TYPE_SHORTSWORD_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_SHORTSWORD_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_SMALL_SHIELD].base_item = BASE_ITEM_SMALL_SHIELD;
    item_info[ITEM_TYPE_SMALL_SHIELD].name = "Small Shield";
    item_info[ITEM_TYPE_SMALL_SHIELD].description = "A generic small shield.";
    item_info[ITEM_TYPE_SMALL_SHIELD].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_SMALL_SHIELD_1].base_item = BASE_ITEM_SMALL_SHIELD;
    item_info[ITEM_TYPE_SMALL_SHIELD_1].name = "Small Shield + 1";
    item_info[ITEM_TYPE_SMALL_SHIELD_1].description = "A special small shield.";
    item_info[ITEM_TYPE_SMALL_SHIELD_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_SMALL_SHIELD_1].item_properties, ac_bonus_create(AC_SHIELD, 1));
    item_info[ITEM_TYPE_SPEAR].base_item = BASE_ITEM_SPEAR;
    item_info[ITEM_TYPE_SPEAR].name = "Spear";
    item_info[ITEM_TYPE_SPEAR].description = "A generic spear.";
    item_info[ITEM_TYPE_SPEAR].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_SPEAR_1].base_item = BASE_ITEM_SPEAR;
    item_info[ITEM_TYPE_SPEAR_1].name = "Spear + 1";
    item_info[ITEM_TYPE_SPEAR_1].description = "A special spear.";
    item_info[ITEM_TYPE_SPEAR_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_SPEAR_1].item_properties, enhancement_bonus_create(1));
    item_info[ITEM_TYPE_TOWER_SHIELD].base_item = BASE_ITEM_TOWER_SHIELD;
    item_info[ITEM_TYPE_TOWER_SHIELD].name = "Tower Shield";
    item_info[ITEM_TYPE_TOWER_SHIELD].description = "A generic tower shield.";
    item_info[ITEM_TYPE_TOWER_SHIELD].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_TOWER_SHIELD_1].base_item = BASE_ITEM_TOWER_SHIELD;
    item_info[ITEM_TYPE_TOWER_SHIELD_1].name = "Tower Shield + 1";
    item_info[ITEM_TYPE_TOWER_SHIELD_1].description = "A special tower shield.";
    item_info[ITEM_TYPE_TOWER_SHIELD_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_TOWER_SHIELD_1].item_properties, ac_bonus_create(AC_SHIELD, 1));
    item_info[ITEM_TYPE_WARHAMMER].base_item = BASE_ITEM_WARHAMMER;
    item_info[ITEM_TYPE_WARHAMMER].name = "Warhammer";
    item_info[ITEM_TYPE_WARHAMMER].description = "A generic warhammer.";
    item_info[ITEM_TYPE_WARHAMMER].item_properties = TCOD_list_new();
    item_info[ITEM_TYPE_WARHAMMER_1].base_item = BASE_ITEM_WARHAMMER;
    item_info[ITEM_TYPE_WARHAMMER_1].name = "Warhammer + 1";
    item_info[ITEM_TYPE_WARHAMMER_1].description = "A special warhammer.";
    item_info[ITEM_TYPE_WARHAMMER_1].item_properties = TCOD_list_new();
    TCOD_list_push(item_info[ITEM_TYPE_WARHAMMER_1].item_properties, enhancement_bonus_create(1));
}

void assets_unload(void)
{
    for (enum item_type item_type = 0; item_type < NUM_ITEM_TYPES; item_type++)
    {
        TCOD_list_t item_properties = item_info[item_type].item_properties;

        TCOD_LIST_FOREACH(item_properties)
        {
            struct base_item_property *base_item_property = *iterator;

            switch (base_item_property->item_property)
            {
            case ITEM_PROPERTY_AC_BONUS:
            {
                struct ac_bonus *ac_bonus = (struct ac_bonus *)base_item_property;

                ac_bonus_destroy(ac_bonus);
            }
            break;
            case ITEM_PROPERTY_ENHANCEMENT_BONUS:
            {
                struct enhancement_bonus *enhancement_bonus = (struct enhancement_bonus *)base_item_property;

                enhancement_bonus_destroy(enhancement_bonus);
            }
            break;
            }
        }

        TCOD_list_delete(item_properties);
    }
}
