#ifndef ROGUELIKE_ITEM_PROPERTY_H
#define ROGUELIKE_ITEM_PROPERTY_H

struct ac_bonus
{
    struct base_item_property base_item_property;
    enum ac ac;
    int bonus;
};

struct ac_bonus *ac_bonus_create(enum ac ac, int bonus);
void ac_bonus_destroy(struct ac_bonus *ac_bonus);

struct enhancement_bonus
{
    struct base_item_property base_item_property;
    int bonus;
};

struct enhancement_bonus *enhancement_bonus_create(int bonus);
void enhancement_bonus_destroy(struct enhancement_bonus *enhancement_bonus);

#endif
