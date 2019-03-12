#ifndef ROGUELIKE_ASSETS_H
#define ROGUELIKE_ASSETS_H

#include "actor.h"
#include "item.h"
#include "object.h"
#include "tile.h"

extern struct tile_common tile_common;
extern struct tile_info tile_info[NUM_TILE_TYPES];
extern struct object_common object_common;
extern struct object_info object_info[NUM_OBJECT_TYPES];
extern struct actor_common actor_common;
extern struct race_info race_info[NUM_RACES];
extern struct class_info class_info[NUM_CLASSES];
extern struct prototype monster_prototype[NUM_MONSTERS];
extern struct ability_info ability_info[NUM_ABILITIES];
extern struct item_common item_common;
extern struct equip_slot_info equip_slot_info[NUM_EQUIP_SLOTS];
extern struct base_item_info base_item_info[NUM_BASE_ITEMS];
extern struct item_info item_info[NUM_ITEM_TYPES];

void assets_load(void);
void assets_unload(void);

#endif
