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
actor_t *player;
int turn;

void game_init(void);
game_input_t game_input(void);
void game_turn(void);
void game_tick(void);
void game_save(void);
void game_load(void);
void game_draw_turn(void);
void game_draw_tick(void);
void game_uninit(void);

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

void tile_init(tile_t *tile, tile_type_t type);
void tile_turn(tile_t *tile);
void tile_tick(tile_t *tile);
void tile_draw_turn(tile_t *tile, int x, int y);
void tile_draw_tick(tile_t *tile, int x, int y, float dx, float dy, float di);
void tile_uninit(tile_t *tile);

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

/* Lights */
typedef struct light_s
{
    map_t *map;
    int x;
    int y;
    int radius;
    TCOD_color_t color;
    bool on;
    TCOD_map_t fov_map;
} light_t;

light_t *light_create(map_t *map, int x, int y, int radius, TCOD_color_t color);
void light_turn(light_t *light);
void light_tick(light_t *light);
void light_calc_fov(light_t *light);
void light_draw_turn(light_t *light);
void light_draw_tick(light_t *light);
void light_destroy(light_t *light);

/* Items */
typedef enum item_type_e {
    ITEM_TYPE_ARMOR,
    ITEM_TYPE_WEAPON,
    ITEM_TYPE_POTION,
    ITEM_TYPE_CORPSE
} item_type_t;

typedef struct item_s
{
    int x;
    int y;
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
    int b;
    int c;
} weapon_t;

typedef struct potion_s
{
    item_t item;
} potion_t;

typedef struct corpse_s
{
    item_t item;
} corpse_t;

item_t *item_create_random(int x, int y);
armor_t *armor_create(int x, int y, unsigned char glyph, TCOD_color_t color, int ac);
weapon_t *weapon_create(int x, int y, unsigned char glyph, TCOD_color_t color, int a, int b, int c);
potion_t *potion_create(int x, int y, unsigned char glyph, TCOD_color_t color);
corpse_t *corpse_create(int x, int y, actor_t *actor);
void item_turn(item_t *item);
void item_tick(item_t *item);
void item_draw_turn(item_t *item);
void item_draw_tick(item_t *item);
void item_destroy(item_t *item);

/* Actors */
typedef enum actor_light_e {
    ACTOR_LIGHT_NONE,
    ACTOR_LIGHT_DEFAULT,
    ACTOR_LIGHT_TORCH,

    NUM_ACTOR_LIGHTS
} actor_light_t;

typedef struct interactions_s
{
    bool descend;
    bool ascend;
    bool light_on;
    bool light_off;
    bool attack;
    bool take_item;
    bool take_items;
} interactions_t;

typedef struct target_data_s
{
    int x;
    int y;
    actor_t *actor;
    interactions_t interactions;
} target_data_t;

typedef struct actor_s
{
    map_t *map;
    int x;
    int y;
    unsigned char glyph;
    TCOD_color_t color;
    TCOD_list_t items;
    actor_light_t light;
    TCOD_map_t fov_map;
    int speed;
    int turns_waited;
    void (*ai)(actor_t *actor);
    bool target;
    target_data_t target_data;
    bool mark_for_delete;
} actor_t;

int actor_light_radius[NUM_ACTOR_LIGHTS];
TCOD_color_t actor_light_color[NUM_ACTOR_LIGHTS];

actor_t *actor_create(map_t *map, int x, int y, unsigned char glyph, TCOD_color_t color, void (*ai)(actor_t *actor));
void actor_turn(actor_t *actor);
void actor_tick(actor_t *actor);
void actor_calc_fov(actor_t *actor);
void actor_target_set(actor_t *actor, int x, int y, interactions_t interactions);
void actor_target_process(actor_t *actor);
void actor_draw_turn(actor_t *actor);
void actor_draw_tick(actor_t *actor);
void actor_destroy(actor_t *actor);

void ai_monster(actor_t *actor);
void ai_pet(actor_t *actor);

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
    TCOD_list_t lights;
    TCOD_list_t actors;
    TCOD_list_t items;
} map_t;

map_t *map_create(int level);
void map_turn(map_t *map);
void map_tick(map_t *map);
bool map_is_inside(int x, int y);
room_t *map_get_random_room(map_t *map);
TCOD_map_t map_to_TCOD_map(map_t *map);
void map_draw_turn(map_t *map);
void map_draw_tick(map_t *map);
void map_destroy(map_t *map);

/* World */
TCOD_list_t maps;

void world_init(void);
void world_turn(void);
void world_tick(void);
void world_draw_turn(void);
void world_draw_tick(void);
void world_uninit(void);

/* Input */
typedef enum game_input_e {
    GAME_INPUT_TICK,
    GAME_INPUT_TURN,
    GAME_INPUT_DRAW,
    GAME_INPUT_QUIT
} game_input_t;

int mouse_x;
int mouse_y;
int mouse_tile_x;
int mouse_tile_y;

/* Graphics */
TCOD_color_t background_color;
TCOD_color_t foreground_color;
TCOD_color_t tile_color_light;
TCOD_color_t tile_color_dark;

void gfx_init(void);
void gfx_draw_turn(void);
void gfx_draw_tick(void);
void gfx_uninit(void);

/* View */
int view_x;
int view_y;
int view_width;
int view_height;

bool view_is_inside(int x, int y);

/* Message Log */
bool msg_visible;
int msg_x;
int msg_y;
int msg_width;
int msg_height;

void msg_init(void);
void msg_log(const char *message, map_t *map, int x, int y);
bool msg_is_inside(int x, int y);
void msg_draw_turn(void);
void msg_draw_tick(void);
void msg_uninit(void);

/* Side Menu */
typedef enum content_e {
    CONTENT_CHARACTER,
    CONTENT_INVENTORY,

    NUM_CONTENTS
} content_t;

bool panel_visible;
int panel_x;
int panel_y;
int panel_width;
int panel_height;

content_t content;
int content_height[NUM_CONTENTS];
int content_scroll[NUM_CONTENTS];

void panel_init(void);
void panel_toggle(content_t new_content);
void panel_content_scroll_down(void);
void panel_content_scroll_up(void);
bool panel_is_inside(int x, int y);
void panel_draw_turn(void);
void panel_draw_tick(void);
void panel_uninit(void);

/* Tooltip */
bool tooltip_visible;
int tooltip_x;
int tooltip_y;
int tooltip_width;
int tooltip_height;

void tooltip_init(void);
void tooltip_show(int x, int y);
void tooltip_hide(void);
bool tooltip_is_inside(int x, int y);
void tooltip_draw_turn(void);
void tooltip_draw_tick(void);
void tooltip_uninit(void);

/* Tooltip options */
typedef struct tooltip_data_s
{
    int tile_x;
    int tile_y;
    item_t *item;
} tooltip_data_t;

typedef struct tooltip_option_s
{
    char *text;
    void (*fn)(tooltip_data_t data);
    tooltip_data_t data;
} tooltip_option_t;

TCOD_list_t tooltip_options;

void tooltip_options_add(char *text, void (*fn)(tooltip_data_t data), tooltip_data_t data);
void tooltip_options_clear(void);

void tooltip_option_move(tooltip_data_t data);
void tooltip_option_stair_descend(tooltip_data_t data);
void tooltip_option_stair_ascend(tooltip_data_t data);
void tooltip_option_light_on(tooltip_data_t data);
void tooltip_option_light_off(tooltip_data_t data);
void tooltip_option_item_take(tooltip_data_t data);
void tooltip_option_item_take_all(tooltip_data_t data);
void tooltip_option_item_drop(tooltip_data_t data);
void tooltip_option_actor_attack(tooltip_data_t data);

#endif