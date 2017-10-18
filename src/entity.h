#ifndef ENTITY_H
#define ENTITY_H

#include <libtcod.h>

#define ID_UNUSED -1
#define ID_PLAYER 0

typedef struct
{
    int id;
    int x;
    int y;
    char glyph;
    TCOD_color_t color;
} Entity;

void entity_init(Entity *entity, int id, int x, int y, char glyph, TCOD_color_t color);
void entity_think(Entity *entity);
void entity_move(Entity *entity, int dx, int dy);
void entity_draw(Entity *entity);

#endif