#include <libtcod.h>

#include "entity.h"
#include "map.h"

void entity_init(Entity *entity, int id, int x, int y, char glyph, TCOD_color_t color)
{
    entity->id = id;
    entity->x = x;
    entity->y = y;
    entity->glyph = glyph;
    entity->color = color;
}

void entity_think(Entity *entity)
{
    int dir = rand() % 4;
    switch (dir)
    {
    case 0:
        entity_move(entity, 0, -1);
        break;
    case 1:
        entity_move(entity, 0, 1);
        break;
    case 2:
        entity_move(entity, -1, 0);
        break;
    case 3:
        entity_move(entity, 1, 0);
        break;
    }
}

void entity_move(Entity *entity, int dx, int dy)
{
    int x = entity->x + dx;
    int y = entity->y + dy;

    if (x >= 0 && x < MAP_WIDTH)
    {
        entity->x = x;
    }

    if (y >= 0 && y < MAP_HEIGHT)
    {
        entity->y = y;
    }
}

void entity_draw(Entity *entity)
{
    TCOD_console_set_default_foreground(NULL, entity->color);
    TCOD_console_put_char(NULL, entity->x, entity->y, entity->glyph, TCOD_BKGND_NONE);
}