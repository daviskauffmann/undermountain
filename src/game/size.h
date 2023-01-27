#ifndef GAME_SIZE_H
#define GAME_SIZE_H

enum size
{
    SIZE_TINY,
    SIZE_SMALL,
    SIZE_MEDIUM,
    SIZE_LARGE,
    SIZE_HUGE,

    NUM_SIZES
};

struct size_data
{
    const char *name;

    int modifier;
};

#endif
