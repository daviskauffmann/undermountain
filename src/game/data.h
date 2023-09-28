#ifndef UM_GAME_DATA_H
#define UM_GAME_DATA_H

#include "actor.h"
#include "corpse.h"
#include "item.h"
#include "light.h"
#include "map.h"
#include "object.h"
#include "projectile.h"
#include "size.h"
#include "spell.h"
#include "surface.h"
#include "tile.h"

extern const struct tile_metadata tile_metadata;
extern const struct tile_data tile_database[NUM_TILE_TYPES];
extern const struct light_data light_database[NUM_LIGHT_TYPES];
extern const struct object_data object_database[NUM_OBJECT_TYPES];
extern const struct monster_pack_data monster_pack_database[NUM_MONSTER_PACKS];
extern const struct actor_metadata actor_metadata;
extern const struct size_data size_database[NUM_SIZES];
extern const struct race_data race_database[NUM_RACES];
extern const struct class_data class_database[NUM_CLASSES];
extern const struct base_attack_bonus_progression_data base_attack_bonus_progression_database[NUM_BASE_ATTACK_BONUS_PROGRESSIONS];
extern const struct ability_data ability_database[NUM_ABILITIES];
extern const struct feat_data feat_database[NUM_FEATS];
extern const struct actor_prototype monster_prototypes[NUM_MONSTERS];
extern const struct corpse_metadata corpse_metadata;
extern const struct equip_slot_data equip_slot_database[NUM_EQUIP_SLOTS];
extern const struct base_item_data base_item_database[NUM_BASE_ITEM_TYPES];
extern const struct item_data item_database[NUM_ITEM_TYPES];
extern const struct spell_data spell_database[NUM_SPELL_TYPES];
extern const struct projectile_data projectile_database[NUM_PROJECTILE_TYPES];
extern const struct surface_data surface_database[NUM_SURFACES];

#endif
