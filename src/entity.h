#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include <libtcod.h>

#define ENTITY_ID_UNUSED -1
#define ENTITY_ID_PLAYER 127

typedef struct
{
    int8_t id;
    uint8_t x;
    uint8_t y;
    uint8_t glyph;
    TCOD_color_t color;
} entity_t;

void entity_init(entity_t *entity, int8_t id, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color);
void entity_destroy(entity_t *entity);
void entity_draw(entity_t *entity);

#endif