#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

typedef struct map_s map_t;
typedef struct tile_s tile_t;
typedef struct room_s room_t;
typedef struct light_s light_t;
typedef struct actor_s actor_t;
typedef struct item_s item_t;
typedef struct armor_s armor_t;
typedef struct weapon_s weapon_t;
typedef struct potion_s potion_t;
typedef enum game_input_e game_input_t;
typedef enum content_e content_t;

/* Game */
#define SIMULATE_ALL_MAPS 1

TCOD_list_t maps;
actor_t *player;
int current_map_index;

void game_initialize(void);
game_input_t game_input(void);
void game_turn(void);
void game_tick(void);
void game_save(void);
void game_load(void);
void game_draw_turn(void);
void game_draw_tick(void);
void game_finalize(void);

/* Tiles */
typedef enum tile_type_e {
    TILE_TYPE_EMPTY = 0,
    TILE_TYPE_FLOOR,
    TILE_TYPE_WALL,
    TILE_TYPE_STAIR_DOWN,
    TILE_TYPE_STAIR_UP,

    NUM_TILE_TYPES
} tile_type_t;

typedef struct tile_s
{
    tile_type_t type;
    bool seen;
    light_t *light;
    actor_t *actor;
    TCOD_list_t items;
} tile_t;

unsigned char tile_glyph[NUM_TILE_TYPES];
bool tile_transparent[NUM_TILE_TYPES];
bool tile_walkable[NUM_TILE_TYPES];

void tile_initialize(tile_t *tile, tile_type_t type);
void tile_turn(tile_t *tile);
void tile_tick(tile_t *tile);
void tile_draw_turn(tile_t *tile, int x, int y);
void tile_draw_tick(tile_t *tile, int x, int y, float dx, float dy, float di);
void tile_finalize(tile_t *tile);

/* Rooms */
typedef struct room_s
{
    int x;
    int y;
    int w;
    int h;
} room_t;

room_t *room_create(map_t *map, int x, int y, int w, int h);
void room_get_random_pos(room_t *room, int *x, int *y);
bool room_is_inside(room_t *room, int x, int y);
void room_destroy(room_t *room);

/* Lights */
typedef struct light_s
{
    map_t *map;
    int x;
    int y;
    int radius;
    TCOD_color_t color;
    TCOD_map_t fov_map;
} light_t;

light_t *light_create(map_t *map, int x, int y, int radius, TCOD_color_t color);
void light_turn(light_t *light);
void light_tick(light_t *light);
void light_calc_fov(light_t *light);
void light_draw_turn(light_t *light);
void light_draw_tick(light_t *light);
void light_destroy(light_t *light);

/* Actors */
#define LIT_ROOMS 0

typedef struct actor_s
{
    map_t *map;
    int x;
    int y;
    unsigned char glyph;
    TCOD_color_t color;
    bool torch;
    TCOD_list_t items;
    int fov_radius;
    TCOD_map_t fov_map;
    bool mark_for_delete;
} actor_t;

actor_t *actor_create(map_t *map, int x, int y, unsigned char glyph, TCOD_color_t color, int fov_radius);
void actor_turn(actor_t *actor);
void actor_tick(actor_t *actor);
void actor_calc_fov(actor_t *actor);
bool actor_move_towards(actor_t *actor, int x, int y);
bool actor_move(actor_t *actor, int x, int y);
void actor_pick_item(actor_t *actor, tile_t *tile);
void actor_draw_turn(actor_t *actor);
void actor_draw_tick(actor_t *actor);
void actor_destroy(actor_t *actor);

/* Items */
typedef enum item_type_e {
    ITEM_TYPE_ARMOR,
    ITEM_TYPE_WEAPON,
    ITEM_TYPE_POTION
} item_type_t;

typedef struct item_s
{
    unsigned char glyph;
    TCOD_color_t color;
    item_type_t type;
} item_t;

typedef struct armor_s
{
    item_t item;
    int ac;
} armor_t;

typedef struct weapon_s
{
    item_t item;
    int a;
    int x;
    int b;
} weapon_t;

typedef struct potion_s
{
    item_t item;
} potion_t;

armor_t *armor_create(TCOD_list_t items, unsigned char glyph, TCOD_color_t color, int ac);
weapon_t *weapon_create(TCOD_list_t items, unsigned char glyph, TCOD_color_t color, int a, int x, int b);
potion_t *potion_create(TCOD_list_t items, unsigned char glyph, TCOD_color_t color);
void item_turn(item_t *item);
void item_tick(item_t *item);
void item_draw_turn(item_t *item, int x, int y);
void item_draw_tick(item_t *item, int x, int y);
void item_destroy(item_t *item);

/* Maps */
#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define BSP_DEPTH 10
#define MIN_ROOM_SIZE 5
#define FULL_ROOMS 1
#define NUM_LIGHTS 5
#define NUM_ACTORS 50
#define NUM_ITEMS 100

typedef struct map_s
{
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
    TCOD_list_t lights;
    TCOD_list_t actors;
} map_t;

map_t *map_create(void);
void map_turn(map_t *map);
void map_tick(map_t *map);
room_t *map_get_random_room(map_t *map);
TCOD_map_t map_to_TCOD_map(map_t *map);
void map_draw_turn(map_t *map);
void map_draw_tick(map_t *map);
void map_destroy(map_t *map);

/* Input */
#define AUTOMOVE_DELAY 0.1f

typedef enum game_input_e {
    GAME_INPUT_TICK,
    GAME_INPUT_TURN,
    GAME_INPUT_DRAW,
    GAME_INPUT_QUIT
} game_input_t;

/* Graphics */
#define CONSTRAIN_VIEW 1

TCOD_color_t background_color;
TCOD_color_t foreground_color;
TCOD_color_t tile_color_light;
TCOD_color_t tile_color_dark;
TCOD_color_t torch_color;

bool sfx;

int view_x;
int view_y;
int view_width;
int view_height;

/* Message Log */
#define MAX_MESSAGES 20

TCOD_console_t msg;
bool msg_visible;
int msg_x;
int msg_y;
int msg_width;
int msg_height;
TCOD_list_t messages;

void msg_log(const char *message, map_t *map, int x, int y);
void msg_draw(void);

/* Side Menu */
typedef enum content_e {
    CONTENT_CHARACTER,
    CONTENT_INVENTORY,

    NUM_CONTENTS
} content_t;

TCOD_console_t panel;
bool panel_visible;
int panel_x;
int panel_y;
int panel_width;
int panel_height;
content_t content;
int content_scroll[NUM_CONTENTS];
int content_height[NUM_CONTENTS];

void panel_draw(void);

/* Tooltip */
TCOD_console_t tooltip;
bool tooltip_visible;
int tooltip_x;
int tooltip_y;
int tooltip_width;
int tooltip_height;

#endif