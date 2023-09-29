#ifndef UM_GAME_EQUIP_SLOT_H
#define UM_GAME_EQUIP_SLOT_H

enum equip_slot
{
    EQUIP_SLOT_NONE,

    EQUIP_SLOT_AMMUNITION,
    EQUIP_SLOT_ARMOR,
    EQUIP_SLOT_SHIELD,
    EQUIP_SLOT_WEAPON,

    NUM_EQUIP_SLOTS
};

struct equip_slot_data
{
    char *name;
};

extern const struct equip_slot_data equip_slot_database[NUM_EQUIP_SLOTS];

#endif
