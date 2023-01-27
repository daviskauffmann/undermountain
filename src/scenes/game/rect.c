#include "rect.h"

bool rect_is_inside(const struct rect rect, const int x, const int y)
{
    return rect.visible &&
           x >= rect.x && x < rect.x + rect.width &&
           y >= rect.y && y < rect.y + rect.height;
}
