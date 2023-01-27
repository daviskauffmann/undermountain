#ifndef SCENES_GAME_TOOLTIP_DATA_H
#define SCENES_GAME_TOOLTIP_DATA_H

#include "../../game/actor.h"

struct object;
struct item;

struct tooltip_data
{
    int x;
    int y;
    struct object *object;
    struct item *item;
    struct actor *actor;
    enum equip_slot equip_slot;
    enum spell_type spell_type;
    enum ability ability;
};

#endif
