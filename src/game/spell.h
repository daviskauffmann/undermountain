#ifndef UM_GAME_SPELL_H
#define UM_GAME_SPELL_H

#include "spell_range.h"

enum spell_type
{
    SPELL_TYPE_NONE, // TODO: maybe remove this?

    SPELL_TYPE_ACID_SPLASH,
    SPELL_TYPE_MINOR_HEAL,
    SPELL_TYPE_MINOR_MANA,
    SPELL_TYPE_LIGHTNING,
    SPELL_TYPE_FIREBALL,
    SPELL_TYPE_SUMMON_FAMILIAR,

    NUM_SPELL_TYPES
};

struct spell_data
{
    const char *name;
    enum spell_range range;
    int level;
};

extern const struct spell_data spell_database[NUM_SPELL_TYPES];

#endif
