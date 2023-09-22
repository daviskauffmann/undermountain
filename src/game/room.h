#ifndef UM_GAME_ROOM_H
#define UM_GAME_ROOM_H

#include <stdint.h>

struct room
{
    uint8_t x;
    uint8_t y;
    uint8_t w;
    uint8_t h;
};

struct room *room_new(
    uint8_t x,
    uint8_t y,
    uint8_t w,
    uint8_t h);
void room_delete(struct room *room);
void room_get_random_pos(
    const struct room *room,
    int *x, int *y);

#endif
