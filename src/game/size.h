#ifndef GAME_SIZE_H
#define GAME_SIZE_H

enum size
{
    SIZE_FINE,
    SIZE_DIMINUTIVE,
    SIZE_TINY,
    SIZE_SMALL,
    SIZE_MEDIUM,
    SIZE_LARGE,
    SIZE_HUGE,
    SIZE_GARGANTUAN,
    SIZE_COLOSSAL,

    NUM_SIZES
};

struct size_data
{
    const char *name;

    int modifier;
    float speed;
};

extern const struct size_data size_database[NUM_SIZES];

#endif
