#include <libtcod.h>

#include "entity.h"

void entity_init(entity_t *entity, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color)
{
    entity->is_active = true;
    entity->x = x;
    entity->y = y;
    entity->glyph = glyph;
    entity->color = color;
}

void entity_destroy(entity_t *entity)
{
    entity->is_active = false;
}