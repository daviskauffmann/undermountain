#include "color.h"

struct color color_lerp(const struct color from, const struct color to, const float amount)
{
    return (struct color){
        (uint8_t)(from.r + (to.r - from.r) * amount),
        (uint8_t)(from.g + (to.g - from.g) * amount),
        (uint8_t)(from.b + (to.b - from.b) * amount),
    };
}
