#include "room.h"

void room_init(room_t *room, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    room->x1 = x;
    room->y1 = y;
    room->x2 = x + w;
    room->y2 = y + h;
    room->is_created = true;
}