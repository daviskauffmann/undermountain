#include <roguelike/roguelike.h>

struct item *item_create(enum item_type type, int floor, int x, int y)
{
    struct item *item = malloc(sizeof(struct item));

    item->type = type;
    item->floor = floor;
    item->x = x;
    item->y = y;

    return item;
}

bool item_can_equip(struct item *item, struct actor *actor)
{
    if (base_item_info[item_info[item->type].base_item].weapon_size == WEAPON_SIZE_LARGE && race_info[actor->race].size == RACE_SIZE_SMALL)
    {
        return false;
    }

    return true;
}

bool item_is_two_handed(struct item *item, struct actor *actor)
{
    enum equip_slot equip_slot = base_item_info[item_info[item->type].base_item].equip_slot;

    if (equip_slot != EQUIP_SLOT_MAIN_HAND)
    {
        return false;
    }

    enum race_size race_size = race_info[actor->race].size;
    enum weapon_size weapon_size = base_item_info[item_info[item->type].base_item].weapon_size;

    switch (race_size)
    {
    case RACE_SIZE_MEDIUM:
    {
        if (weapon_size == WEAPON_SIZE_TINY || weapon_size == WEAPON_SIZE_SMALL || weapon_size == WEAPON_SIZE_MEDIUM)
        {
            return false;
        }
        else if (weapon_size == WEAPON_SIZE_LARGE)
        {
            return true;
        }
    }
    break;
    case RACE_SIZE_SMALL:
    {
        if (weapon_size == WEAPON_SIZE_TINY || weapon_size == WEAPON_SIZE_SMALL)
        {
            return false;
        }
        else if (weapon_size == WEAPON_SIZE_MEDIUM)
        {
            return true;
        }
    }
    break;
    }

    return false;
}

void item_destroy(struct item *item)
{
    free(item);
}
