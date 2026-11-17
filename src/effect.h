#ifndef UM_GAME_EFFECT_H
#define UM_GAME_EFFECT_H

enum effect_type
{
    EFFECT_TYPE_POISON,

    NUM_EFFECT_TYPES,
};

struct effect_data
{
    int duration;
    int value;
};

struct effect
{
    enum effect_type type;
    int value;
};

extern const struct effect_data effect_database[NUM_EFFECT_TYPES];

struct effect *effect_create(enum effect_type type, int value);
void effect_destroy(struct effect *effect);

#endif
