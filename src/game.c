#include <libtcod.h>

#include "game.h"

tileinfo_t tileinfo[TILETYPE_COUNT];

void tileinfo_init(void)
{
    tileinfo[TILETYPE_EMPTY].glyph = ' ';
    tileinfo[TILETYPE_EMPTY].color = TCOD_white;
    tileinfo[TILETYPE_EMPTY].opaque = false;
    tileinfo[TILETYPE_EMPTY].solid = false;

    tileinfo[TILETYPE_FLOOR].glyph = '.';
    tileinfo[TILETYPE_FLOOR].color = TCOD_white;
    tileinfo[TILETYPE_FLOOR].opaque = false;
    tileinfo[TILETYPE_FLOOR].solid = false;

    tileinfo[TILETYPE_WALL].glyph = '#';
    tileinfo[TILETYPE_WALL].color = TCOD_white;
    tileinfo[TILETYPE_WALL].opaque = true;
    tileinfo[TILETYPE_WALL].solid = true;
}

void tile_init(tile_t *tile, tiletype_t type)
{
    tile->type = type;
    tile->seen = false;
}

void tile_draw(tile_t *tile, int x, int y)
{
    // TODO: use visibility to determine color
    // if visible && seen
    //     white
    // if !visible && seen
    //     grey
    // if !visible && !seen
    //     don't draw
    TCOD_console_set_default_foreground(NULL, tileinfo[tile->type].color);
    TCOD_console_put_char(NULL, x, y, tileinfo[tile->type].glyph, TCOD_BKGND_NONE);
}

void room_init(room_t *room, int x, int y, int w, int h)
{
    room->x1 = x;
    room->y1 = y;
    room->x2 = x + w;
    room->y2 = y + h;
}

void room_carve(map_t *map, room_t *room)
{
    for (int x = room->x1; x < room->x2; x++)
    {
        for (int y = room->y1; y < room->y2; y++)
        {
            map->tiles[x][y].type = TILETYPE_FLOOR;
        }
    }
}

void entity_init(entity_t *entity, int id, int x, int y, char glyph, TCOD_color_t color)
{
    entity->id = id;
    entity->x = x;
    entity->y = y;
    entity->glyph = glyph;
    entity->color = color;
}

void entity_think(map_t *map, entity_t *entity)
{
    int dir = rand() % 4;
    switch (dir)
    {
    case 0:
        entity_move(map, entity, 0, -1);
        break;
    case 1:
        entity_move(map, entity, 0, 1);
        break;
    case 2:
        entity_move(map, entity, -1, 0);
        break;
    case 3:
        entity_move(map, entity, 1, 0);
        break;
    }
}

void entity_move(map_t *map, entity_t *entity, int dx, int dy)
{
    int x = entity->x + dx;
    int y = entity->y + dy;

    if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
    {
        return;
    }

    tile_t *tile = &map->tiles[x][y];
    if (tileinfo[tile->type].solid)
    {
        return;
    }

    entity->x = x;
    entity->y = y;
}

void entity_draw(entity_t *entity)
{
    TCOD_console_set_default_foreground(NULL, entity->color);
    TCOD_console_put_char(NULL, entity->x, entity->y, entity->glyph, TCOD_BKGND_NONE);
}

void map_init(map_t *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile_init(tile, TILETYPE_EMPTY);
        }
    }

    for (int i = 0; i < MAX_ROOMS; i++)
    {
        room_t *room = &map->rooms[i];

        room_init(room, 0, 0, 0, 0);
    }

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &map->entities[i];

        entity_init(entity, ID_UNUSED, 0, 0, ' ', TCOD_white);
    }
}

void map_generate(map_t *map)
{
    // TODO: libtcod BSP generation
    room_t *room1 = &map->rooms[0];
    room_init(room1, 20, 15, 10, 15);
    room_carve(map, room1);

    entity_t *player = &map->entities[ID_PLAYER];
    entity_init(player, ID_PLAYER, 1, 1, '@', TCOD_white);

    int npcId = 0;
    entity_t *npc = &map->entities[npcId];
    entity_init(npc, npcId, 5, 5, '@', TCOD_yellow);
}

void map_update(map_t *map)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &map->entities[i];

        if (entity->id == ID_UNUSED)
        {
            continue;
        }

        if (entity->id != ID_PLAYER)
        {
            entity_think(map, entity);
        }
    }
}

void map_draw(map_t *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            // TODO: check if visible
            tile_draw(tile, x, y /*, visible: bool*/);
        }
    }

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &map->entities[i];

        if (entity->id == ID_UNUSED)
        {
            continue;
        }

        // TODO: check if visible
        entity_draw(entity);
    }

    TCOD_console_flush();
}