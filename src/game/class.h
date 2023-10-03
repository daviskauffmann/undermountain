#ifndef UM_GAME_CLASS_H
#define UM_GAME_CLASS_H

#include "ability.h"
#include "feat.h"
#include "item.h"
#include "spell.h"
#include <libtcod.h>

enum class
{
    CLASS_NONE,

    // player classes
    CLASS_CLERIC,
    CLASS_FIGHTER,
    CLASS_PALADIN,
    CLASS_ROGUE,
    CLASS_WIZARD,

    // monster classes
    CLASS_BAT,
    CLASS_BUGBEAR,
    CLASS_CAT,
    CLASS_DIRE_RAT,
    CLASS_DOG,
    CLASS_FIRE_BEETLE,
    CLASS_HYENA,
    CLASS_GIANT_ANT,
    CLASS_GNOLL,
    CLASS_GOBLIN,
    CLASS_HOBGOBLIN,
    CLASS_KOBOLD,
    CLASS_KRENSHAR,
    CLASS_OGRE,
    CLASS_ORC,
    CLASS_OWL,
    CLASS_PONY,
    CLASS_RAKSHASA,
    CLASS_RAT,
    CLASS_RED_DRAGON_WYRMLING,
    CLASS_RED_DRAGON_ADULT,
    CLASS_RED_DRAGON_ANCIENT,
    CLASS_SKELETON_WARRIOR,
    CLASS_SNAKE,
    CLASS_SPIDER,
    CLASS_TROLL,
    CLASS_WOLF,

    NUM_CLASSES,
};
#define PLAYER_CLASS_BEGIN CLASS_CLERIC
#define PLAYER_CLASS_END CLASS_WIZARD
#define NUM_PLAYER_CLASSES PLAYER_CLASS_END + 1
#define MONSTER_CLASS_BEGIN CLASS_BAT
#define MONSTER_CLASS_END CLASS_WOLF
#define NUM_MONSTER_CLASSES MONSTER_CLASS_END + 1

struct class_data
{
    const char *name;
    TCOD_ColorRGB color;
    unsigned char glyph;

    const char *hit_die;

    int natural_armor_bonus;

    enum base_attack_bonus_type base_attack_bonus_type;

    enum natural_weapon_type natural_weapon_type;

    int default_ability_scores[NUM_ABILITIES];

    int feat_progression[NUM_FEATS];

    enum ability spellcasting_ability;
    int spell_progression[NUM_SPELL_TYPES];

    enum item_type starting_equipment[NUM_EQUIP_SLOTS];

    int starting_items[NUM_ITEM_TYPES];

    enum monster_type companion;
};

extern const struct class_data class_database[NUM_CLASSES];

#endif
