#ifndef SCENES_GAME_DIRECTION_H
#define SCENES_GAME_DIRECTION_H

enum direction
{
    DIRECTION_N,
    DIRECTION_NE,
    DIRECTION_E,
    DIRECTION_SE,
    DIRECTION_S,
    DIRECTION_SW,
    DIRECTION_W,
    DIRECTION_NW,
};

enum direction get_direction_from_angle(float angle);
void get_neighbor_by_direction(
    int x, int y,
    enum direction direction,
    int *nx, int *ny);

#endif
