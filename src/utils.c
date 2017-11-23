#include <libtcod.h>

#include "utils.h"

int roll(int a, int x)
{
    int result = 0;

    for (int i = 1; i <= a; i++)
    {
        result += TCOD_random_get_int(NULL, 1, x);
    }

    return result;
}