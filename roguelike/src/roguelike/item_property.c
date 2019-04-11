#include <roguelike/roguelike.h>

struct ac_bonus *ac_bonus_create(enum ac ac, int bonus)
{
    struct ac_bonus *ac_bonus = malloc(sizeof(struct ac_bonus));

    if (!ac_bonus)
    {
        printf("Couldn't allocate ac_bonus\n");

        return NULL;
    }

    struct base_item_property *base_item_property = (struct base_item_property *)ac_bonus;

    base_item_property->item_property = ITEM_PROPERTY_AC_BONUS;
    ac_bonus->ac = ac;
    ac_bonus->bonus = bonus;

    return ac_bonus;
}

void ac_bonus_destroy(struct ac_bonus *ac_bonus)
{
    free(ac_bonus);
}

struct enhancement_bonus *enhancement_bonus_create(int bonus)
{
    struct enhancement_bonus *enhancement_bonus = malloc(sizeof(struct enhancement_bonus));

    if (!enhancement_bonus)
    {
        printf("Couldn't allocate enhancement_bonus\n");

        return NULL;
    }

    struct base_item_property *base_item_property = (struct base_item_property *)enhancement_bonus;

    base_item_property->item_property = ITEM_PROPERTY_ENHANCEMENT_BONUS;
    enhancement_bonus->bonus = bonus;

    return enhancement_bonus;
}

void enhancement_bonus_destroy(struct enhancement_bonus *enhancement_bonus)
{
    free(enhancement_bonus);
}
