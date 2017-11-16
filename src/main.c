#include <stdio.h>
#include <SDL.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "system.h"

/* Game */
typedef struct game_s
{
    TCOD_list_t maps;
} game_t;

/* Maps */
#define MAP_WIDTH 50
#define MAP_HEIGHT 50

typedef struct map_s
{
    int level;
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
} map_t;

map_t *map_create(int level);
void map_draw(map_t *map);
void map_destroy(map_t *map);

/* Rooms */
typedef struct room_s
{
    int x;
    int y;
    int w;
    int h;
} room_t;

room_t *room_create(int x, int y, int w, int h);
void room_get_random_pos(room_t *room, int *x, int *y);
bool room_is_inside(room_t *room, int x, int y);
void room_destroy(room_t *room);

/* Tiles */
typedef enum tile_type_e {
    TILE_TYPE_EMPTY,
    TILE_TYPE_FLOOR,
    TILE_TYPE_WALL,
    TILE_TYPE_STAIR_DOWN,
    TILE_TYPE_STAIR_UP,

    NUM_TILE_TYPES
} tile_type_t;

typedef struct tile_info_s
{
    unsigned char glyph;
    bool is_transparent;
    bool is_walkable;
} tile_info_t;

typedef struct tile_s
{
    tile_type_t type;
    bool seen;
} tile_t;

tile_info_t tile_info[NUM_TILE_TYPES];

void tile_init(tile_t *tile, tile_type_t type);
void tile_update(tile_t *tile);
void tile_draw(tile_t *tile, int x, int y, float dx, float dy, float di);
void tile_uninit(tile_t *tile);

/* Components */
typedef enum component_e {
    COMPONENT_POSITION,
    COMPONENT_APPEARANCE,
    COMPONENT_INPUT,
    COMPONENT_AI,

    NUM_COMPONENTS
} component_t;

typedef struct position_s
{
    int x;
    int y;
} position_t;

/* Systems */
void input_system();

int main(int argc, char *argv[])
{
    system_init();

    SDL_Quit();

    return 0;
}