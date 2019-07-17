#ifndef GAME_ROOM_H
#define GAME_ROOM_H

struct room
{
    int x;
    int y;
    int w;
    int h;
};

struct room *room_create(int x, int y, int w, int h);
void room_get_random_pos(struct room *room, int *x, int *y);
void room_destroy(struct room *room);

#endif
