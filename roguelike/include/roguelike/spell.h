#ifndef ROGUELIKE_SPELL_H
#define ROGUELIKE_SPELL_H

// ideas:
// spell types: arcane or divine
// innate level
// levels for each class
// school of magic
// spells must also be able to be stored in potions and scrolls
//     these spells should contain information about what caster level to use
// spell target type: self or target

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

#endif
