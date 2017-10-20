#ifndef ROOM_H
#define ROOM_H

#include <stdint.h>
#include <libtcod.h>

typedef struct
{
    uint8_t x1;
    uint8_t y1;
    uint8_t x2;
    uint8_t y2;
    bool is_created;
} room_t;

void room_init(room_t *room, uint8_t x, uint8_t y, uint8_t w, uint8_t h);

#endif