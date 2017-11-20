#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

/* Entities */
#define MAX_ENTITIES 65536
#define ID_UNUSED -1

typedef struct entity_s
{
    int id;
} entity_t;

entity_t entities[MAX_ENTITIES];

entity_t *entity_create(void);
void entity_path_towards(entity_t *entity, int x, int y);
void entity_move_towards(entity_t *entity, int x, int y);
void entity_move_random(entity_t *entity);
void entity_move(entity_t *entity, int x, int y);
void entity_swap(entity_t *entity, entity_t *other);
void entity_swing(entity_t *entity, int x, int y);
void entity_attack(entity_t *entity, entity_t *other);
void entity_die(entity_t *entity);
void entity_destroy(entity_t *entity);

/* Components */
typedef enum component_type_e {
    COMPONENT_POSITION,
    COMPONENT_PHYSICS,
    COMPONENT_LIGHT,
    COMPONENT_FOV,
    COMPONENT_APPEARANCE,
    COMPONENT_AI,
    COMPONENT_HEALTH,
    COMPONENT_ALIGNMENT,
    COMPONENT_TARGETING,

    NUM_COMPONENTS
} component_type_t;

typedef struct position_s
{
    struct map_s *map;
    int x;
    int y;
} position_t;

typedef enum ai_type_e {
    AI_MONSTER,
    AI_PET
} ai_type_t;

typedef struct ai_s
{
    ai_type_t type;
    float energy;
    float energy_per_turn;
} ai_t;

typedef struct physics_s
{
    bool is_walkable;
    bool is_transparent;
} physics_t;

typedef enum light_type_e {
    LIGHT_PRIORITY_0,
    LIGHT_PRIORITY_1,

    NUM_LIGHT_PRIORITIES
} light_priority_t;

typedef struct light_s
{
    int radius;
    TCOD_color_t color;
    bool flicker;
    light_priority_t priority;
    TCOD_map_t fov_map;
} light_t;

typedef struct fov_s
{
    int radius;
    TCOD_map_t fov_map;
} fov_t;

typedef enum layer_e {
    LAYER_0,
    LAYER_1,
    LAYER_2,

    NUM_LAYERS,
} layer_t;

typedef struct appearance_s
{
    char *name;
    unsigned char glyph;
    TCOD_color_t color;
    layer_t layer;
} appearance_t;

typedef struct health_s
{
    int max;
    int current;
} health_t;

typedef enum alignment_type_e {
    ALIGNMENT_GOOD,
    ALIGNMENT_EVIL,
} alignment_type_t;

typedef struct alignment_s
{
    alignment_type_t type;
} alignment_t;

typedef struct targeting_s
{
    bool active;
    int x;
    int y;
} targeting_t;

typedef struct component_s
{
    int id;
    component_type_t type;
    union {
        position_t position;
        ai_t ai;
        physics_t physics;
        light_t light;
        fov_t fov;
        appearance_t appearance;
        health_t health;
        alignment_t alignment;
        targeting_t targeting;
    };
} component_t;

component_t components[NUM_COMPONENTS][MAX_ENTITIES];

component_t *component_add(entity_t *entity, component_type_t component_type);
component_t *component_get(entity_t *entity, component_type_t component_type);
void component_remove(entity_t *entity, component_type_t component_type);

/* Tiles */
typedef enum tile_type_e {
    TILE_FLOOR,
    TILE_WALL,
    TILE_STAIR_DOWN,
    TILE_STAIR_UP,

    NUM_TILES
} tile_type_t;

typedef struct tile_common_s
{
    TCOD_color_t shadow_color;
} tile_common_t;

typedef struct tile_info_s
{
    unsigned char glyph;
    TCOD_color_t color;
    bool is_walkable;
    bool is_transparent;
} tile_info_t;

typedef struct tile_s
{
    tile_type_t type;
    bool seen;
    TCOD_list_t entities;
} tile_t;

tile_common_t tile_common;
tile_info_t tile_info[NUM_TILES];

void tile_init(tile_t *tile, tile_type_t type, bool seen);
void tile_reset(tile_t *tile);

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
void room_destroy(room_t *room);

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
    TCOD_list_t entities;
} map_t;

map_t *map_create(int level);
bool map_is_inside(int x, int y);
room_t *map_get_random_room(map_t *map);
TCOD_map_t map_to_TCOD_map(map_t *map);
TCOD_map_t map_to_fov_map(map_t *map, int x, int y, int radius);
void map_destroy(map_t *map);

/* World */
TCOD_list_t maps;

/* Game */
typedef enum game_status_e {
    STATUS_WAITING,
    STATUS_UPDATE,
    STATUS_QUIT
} game_status_t;

game_status_t game_status;
int turn;
struct entity_s *player;

void game_init(void);
void game_input(void);
void game_update(void);
void game_render(void);
void game_new(void);
void game_reset(void);

/* Message Log */
typedef struct message_s
{
    char *text;
    TCOD_color_t color;
} message_t;

TCOD_console_t msg;
TCOD_list_t messages;

message_t *message_create(char *text, TCOD_color_t color);
void message_destroy(message_t *message);
void msg_log(position_t *position, TCOD_color_t color, char *text, ...);

#endif