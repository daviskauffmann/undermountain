#include <libtcod.h>

#include "entity.h"

void entity_init(entity_t *entity, int8_t id, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color)
{
    entity->id = id;
    entity->x = x;
    entity->y = y;
    entity->glyph = glyph;
    entity->color = color;
}

void entity_destroy(entity_t *entity)
{
    entity->id = ENTITY_ID_UNUSED;
}

void entity_draw(entity_t *entity)
{
    TCOD_console_set_default_foreground(NULL, entity->color);
    TCOD_console_put_char(NULL, entity->x, entity->y, entity->glyph, TCOD_BKGND_NONE);
}