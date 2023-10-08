#ifndef UM_GAME_SAVING_THROW_H
#define UM_GAME_SAVING_THROW_H

enum saving_throw
{
    SAVING_THROW_NONE,

    SAVING_THROW_FORTITUDE,
    SAVING_THROW_REFLEX,
    SAVING_THROW_WILL,

    NUM_SAVING_THROWS
};

struct saving_throw_data
{
    const char *name;

    enum ability ability;
};

extern const struct saving_throw_data saving_throw_database[NUM_SAVING_THROWS];

#endif
