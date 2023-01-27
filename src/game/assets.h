#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include "actor.h"
#include "corpse.h"
#include "item.h"
#include "light.h"
#include "object.h"
#include "projectile.h"
#include "size.h"
#include "spell.h"
#include "tile.h"

extern struct tile_common tile_common;
extern struct tile_data tile_database[NUM_TILE_TYPES];
extern struct light_data light_database[NUM_LIGHT_TYPES];
extern struct object_data object_database[NUM_OBJECT_TYPES];
extern struct actor_common actor_common;
extern struct size_data size_database[NUM_SIZES];
extern struct race_data race_database[NUM_RACES];
extern struct class_data class_database[NUM_CLASSES];
extern struct actor_prototype monster_prototypes[NUM_MONSTERS];
extern struct ability_data ability_database[NUM_ABILITIES];
extern struct corpse_common corpse_common;
extern struct equip_slot_data equip_slot_database[NUM_EQUIP_SLOTS];
extern struct base_item_data base_item_database[NUM_BASE_ITEM_TYPES];
extern struct item_data item_database[NUM_ITEM_TYPES];
extern struct spell_data spell_database[NUM_SPELL_TYPES];
extern struct projectile_data projectile_database[NUM_PROJECTILE_TYPES];

void assets_load(void);

#endif
