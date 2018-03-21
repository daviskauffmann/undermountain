#include <libtcod/libtcod.h>
#include <math.h>

#include "util.h"

double distance_sq(int x1, int y1, int x2, int y2)
{
    double dx = (double)(x1 - x2);
    double dy = (double)(y1 - y2);

    return pow(dx, 2) + pow(dy, 2);
}

double distance(int x1, int y1, int x2, int y2)
{
    return sqrt(distance_sq(x1, y1, x2, y2));
}

double angle(int x1, int y1, int x2, int y2)
{
    double dx = (double)(x1 - x2);
    double dy = (double)(y1 - y2);

    return atan2f(dy, dx);
}

int roll(int a, int x)
{
    int result = 0;

    for (int i = 1; i <= a; i++)
    {
        result += TCOD_random_get_int(NULL, 1, x);
    }

    return result;
}
