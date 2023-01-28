#include "direction.h"

#include "../../game/util.h"

enum direction get_direction_from_angle(const float angle)
{
    if ((angle > 11 * PI / 6 && angle <= 2 * PI) || (angle >= 0 && angle < PI / 6))
    {
        return DIRECTION_E;
    }
    else if (angle >= PI / 6 && angle <= PI / 3)
    {
        return DIRECTION_NE;
    }
    else if (angle > PI / 3 && angle < 2 * PI / 3)
    {
        return DIRECTION_N;
    }
    else if (angle >= 2 * PI / 3 && angle <= 5 * PI / 6)
    {
        return DIRECTION_NW;
    }
    else if (angle > 5 * PI / 6 && angle < 7 * PI / 6)
    {
        return DIRECTION_W;
    }
    else if (angle >= 7 * PI / 6 && angle <= 4 * PI / 3)
    {
        return DIRECTION_SW;
    }
    else if (angle > 4 * PI / 3 && angle < 5 * PI / 3)
    {
        return DIRECTION_S;
    }
    else if (angle >= 5 * PI / 3 && angle <= 11 * PI / 6)
    {
        return DIRECTION_SE;
    }

    return -1;
}

void get_neighbor_by_direction(
    const int x, const int y,
    const enum direction direction,
    int *const nx, int *const ny)
{
    *nx = x;
    *ny = y;
    switch (direction)
    {
    case DIRECTION_N:
        (*ny)--;
        break;
    case DIRECTION_NE:
        (*ny)--;
        (*nx)++;
        break;
    case DIRECTION_E:
        (*nx)++;
        break;
    case DIRECTION_SE:
        (*nx)++;
        (*ny)++;
        break;
    case DIRECTION_S:
        (*ny)++;
        break;
    case DIRECTION_SW:
        (*nx)--;
        (*ny)++;
        break;
    case DIRECTION_W:
        (*nx)--;
        break;
    case DIRECTION_NW:
        (*nx)--;
        (*ny)--;
        break;
    }
}
