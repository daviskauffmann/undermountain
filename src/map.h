#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

#include "tile.h"
#include "room.h"
#include "entity.h"

#define MAP_WIDTH 80
#define MAP_HEIGHT 50
#define MAP_MIN_ROOMS 5
#define MAP_MAX_ROOMS 10
#define MAP_MAX_ENTITIES 128

typedef struct
{
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    room_t rooms[MAP_MAX_ROOMS];
    entity_t entities[MAP_MAX_ENTITIES];
} map_t;

void map_init(map_t *map);
void map_generate(map_t *map);
void map_room_carve(map_t *map, room_t *room);
void map_update(map_t *map);
void map_entity_update(map_t *map, entity_t *entity);
void map_entity_move(map_t *map, entity_t *entity, int dx, int dy);
void map_draw(map_t *map);

#endif