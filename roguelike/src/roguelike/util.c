#include <roguelike/roguelike.h>

float distance_sq(int x1, int y1, int x2, int y2)
{
    float dx = (float)(x1 - x2);
    float dy = (float)(y1 - y2);

    return powf(dx, 2) + powf(dy, 2);
}

float distance(int x1, int y1, int x2, int y2)
{
    return sqrtf(distance_sq(x1, y1, x2, y2));
}

float angle(int x1, int y1, int x2, int y2)
{
    float dx = (float)(x1 - x2);
    float dy = (float)(y1 - y2);

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
