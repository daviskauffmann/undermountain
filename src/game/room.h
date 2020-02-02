#ifndef GAME_ROOM_H
#define GAME_ROOM_H

struct room
{
    int x;
    int y;
    int w;
    int h;
};

void room_init(struct room *room, int x, int y, int w, int h);
void room_reset(struct room *room);
void room_get_random_pos(struct room *room, int *x, int *y);

#endif
