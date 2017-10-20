#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <libtcod.h>

#include "tile.h"
#include "room.h"
#include "entity.h"

#define MAP_WIDTH 80
#define MAP_HEIGHT 50
#define MAP_MAX_ROOMS 256
#define MAP_MAX_ENTITIES 256

// TODO: use TCOD lists?
typedef struct
{
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    room_t rooms[MAP_MAX_ROOMS];
    entity_t entities[MAP_MAX_ENTITIES];
} map_t;

void map_init(map_t *map);
void map_generate(map_t *map);
room_t *map_room_create(map_t *map, uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void map_room_carve(map_t *map, room_t *room);
void map_update(map_t *map);
entity_t *map_entity_create(map_t *map, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color);
void map_entity_update(map_t *map, entity_t *entity);
void map_entity_move(map_t *map, entity_t *entity, int dx, int dy);
TCOD_map_t map_calc_fov(map_t *map, entity_t *entity);
void map_draw(map_t *map, entity_t *player);

#endif