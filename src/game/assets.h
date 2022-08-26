#ifndef ASSETS_H
#define ASSETS_H

#include "actor.h"
#include "corpse.h"
#include "item.h"
#include "light.h"
#include "object.h"
#include "projectile.h"
#include "spell.h"
#include "tile.h"

extern struct tile_common tile_common;
extern struct tile_datum tile_data[NUM_TILE_TYPES];
extern struct object_common object_common;
extern struct object_datum object_data[NUM_OBJECT_TYPES];
extern struct light_datum light_data[NUM_LIGHT_TYPES];
extern struct actor_common actor_common;
extern struct size_datum size_data[NUM_SIZES];
extern struct race_datum race_data[NUM_RACES];
extern struct class_datum class_data[NUM_CLASSES];
extern struct actor_prototype monster_prototypes[NUM_MONSTERS];
extern struct ability_datum ability_data[NUM_ABILITIES];
extern struct corpse_common corpse_common;
extern struct item_common item_common;
extern struct equip_slot_datum equip_slot_data[NUM_EQUIP_SLOTS];
extern struct base_item_datum base_item_data[NUM_BASE_ITEM_TYPES];
extern struct item_datum item_data[NUM_ITEM_TYPES];
extern struct spell_datum spell_data[NUM_SPELL_TYPES];
extern struct projectile_datum projectile_data[NUM_PROJECTILE_TYPES];

void assets_load(void);

#endif
