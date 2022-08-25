#ifndef SPELL_H
#define SPELL_H

enum spell_type
{
    SPELL_TYPE_NONE,

    SPELL_TYPE_MINOR_HEAL,
    SPELL_TYPE_LIGHTNING,
    SPELL_TYPE_FIREBALL,

    NUM_SPELL_TYPES
};

enum spell_range
{
    SPELL_RANGE_SELF,
    SPELL_RANGE_TARGET
};

struct spell_datum
{
    const char *name;
    enum spell_range range;
    int mana_cost;
};

#endif
