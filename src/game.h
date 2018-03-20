#ifndef GAME_H
#define GAME_H

#include "actor.h"
#include "map.h"
#include "object.h"
#include "tile.h"

#define NUM_MAPS 10

typedef struct
{
    Map maps[NUM_MAPS];
    TileCommon tile_common;
    TileInfo tile_info[NUM_TILE_TYPES];
    ObjectCommon object_common;
    ObjectInfo object_info[NUM_OBJECT_TYPES];
    ActorCommon actor_common;
    ActorInfo actor_info[NUM_ACTOR_TYPES];
} Game;

#endif
