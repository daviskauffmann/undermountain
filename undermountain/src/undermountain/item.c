#include <undermountain/undermountain.h>

struct item *item_create(enum item_type type, int floor, int x, int y)
{
    struct item *item = malloc(sizeof(struct item));

    if (!item)
    {
        printf("Couldn't allocate item\n");

        return NULL;
    }

    item->type = type;
    item->floor = floor;
    item->x = x;
    item->y = y;

    return item;
}

bool item_is_two_handed(struct item *item, enum race_size race_size)
{
    enum base_item base_item = item_info[item->type].base_item;

    enum equip_slot equip_slot = base_item_info[base_item].equip_slot;

    if (equip_slot != EQUIP_SLOT_MAIN_HAND)
    {
        return false;
    }

    enum weapon_size weapon_size = base_item_info[base_item].weapon_size;

    switch (race_size)
    {
    case RACE_SIZE_MEDIUM:
    {
        switch (weapon_size)
        {
        case WEAPON_SIZE_TINY:
        case WEAPON_SIZE_SMALL:
        case WEAPON_SIZE_MEDIUM:
            return false;
        case WEAPON_SIZE_LARGE:
            return true;
        }
    }
    break;
    case RACE_SIZE_SMALL:
    {
        switch (weapon_size)
        {
        case WEAPON_SIZE_TINY:
        case WEAPON_SIZE_SMALL:
            return false;
        case WEAPON_SIZE_MEDIUM:
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
