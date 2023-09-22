#ifndef UM_GAME_SPELL_H
#define UM_GAME_SPELL_H

enum spell_type
{
    SPELL_TYPE_NONE,

    SPELL_TYPE_MINOR_HEAL,
    SPELL_TYPE_MINOR_MANA,
    SPELL_TYPE_LIGHTNING,
    SPELL_TYPE_FIREBALL,

    NUM_SPELL_TYPES
};

enum spell_range
{
    SPELL_RANGE_PERSONAL,
    SPELL_RANGE_TOUCH
};

struct spell_data
{
    const char *name;
    enum spell_range range;
    int mana_cost;
};

#endif
