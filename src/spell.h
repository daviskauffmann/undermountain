#ifndef SPELL_H
#define SPELL_H

enum spell_type
{
    SPELL_HEAL_SELF,
    SPELL_INSTAKILL,

    NUM_SPELL_TYPES
};

struct spell_info
{
    const char *name;
};

struct spell
{
    enum spell_type type;
};

struct spell *spell_create(enum spell_type *type);
void spell_destroy(struct spell *spell);

#endif
