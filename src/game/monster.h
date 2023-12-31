#ifndef UM_GAME_MONSTER_H
#define UM_GAME_MONSTER_H

#include "ability.h"
#include "feat.h"
#include "item.h"
#include "special_ability.h"
#include "special_attack.h"

// TODO: maybe just merge all/some of this into the class data?

enum monster_type
{
    MONSTER_TYPE_BAT,
    MONSTER_TYPE_BUGBEAR,
    MONSTER_TYPE_CAT,
    MONSTER_TYPE_DIRE_RAT,
    MONSTER_TYPE_DOG,
    MONSTER_TYPE_FIRE_BEETLE,
    MONSTER_TYPE_HYENA,
    MONSTER_TYPE_GIANT_ANT,
    MONSTER_TYPE_GNOLL,
    MONSTER_TYPE_GOBLIN,
    MONSTER_TYPE_HOBGOBLIN,
    MONSTER_TYPE_KOBOLD,
    MONSTER_TYPE_KRENSHAR,
    MONSTER_TYPE_OGRE,
    MONSTER_TYPE_ORC,
    MONSTER_TYPE_OWL,
    MONSTER_TYPE_PONY,
    MONSTER_TYPE_RAKSHASA,
    MONSTER_TYPE_RAT,
    MONSTER_TYPE_RED_DRAGON_WYRMLING,
    MONSTER_TYPE_RED_DRAGON_ADULT,
    MONSTER_TYPE_RED_DRAGON_ANCIENT,
    MONSTER_TYPE_SKELETON_WARRIOR,
    MONSTER_TYPE_SNAKE,
    MONSTER_TYPE_SPIDER,
    MONSTER_TYPE_TROLL,
    MONSTER_TYPE_WOLF,
    MONSTER_TYPE_ZOMBIE,

    NUM_MONSTER_TYPES,
};

struct monster_data
{
    const char *name;

    enum race race;
    enum class class;
    enum size size;
    enum faction faction;

    int level;

    int ability_scores[NUM_ABILITIES];

    bool special_abilities[NUM_SPECIAL_ABILITIES];

    bool special_attacks[NUM_SPECIAL_ATTACKS];

    bool feats[NUM_FEATS];

    struct
    {
        enum item_type type;

        int min_stack;
        int max_stack;
    } equipment[NUM_EQUIP_SLOTS];

    struct
    {
        int min_stack;
        int max_stack;
    } items[NUM_ITEM_TYPES];

    // TODO: spells
};

extern const struct monster_data monster_database[NUM_MONSTER_TYPES];

#endif
