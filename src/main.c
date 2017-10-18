#include <SDL.h>
#include <libtcod.h>
#include <stdio.h>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 50
#define WINDOW_TITLE "Roguelike"

#define MAP_WIDTH 80
#define MAP_HEIGHT 50
#define MAX_ENTITIES 255

#define ID_UNUSED -1
#define ID_PLAYER 254

typedef enum {
    TILE_EMPTY = 0,
    TILE_FLOOR,
    TILE_WALL
} tile_type_t;

typedef struct
{
    tile_type_t type;
    bool seen;
} tile_t;

typedef struct
{
    int id;
    int x;
    int y;
    char glyph;
    TCOD_color_t color;
} entity_t;

typedef struct
{
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    entity_t entities[MAX_ENTITIES];
} map_t;

void tile_init(tile_t *tile, tile_type_t type);
void tile_draw(tile_t *tile, int x, int y);

void entity_init(entity_t *entity, int id, int x, int y, char glyph,
                 TCOD_color_t color);
void entity_think(map_t *map, entity_t *entity);
void entity_move(map_t *map, entity_t *entity, int dx, int dy);
void entity_draw(entity_t *entity);

void map_init(map_t *map);
void map_generate(map_t *map);
void map_update(map_t *map);
void map_draw(map_t *map);

int main(int argc, char *argv[])
{
    time_t t;
    srand((unsigned)time(&t));

    TCOD_console_init_root(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE, false,
                           TCOD_RENDERER_SDL);

    map_t *map = malloc(sizeof(map_t));
    map_init(map);
    map_generate(map);

    map_draw(map);

    entity_t *player = &map->entities[ID_PLAYER];

    while (!TCOD_console_is_window_closed())
    {
        TCOD_key_t key;
        TCOD_mouse_t mouse;
        TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

        if (ev == TCOD_EVENT_KEY_PRESS)
        {
            switch (key.vk)
            {
            case TCODK_ESCAPE:
                goto quit;
            case TCODK_UP:
                entity_move(map, player, 0, -1);
                map_update(map);
                map_draw(map);
                break;
            case TCODK_LEFT:
                entity_move(map, player, -1, 0);
                map_update(map);
                map_draw(map);
                break;
            case TCODK_DOWN:
                entity_move(map, player, 0, 1);
                map_update(map);
                map_draw(map);
                break;
            case TCODK_RIGHT:
                entity_move(map, player, 1, 0);
                map_update(map);
                map_draw(map);
                break;
            }
        }
    }
quit:

    SDL_Quit();

    return 0;
}

void tile_init(tile_t *tile, tile_type_t type)
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
    TCOD_color_t color = TCOD_white;

    char glyph = ' ';
    switch (tile->type)
    {
    case TILE_FLOOR:
        glyph = '.';
        break;
    case TILE_WALL:
        glyph = '#';
        break;
    }

    TCOD_console_set_default_foreground(NULL, color);
    TCOD_console_put_char(NULL, x, y, glyph, TCOD_BKGND_NONE);
}

void entity_init(entity_t *entity, int id, int x, int y, char glyph,
                 TCOD_color_t color)
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
    if (tile->type == TILE_WALL)
    {
        return;
    }

    entity->x = x;
    entity->y = y;
}

void entity_draw(entity_t *entity)
{
    TCOD_console_set_default_foreground(NULL, entity->color);
    TCOD_console_put_char(NULL, entity->x, entity->y, entity->glyph,
                          TCOD_BKGND_NONE);
}

void map_init(map_t *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile_init(tile, TILE_EMPTY);
        }
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
    map->tiles[20][15].type = TILE_FLOOR;
    map->tiles[21][15].type = TILE_FLOOR;
    map->tiles[20][16].type = TILE_FLOOR;
    map->tiles[21][16].type = TILE_FLOOR;

    map->tiles[25][15].type = TILE_WALL;
    map->tiles[26][15].type = TILE_WALL;
    map->tiles[25][16].type = TILE_WALL;
    map->tiles[26][16].type = TILE_WALL;

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