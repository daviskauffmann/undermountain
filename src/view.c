#include <libtcod.h>

#include "game.h"

bool view_is_inside(int x, int y)
{
    return x >= 0 && x < view_width && y >= 0 && y < view_height;
}