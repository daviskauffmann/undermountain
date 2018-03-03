#ifndef SPELL_H
#define SPELL_H

#include <libtcod/libtcod.h>

typedef enum spell_type_e spell_type_t;
typedef struct spell_s spell_t;

enum spell_type_e
{
    SPELL_HEAL_SELF,
    SPELL_INSTAKILL,

    NUM_SPELL_TYPES
};

struct spell_s
{
    spell_type_t type;
    bool known;
};

#endif
