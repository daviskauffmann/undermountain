#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

typedef struct map_s map_t;
typedef struct tile_s tile_t;
typedef struct room_s room_t;
typedef struct light_s light_t;
typedef struct item_s item_t;
typedef struct armor_s armor_t;
typedef struct weapon_s weapon_t;
typedef struct potion_s potion_t;
typedef struct scroll_s scroll_t;
typedef struct spell_s spell_t;
typedef struct actor_s actor_t;
typedef enum game_input_e game_input_t;
typedef enum content_e content_t;

/* Game */
typedef enum game_status_e {
    GAME_STATUS_WAITING,
    GAME_STATUS_UPDATE,
    GAME_STATUS_QUIT
} game_status_t;

game_status_t game_status;
int turn;
actor_t *player;

void game_init(void);
void game_input(void);
void game_update(void);
void game_save(void);
void game_load(void);
void game_draw(void);
void game_uninit(void);

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
    light_t *light;
    actor_t *actor;
    TCOD_list_t items;
} tile_t;

tile_info_t tile_info[NUM_TILE_TYPES];

void tile_init(tile_t *tile, tile_type_t type);
void tile_update(tile_t *tile);
void tile_draw(tile_t *tile, int x, int y, float dx, float dy, float di);
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
typedef struct light_common_s
{
    unsigned char glyph;
} light_common_t;

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

light_common_t light_common;

light_t *light_create(map_t *map, int x, int y, int radius, TCOD_color_t color);
void light_calc_fov(light_t *light);
void light_update(light_t *light);
void light_draw(light_t *light);
void light_destroy(light_t *light);

/* Items */
typedef enum item_type_e {
    ITEM_TYPE_GOLD,
    ITEM_TYPE_TORCH,
    ITEM_TYPE_SWORD_LONG,
    ITEM_TYPE_SHIELD_LARGE,
    ITEM_TYPE_POTION_HEALING,
    ITEM_TYPE_POTION_MANA,

    NUM_ITEM_TYPES
} item_type_t;

typedef enum item_light_type_e {
    ITEM_LIGHT_TYPE_TORCH,

    NUM_ITEM_LIGHT_TYPES
} item_light_type_t;

typedef struct item_info_s
{
    char *name;
    unsigned char glyph;
    TCOD_color_t color;
    bool identified;
    int max_stack;
} item_info_t;

typedef struct item_common_s
{
    int torch_radius;
    TCOD_color_t torch_color;
} item_common_t;

typedef struct item_s
{
    item_type_t type;
    map_t *map;
    int x;
    int y;
    int stack;
    bool torch;
    TCOD_map_t fov_map;
} item_t;

item_common_t item_common;
item_info_t item_info[NUM_ITEM_TYPES];

item_t *item_create(item_type_t type, map_t *map, int x, int y, int stack);
void item_update(item_t *item);
void item_calc_fov(item_t *item);
void item_draw(item_t *item);
void item_destroy(item_t *item);

/* Spells */
typedef enum spell_type_e {
    SPELL_INSTAKILL,

    NUM_SPELL_TYPES
} spell_type_t;

typedef struct spell_s
{
    char *name;
    void (*cast)(actor_t *caster, int x, int y);
} spell_info_t;

spell_info_t spell_info[NUM_SPELL_TYPES];

void spell_instakill(actor_t *caster, int x, int y);

/* Actors */
typedef enum actor_type_e {
    ACTOR_TYPE_WARRIOR,
    ACTOR_TYPE_MAGE,
    ACTOR_TYPE_ROGUE,
    ACTOR_TYPE_DOG,
    ACTOR_TYPE_SKELETON,
    ACTOR_TYPE_SKELETON_CAPTAIN,
    ACTOR_TYPE_ZOMBIE,
    ACTOR_TYPE_JACKAL,

    NUM_ACTOR_TYPES
} actor_type_t;

typedef struct actor_common_s
{
    int glow_radius;
    TCOD_color_t glow_color;
} actor_common_t;

typedef struct actor_info_s
{
    char *name;
    unsigned char glyph;
    TCOD_color_t color;
    float energy_per_turn;
} actor_info_t;

typedef struct actor_light_info_s
{
    int radius;
    TCOD_color_t color;
} actor_light_info_t;

typedef struct actor_s
{
    actor_type_t type;
    map_t *map;
    int x;
    int y;
    char *unique_name;
    void (*ai)(actor_t *actor);
    TCOD_list_t items;
    TCOD_list_t spells;
    spell_t *spell_ready;
    bool glow;
    TCOD_map_t fov_map;
    float energy;
    bool mark_for_delete;
} actor_t;

actor_common_t actor_common;
actor_info_t actor_info[NUM_ACTOR_TYPES];

actor_t *actor_create(actor_type_t type, map_t *map, int x, int y, void (*ai)(actor_t *actor), char *unique_name);
void actor_update(actor_t *actor);
char *actor_get_name(actor_t *actor);
void actor_calc_fov(actor_t *actor);
void actor_move(actor_t *actor, int x, int y);
void actor_path_towards(actor_t *actor, int x, int y);
void actor_move_towards(actor_t *actor, int x, int y);
void actor_swap(actor_t *actor, actor_t *other);
void actor_attack(actor_t *actor, actor_t *other);
void actor_light_toggle(actor_t *actor, light_t *light);
void actor_item_take(actor_t *actor, TCOD_list_t items);
void actor_look(actor_t *actor, int x, int y);
void actor_swing(actor_t *actor, int x, int y);
void actor_draw(actor_t *actor);
void actor_destroy(actor_t *actor);

/* AI */
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
void map_update(map_t *map);
bool map_is_inside(int x, int y);
room_t *map_get_random_room(map_t *map);
TCOD_map_t map_to_TCOD_map(map_t *map);
TCOD_map_t map_to_fov_map(map_t *map, int x, int y, int radius);
void map_draw(map_t *map);
void map_destroy(map_t *map);

/* World */
TCOD_list_t maps;

void world_init(void);
void world_update(void);
void world_draw(void);
void world_uninit(void);

/* Graphics */
TCOD_color_t background_color;
TCOD_color_t foreground_color;
TCOD_color_t tile_color_light;
TCOD_color_t tile_color_dark;

void gfx_init(void);
void gfx_draw(void);
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
void msg_log(map_t *map, int x, int y, TCOD_color_t color, char *text, ...);
bool msg_is_inside(int x, int y);
void msg_draw(void);
void msg_uninit(void);

/* Messages */
typedef struct message_s
{
    char *text;
    TCOD_color_t color;
} message_t;

message_t *message_create(char *text, TCOD_color_t color);
void message_destroy(message_t *message);

/* Side Menu */
typedef enum content_e {
    CONTENT_CHARACTER,
    CONTENT_INVENTORY,
    CONTENT_SPELLBOOK,

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
int content_selected_idx[NUM_CONTENTS];

void panel_init(void);
void panel_toggle(content_t new_content);
void panel_content_scroll_down(void);
void panel_content_scroll_up(void);
void panel_content_idx_up(void);
void panel_content_idx_down(void);
bool panel_is_inside(int x, int y);
void panel_draw(void);
void panel_uninit(void);

#endif