#ifndef GAME_H
#define GAME_H

#include <libtcod.h>

typedef struct game_s game_t;

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
    const char *name;
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
#define NUM_MAPS 10
#define MAP_WIDTH 50
#define MAP_HEIGHT 50

typedef struct map_s
{
    game_t *game;
    int level;
    int stair_down_x;
    int stair_down_y;
    int stair_up_x;
    int stair_up_y;
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
    TCOD_list_t rooms;
    TCOD_list_t entities;
} map_t;

void map_init(map_t *map, game_t *game, int level);
bool map_is_inside(int x, int y);
room_t *map_get_random_room(map_t *map);
TCOD_map_t map_to_TCOD_map(map_t *map);
TCOD_map_t map_to_fov_map(map_t *map, int x, int y, int radius);
void map_reset(map_t *map);

/* Spells */
typedef enum spell_type_e {
    SPELL_HEAL_SELF,
    SPELL_INSTAKILL,

    NUM_SPELL_TYPES
} spell_type_t;

typedef struct spell_s
{
    spell_type_t type;
    bool known;
} spell_t;

/* Entities */
#define NUM_ENTITIES 1024
#define ID_UNUSED -1

typedef struct entity_s
{
    int id;
    struct game_s *game;
} entity_t;

void entity_init(entity_t *entity, int id, struct game_s *game);
entity_t *entity_create(struct game_s *game);
void entity_path_towards(entity_t *entity, int x, int y);
void entity_move_towards(entity_t *entity, int x, int y);
void entity_move_random(entity_t *entity);
void entity_move(entity_t *entity, int x, int y);
void entity_swap(entity_t *entity, entity_t *other);
void entity_pick(entity_t *entity, entity_t *other);
void entity_swing(entity_t *entity, int x, int y);
void entity_shoot(entity_t *entity, int x, int y);
void entity_attack(entity_t *entity, entity_t *other);
void entity_cast_spell(entity_t *entity);
void entity_die(entity_t *entity, entity_t *killer);
void entity_destroy(entity_t *entity);
void entity_reset(entity_t *entity);

/* Components */
typedef struct ai_s
{
    float energy;
    float energy_per_turn;
    entity_t *follow_target;
} ai_t;

typedef enum alignment_type_e {
    ALIGNMENT_GOOD,
    ALIGNMENT_EVIL,
} alignment_type_t;

typedef struct alignment_s
{
    alignment_type_t type;
} alignment_t;

typedef enum layer_e {
    LAYER_0,
    LAYER_1,
    LAYER_2,

    NUM_LAYERS,
} layer_t;

typedef struct appearance_s
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
    layer_t layer;
} appearance_t;

typedef struct caster_s
{
    spell_t spells[NUM_SPELL_TYPES];
    spell_type_t current;
} caster_t;

typedef struct flash_s
{
    TCOD_color_t color;
    float fade;
} flash_t;

typedef struct fov_s
{
    int radius;
    TCOD_map_t fov_map;
} fov_t;

typedef struct health_s
{
    int max;
    int current;
} health_t;

typedef enum equipment_slot_e {
    SLOT_HEAD,
    SLOT_CHEST,
    SLOT_LEGS,
    SLOT_FEET,
    SLOT_MHAND,
    SLOT_OHAND,

    NUM_EQUIPMENT_SLOTS
} equipment_slot_t;

typedef struct equipable_s
{
    equipment_slot_t slot;
} equipable_t;

typedef struct inventory_s
{
    TCOD_list_t items;
    entity_t *equipment[NUM_EQUIPMENT_SLOTS];
} inventory_t;

typedef enum light_type_e {
    LIGHT_PRIORITY_0,
    LIGHT_PRIORITY_1,
    LIGHT_PRIORITY_2,

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

typedef struct pickable_s
{
    float weight;
} pickable_t;

typedef struct position_s
{
    int level;
    int x;
    int y;
} position_t;

typedef enum targeting_type_e {
    TARGETING_LOOK,
    TARGETING_SHOOT,
    TARGETING_ZAP
} targeting_type_t;

typedef struct targeting_s
{
    targeting_type_t type;
    int x;
    int y;
} targeting_t;

typedef enum component_type_e {
    COMPONENT_AI,
    COMPONENT_ALIGNMENT,
    COMPONENT_APPEARANCE,
    COMPONENT_CASTER,
    COMPONENT_FLASH,
    COMPONENT_FOV,
    COMPONENT_HEALTH,
    COMPONENT_INVENTORY,
    COMPONENT_LIGHT,
    COMPONENT_OPAQUE,
    COMPONENT_PICKABLE,
    COMPONENT_POSITION,
    COMPONENT_TARGETING,
    COMPONENT_SOLID,

    NUM_COMPONENTS
} component_type_t;

typedef struct component_s
{
    union {
        ai_t ai;
        alignment_t alignment;
        appearance_t appearance;
        caster_t caster;
        flash_t took_damage;
        fov_t fov;
        health_t health;
        inventory_t inventory;
        light_t light;
        pickable_t pickable;
        position_t position;
        targeting_t targeting;
    };
    int id;
    component_type_t type;
} component_t;

void component_init(component_t *component, int id, component_type_t component_type);
component_t *component_add(entity_t *entity, component_type_t component_type);
component_t *component_get(entity_t *entity, component_type_t component_type);
void component_remove(entity_t *entity, component_type_t component_type);
void component_reset(component_t *component);

/* Messages */
typedef struct message_s
{
    char *text;
    TCOD_color_t color;
} message_t;

message_t *message_create(char *text, TCOD_color_t color);
void message_destroy(message_t *message);

/* Game */
typedef struct game_s
{
    tile_common_t tile_common;
    tile_info_t tile_info[NUM_TILES];
    map_t maps[NUM_MAPS];
    entity_t entities[NUM_ENTITIES];
    component_t components[NUM_COMPONENTS][NUM_ENTITIES];
    entity_t *player;
    TCOD_list_t messages;
    int turn;
    bool turn_available;
    bool should_update;
    bool should_restart;
    bool should_quit;
    bool game_over;
    bool message_log_visible;
    bool panel_visible;
} game_t;

void game_init(game_t *game);
void game_new(game_t *game);
void game_save(game_t *game);
void game_load(game_t *game);
void game_input(game_t *game);
void game_update(game_t *game);
void game_render(game_t *game);
void game_log(game_t *game, position_t *position, TCOD_color_t color, char *text, ...);
void game_reset(game_t *game);

#endif