#include <libtcod/libtcod.h>
#include <math.h>

#include "utils.h"

double distance_sq(int x1, int y1, int x2, int y2)
{
    return pow(x1 - x2, 2) + pow(y1 - y2, 2);
}

double distance(int x1, int y1, int x2, int y2)
{
    return sqrt(distance_sq(x1, y1, x2, y2));
}

double angle(int x1, int y1, int x2, int y2)
{
    double a = atan2(y1 - y2, -(x1 - x2)) * 57.2957795;

    a = a < 0 ? a + 360.0 : a;

    return a;
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
