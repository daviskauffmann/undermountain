#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include "actor.h"
#include "item.h"
#include "object.h"
#include "spell.h"
#include "tile.h"

extern struct tile_common tile_common;
extern struct tile_datum tile_data[NUM_TILE_TYPES];
extern struct object_common object_common;
extern struct object_datum object_data[NUM_OBJECT_TYPES];
extern struct actor_common actor_common;
extern struct race_datum race_data[NUM_RACES];
extern struct class_datum class_data[NUM_CLASSES];
extern struct actor_prototype monster_prototypes[NUM_MONSTERS];
extern struct item_common item_common;
extern struct equip_slot_datum equip_slot_data[NUM_EQUIP_SLOTS];
extern struct item_datum item_data[NUM_ITEM_TYPES];
extern struct spell_datum spell_data[NUM_SPELL_TYPES];

void assets_load(void);

#endif
