#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include "actor.h"
#include "item.h"
#include "object.h"
#include "spell.h"
#include "tile.h"

extern struct tile_common tile_common;
extern struct tile_data tile_datum[NUM_TILE_TYPES];
extern struct object_common object_common;
extern struct object_data object_datum[NUM_OBJECT_TYPES];
extern struct actor_common actor_common;
extern struct race_data race_datum[NUM_RACES];
extern struct class_data class_datum[NUM_CLASSES];
extern struct actor_prototype monster_prototypes[NUM_MONSTERS];
extern struct item_common item_common;
extern struct equip_slot_data equip_slot_datum[NUM_EQUIP_SLOTS];
extern struct item_data item_datum[NUM_ITEM_TYPES];
extern struct spell_data spell_datum[NUM_SPELL_TYPES];

void assets_load(void);
void assets_unload(void);

#endif
