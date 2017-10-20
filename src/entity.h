#ifndef ENTITY_H
#define ENTITY_H

#include <stdint.h>
#include <libtcod.h>

typedef struct
{
    bool is_player;
    bool is_active;
    uint8_t x;
    uint8_t y;
    uint8_t glyph;
    TCOD_color_t color;
} entity_t;

void entity_init(entity_t *entity, uint8_t x, uint8_t y, uint8_t glyph, TCOD_color_t color);
void entity_destroy(entity_t *entity);

#endif