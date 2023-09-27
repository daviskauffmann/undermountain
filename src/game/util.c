#include "util.h"

#include <math.h>

float distance_between_sq(
    const int x1, const int y1,
    const int x2, const int y2)
{
    const float dx = (float)(x1 - x2);
    const float dy = (float)(y1 - y2);

    return powf(dx, 2) + powf(dy, 2);
}

float distance_between(
    const int x1, const int y1,
    const int x2, const int y2)
{
    return sqrtf(distance_between_sq(x1, y1, x2, y2));
}

float angle_between(
    const int x1, const int y1,
    const int x2, const int y2)
{
    const float dx = (float)(x2 - x1);
    const float dy = (float)(y1 - y2);

    float angle = atan2f(dy, dx);

    if (angle < 0)
    {
        angle += PI * 2;
    }

    return angle;
}
