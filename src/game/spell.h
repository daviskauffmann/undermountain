#ifndef GAME_SPELL_H
#define GAME_SPELL_H

// casting:
// open spellbook
// select spell and press a to activate it
// press z to cast
// if non-targeted spell, cast immediately
//     this could be self-buffing spells or aoe spells
// if targeted spell, enter targeting mode
// press z again to cast spell, calling spell_cast() with the coordinates of the cursor
// activating a spell should be a input module thing, not an actor thing
//     it doesn't make much sense for an npc to activate a spell, their AI would just cast it

enum spell_type
{
    SPELL_TYPE_HEAL,
    SPELL_TYPE_LIGHTNING,

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
};

#endif
