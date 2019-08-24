#ifndef GAME_ITEM_PROPERTY_H
#define GAME_ITEM_PROPERTY_H

enum item_property
{
    ITEM_PROPERTY_AC_BONUS,
    ITEM_PROPERTY_ENHANCEMENT_BONUS,

    NUM_ITEM_PROPERTIES
};

// TODO: where to store names?
// are they set when the item property is created, or calculated on the fly by the systems that use them?
struct base_item_property
{
    enum item_property item_property;
};

enum ac
{
    AC_ARMOR,
    AC_DEFLECTION,
    AC_DODGE,
    AC_NATURAL,
    AC_SHIELD
};

struct ac_bonus
{
    struct base_item_property base_item_property;
    enum ac ac;
    int bonus;
};

struct ac_bonus *ac_bonus_new(enum ac ac, int bonus);
void ac_bonus_delete(struct ac_bonus *ac_bonus);

struct enhancement_bonus
{
    struct base_item_property base_item_property;
    int bonus;
};

struct enhancement_bonus *enhancement_bonus_new(int bonus);
void enhancement_bonus_delete(struct enhancement_bonus *enhancement_bonus);

#endif
