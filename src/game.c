#include <assert.h>
#include <libtcod.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "engine.h"
#include "game.h"
#include "utils.h"

/* Tile Declarations */
typedef enum tile_type_e tile_type_t;
typedef struct tile_common_s tile_common_t;
typedef struct tile_info_s tile_info_t;
typedef struct tile_s tile_t;

/* Room Declarations */
typedef struct room_s room_t;

/* Map Declarations */
#define NUM_MAPS 256
#define MAP_WIDTH 50
#define MAP_HEIGHT 50

typedef struct map_s map_t;

/* Spell Declarations */
typedef enum spell_type_e spell_type_t;
typedef struct spell_s spell_t;

/* Entity Declarations */
#define NUM_ENTITIES 65536
#define ID_UNUSED -1

typedef struct entity_s entity_t;

/* Component Declarations */
typedef struct ai_s ai_t;
typedef enum alignment_type_e alignment_type_t;
typedef struct alignment_s alignment_t;
typedef enum layer_e layer_t;
typedef struct appearance_s appearance_t;
typedef struct caster_s caster_t;
typedef struct flash_s flash_t;
typedef struct fov_s fov_t;
typedef struct health_s health_t;
typedef enum equipment_slot_e equipment_slot_t;
typedef struct equipable_s equipable_t;
typedef struct inventory_s inventory_t;
typedef enum light_type_e light_priority_t;
typedef struct light_s light_t;
typedef struct pickable_s pickable_t;
typedef struct position_s position_t;
typedef enum targeting_type_e targeting_type_t;
typedef struct targeting_s targeting_t;
typedef enum component_type_e component_type_t;
typedef struct component_s component_t;

/* Message Declarations */
typedef struct message_s message_t;

/* Game Declarations */
typedef struct game_s game_t;

/* Tile Definitions */
enum tile_type_e
{
    TILE_FLOOR,
    TILE_WALL,
    TILE_STAIR_DOWN,
    TILE_STAIR_UP,

    NUM_TILES
};

struct tile_common_s
{
    TCOD_color_t shadow_color;
};

struct tile_info_s
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
    bool is_walkable;
    bool is_transparent;
};

struct tile_s
{
    tile_type_t type;
    bool seen;
    TCOD_list_t entities;
};

/* Room Definitions */
struct room_s
{
    int x;
    int y;
    int w;
    int h;
};

/* Map Definitions */
struct map_s
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
};

/* Spell Definitions */
enum spell_type_e
{
    SPELL_HEAL_SELF,
    SPELL_INSTAKILL,

    NUM_SPELL_TYPES
};

struct spell_s
{
    spell_type_t type;
    bool known;
};

/* Entity Definitions */
struct entity_s
{
    int id;
    game_t *game;
};

/* Component Definitions */
struct ai_s
{
    float energy;
    float energy_per_turn;
    entity_t *follow_target;
};

enum alignment_type_e
{
    ALIGNMENT_GOOD,
    ALIGNMENT_EVIL,
};

struct alignment_s
{
    alignment_type_t type;
};

enum layer_e
{
    LAYER_0,
    LAYER_1,
    LAYER_2,

    NUM_LAYERS,
};

struct appearance_s
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
    layer_t layer;
};

struct caster_s
{
    spell_t spells[NUM_SPELL_TYPES];
    spell_type_t current;
};

struct flash_s
{
    TCOD_color_t color;
    float fade;
};

struct fov_s
{
    int radius;
    TCOD_map_t fov_map;
};

struct health_s
{
    int max;
    int current;
};

enum equipment_slot_e
{
    SLOT_HEAD,
    SLOT_CHEST,
    SLOT_LEGS,
    SLOT_FEET,
    SLOT_MHAND,
    SLOT_OHAND,

    NUM_EQUIPMENT_SLOTS
};

struct equipable_s
{
    equipment_slot_t slot;
};

struct inventory_s
{
    TCOD_list_t items;
    entity_t *equipment[NUM_EQUIPMENT_SLOTS];
};

enum light_type_e
{
    LIGHT_PRIORITY_0,
    LIGHT_PRIORITY_1,
    LIGHT_PRIORITY_2,

    NUM_LIGHT_PRIORITIES
};

struct light_s
{
    int radius;
    TCOD_color_t color;
    bool flicker;
    light_priority_t priority;
    TCOD_map_t fov_map;
};

struct pickable_s
{
    float weight;
};

struct position_s
{
    int level;
    int x;
    int y;
};

enum targeting_type_e
{
    TARGETING_LOOK,
    TARGETING_SHOOT,
    TARGETING_ZAP
};

struct targeting_s
{
    targeting_type_t type;
    int x;
    int y;
};

enum component_type_e
{
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
};

struct component_s
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
};

/* Message Definitions */
struct message_s
{
    char *text;
    TCOD_color_t color;
};

/* Game Definitions */
struct game_s
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
    bool should_render;
    bool should_restart;
    bool should_quit;
    bool game_over;
    bool message_log_visible;
    bool panel_visible;
};

/* Tile Function Declarations */
internal void
tile_init(tile_t *tile, tile_type_t type, bool seen);

internal void
tile_reset(tile_t *tile);

/* Room Function Declarations */
internal room_t *
room_create(int x, int y, int w, int h);

internal void
room_get_random_pos(room_t *room, int *x, int *y);

internal void
room_destroy(room_t *room);

/* Map Function Declarations */
internal void
map_init(map_t *map, game_t *game, int level);

internal bool
traverse_node(TCOD_bsp_t *node, map_t *map);

internal void
vline(map_t *map, int x, int y1, int y2);

internal void
vline_up(map_t *map, int x, int y);

internal void
vline_down(map_t *map, int x, int y);

internal void
hline(map_t *map, int x1, int y, int x2);

internal void
hline_left(map_t *map, int x, int y);

internal void
hline_right(map_t *map, int x, int y);

internal bool
map_is_inside(int x, int y);

internal room_t *
map_get_random_room(map_t *map);

internal TCOD_map_t
map_to_TCOD_map(map_t *map);

internal TCOD_map_t
map_to_fov_map(map_t *map, int x, int y, int radius);

internal void
map_reset(map_t *map);

/* Spell Function Declarations */

/* Entity Function Declarations */
internal void
entity_init(entity_t *entity, int id, struct game_s *game);

internal entity_t *
entity_create(struct game_s *game);

internal void
entity_path_towards(entity_t *entity, int x, int y);

internal void
entity_move_towards(entity_t *entity, int x, int y);

internal void
entity_move_random(entity_t *entity);

internal void
entity_move(entity_t *entity, int x, int y);

internal void
entity_swap(entity_t *entity, entity_t *other);

internal void
entity_pick(entity_t *entity, entity_t *other);

internal void
entity_swing(entity_t *entity, int x, int y);

internal void
entity_shoot(entity_t *entity, int x, int y);

internal void
entity_attack(entity_t *entity, entity_t *other);

internal void
entity_cast_spell(entity_t *entity);

internal void
entity_die(entity_t *entity, entity_t *killer);

internal void
entity_destroy(entity_t *entity);

internal void
entity_reset(entity_t *entity);

/* Component Function Declarations */
internal void
component_init(component_t *component, int id, component_type_t component_type);

internal component_t *
component_add(entity_t *entity, component_type_t component_type);

internal component_t *
component_get(entity_t *entity, component_type_t component_type);

internal void
component_remove(entity_t *entity, component_type_t component_type);

internal void
component_reset(component_t *component);

/* Message Function Declarations */
internal message_t *
message_create(char *text, TCOD_color_t color);

internal void
message_destroy(message_t *message);

/* Game Function Declarations */
internal void
game_init(game_t *game);

internal void
game_new(game_t *game);

internal void
game_save(game_t *game);

internal void
game_load(game_t *game);

internal void
game_update(game_t *game);

internal void
game_log(game_t *game, position_t *position, TCOD_color_t color, char *text, ...);

internal void
game_reset(game_t *game);

/* Tile Function Definitions */
internal void
tile_init(tile_t *tile, tile_type_t type, bool seen)
{
    tile->type = type;
    tile->seen = seen;
    tile->entities = TCOD_list_new();
}

internal void
tile_reset(tile_t *tile)
{
    TCOD_list_delete(tile->entities);
}

/* Room Function Definitions */
internal room_t *
room_create(int x, int y, int w, int h)
{
    room_t *room = (room_t *)malloc(sizeof(room_t));

    room->x = x;
    room->y = y;
    room->w = w;
    room->h = h;

    return room;
}

internal void
room_get_random_pos(room_t *room, int *x, int *y)
{
    *x = TCOD_random_get_int(
        NULL,
        room->x,
        room->x + room->w - 1);
    *y = TCOD_random_get_int(
        NULL,
        room->y,
        room->y + room->h - 1);
}

internal void
room_destroy(room_t *room)
{
    free(room);
}

/* Map Function Definitions */
#define BSP_DEPTH 10
#define MIN_ROOM_SIZE 5
#define FULL_ROOMS 1
#define NUM_MONSTERS 50
#define NUM_ADVENTURERS 10
#define NUM_ITEMS 10
#define NUM_BRAZIERS 0

internal void
map_init(map_t *map, game_t *game, int level)
{
    map->game = game;
    map->level = level;
    map->rooms = TCOD_list_new();
    map->entities = TCOD_list_new();

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile_init(tile, TILE_WALL, false);
        }
    }

    TCOD_bsp_t *bsp = TCOD_bsp_new_with_size(0, 0, MAP_WIDTH, MAP_HEIGHT);
    TCOD_bsp_split_recursive(bsp, NULL, BSP_DEPTH, MIN_ROOM_SIZE + 1, MIN_ROOM_SIZE + 1, 1.5f, 1.5f);
    TCOD_bsp_traverse_inverted_level_order(bsp, traverse_node, map);
    TCOD_bsp_delete(bsp);

    room_t *stair_down_room = map_get_random_room(map);
    room_get_random_pos(stair_down_room, &map->stair_down_x, &map->stair_down_y);
    map->tiles[map->stair_down_x][map->stair_down_y].type = TILE_STAIR_DOWN;

    room_t *stair_up_room = map_get_random_room(map);
    room_get_random_pos(stair_up_room, &map->stair_up_x, &map->stair_up_y);
    map->tiles[map->stair_up_x][map->stair_up_y].type = TILE_STAIR_UP;

    for (int i = 0; i < NUM_MONSTERS; i++)
    {
        room_t *room = map_get_random_room(map);

        int x, y;
        room_get_random_pos(room, &x, &y);

        if (room == stair_up_room)
        {
            i--;

            continue;
        }

        entity_t *entity = entity_create(game);
        position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
        position->level = level;
        position->x = x;
        position->y = y;
        TCOD_list_push(map->tiles[position->x][position->y].entities, entity);
        TCOD_list_push(map->entities, entity);
        component_add(entity, COMPONENT_SOLID);
        appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
        appearance->layer = LAYER_1;
        fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
        fov->radius = 5;
        fov->fov_map = NULL;
        ai_t *ai = (ai_t *)component_add(entity, COMPONENT_AI);
        ai->energy = 1.0f;
        ai->follow_target = NULL;
        health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
        alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
        alignment->type = ALIGNMENT_EVIL;

        switch (TCOD_random_get_int(NULL, 0, 3))
        {
        case 0:
        {
            appearance->name = "Skeleton";
            appearance->glyph = 's';
            appearance->color = TCOD_white;
            ai->energy_per_turn = 0.5f;
            health->max = TCOD_random_get_int(NULL, 10, 20);
            health->current = health->max;

            break;
        }
        case 1:
        {
            appearance->name = "Skeleton Captain";
            appearance->glyph = 'S';
            appearance->color = TCOD_white;
            ai->energy_per_turn = 0.75f;
            health->max = TCOD_random_get_int(NULL, 15, 25);
            health->current = health->max;

            break;
        }
        case 2:
        {
            appearance->name = "Zombie";
            appearance->glyph = 'z';
            appearance->color = TCOD_dark_green;
            ai->energy_per_turn = 0.25f;
            health->max = TCOD_random_get_int(NULL, 20, 30);
            health->current = health->max;

            break;
        }
        case 3:
        {
            appearance->name = "Jackal";
            appearance->glyph = 'j';
            appearance->color = TCOD_dark_orange;
            ai->energy_per_turn = 1.5f;
            health->max = TCOD_random_get_int(NULL, 5, 10);
            health->current = health->max;

            break;
        }
        }

        if (TCOD_random_get_int(NULL, 0, 100) == 0)
        {
            light_t *light = (light_t *)component_add(entity, COMPONENT_LIGHT);
            light->radius = 10;
            light->color = TCOD_light_amber;
            light->flicker = true;
            light->priority = LIGHT_PRIORITY_2;
            light->fov_map = NULL;
        }
    }

    for (int i = 0; i < NUM_ADVENTURERS; i++)
    {
        room_t *room = map_get_random_room(map);

        int x, y;
        room_get_random_pos(room, &x, &y);

        if (room == stair_up_room)
        {
            i--;

            continue;
        }

        entity_t *entity = entity_create(game);
        position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
        position->level = level;
        position->x = x;
        position->y = y;
        TCOD_list_push(map->tiles[position->x][position->y].entities, entity);
        TCOD_list_push(map->entities, entity);
        component_add(entity, COMPONENT_SOLID);
        appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
        appearance->name = "Adventurer";
        appearance->glyph = 'a';
        appearance->color = TCOD_azure;
        appearance->layer = LAYER_1;
        fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
        fov->radius = 1;
        fov->fov_map = NULL;
        ai_t *ai = (ai_t *)component_add(entity, COMPONENT_AI);
        ai->energy = 1.0f;
        ai->energy_per_turn = 0.5f;
        ai->follow_target = NULL;
        health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
        health->max = TCOD_random_get_int(NULL, 10, 20);
        health->current = health->max;
        alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
        alignment->type = ALIGNMENT_GOOD;

        if (TCOD_random_get_int(NULL, 0, 100) == 0)
        {
            light_t *light = (light_t *)component_add(entity, COMPONENT_LIGHT);
            light->radius = 10;
            light->color = TCOD_light_amber;
            light->flicker = true;
            light->priority = LIGHT_PRIORITY_2;
            light->fov_map = NULL;
        }
    }

    for (int i = 0; i < NUM_ITEMS; i++)
    {
        room_t *room = map_get_random_room(map);

        int x, y;
        room_get_random_pos(room, &x, &y);

        entity_t *entity = entity_create(game);
        position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
        position->level = level;
        position->x = x;
        position->y = y;
        TCOD_list_push(map->tiles[position->x][position->y].entities, entity);
        TCOD_list_push(map->entities, entity);
        appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
        appearance->name = "Longsword";
        appearance->glyph = '|';
        appearance->color = TCOD_white;
        pickable_t *pickable = (pickable_t *)component_add(entity, COMPONENT_PICKABLE);
        pickable->weight = 10.0f;
    }

    for (int i = 0; i < NUM_BRAZIERS; i++)
    {
        room_t *room = map_get_random_room(map);

        int x, y;
        room_get_random_pos(room, &x, &y);

        entity_t *entity = entity_create(game);
        position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
        position->level = level;
        position->x = x;
        position->y = y;
        TCOD_list_push(map->tiles[position->x][position->y].entities, entity);
        TCOD_list_push(map->entities, entity);
        appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
        appearance->name = "Brazier";
        appearance->glyph = '*';
        appearance->color = TCOD_color_RGB(TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255), TCOD_random_get_int(NULL, 0, 255));
        light_t *light = (light_t *)component_add(entity, COMPONENT_LIGHT);
        light->radius = TCOD_random_get_int(NULL, 10, 20);
        light->color = appearance->color;
        light->flicker = false;
        light->priority = LIGHT_PRIORITY_1;
        light->fov_map = NULL;
        component_add(entity, COMPONENT_SOLID);
        component_add(entity, COMPONENT_OPAQUE);
    }
}

internal bool
traverse_node(TCOD_bsp_t *node, map_t *map)
{
    if (TCOD_bsp_is_leaf(node))
    {
        int min_x = node->x + 1;
        int max_x = node->x + node->w - 1;
        int min_y = node->y + 1;
        int max_y = node->y + node->h - 1;

        if (max_x == MAP_WIDTH - 1)
        {
            max_x--;
        }
        if (max_y == MAP_HEIGHT - 1)
        {
            max_y--;
        }

#if !FULL_ROOMS
        min_x = TCOD_random_get_int(NULL, min_x, max_x - MIN_ROOM_SIZE + 1);
        min_y = TCOD_random_get_int(NULL, min_y, max_y - MIN_ROOM_SIZE + 1);
        max_x = TCOD_random_get_int(NULL, min_x + MIN_ROOM_SIZE - 2, max_x);
        max_y = TCOD_random_get_int(NULL, min_y + MIN_ROOM_SIZE - 2, max_y);
#endif

        node->x = min_x;
        node->y = min_y;
        node->w = max_x - min_x + 1;
        node->h = max_y - min_y + 1;

        for (int x = min_x; x < max_x + 1; x++)
        {
            for (int y = min_y; y < max_y + 1; y++)
            {
                tile_t *tile = &map->tiles[x][y];

                tile->type = TILE_FLOOR;
            }
        }

        room_t *room = room_create(node->x, node->y, node->w, node->h);

        TCOD_list_push(map->rooms, room);
    }
    else
    {
        TCOD_bsp_t *left = TCOD_bsp_left(node);
        TCOD_bsp_t *right = TCOD_bsp_right(node);

        node->x = MIN(left->x, right->x);
        node->y = MIN(left->y, right->y);
        node->w = MAX(left->x + left->w, right->x + right->w) - node->x;
        node->h = MAX(left->y + left->h, right->y + right->h) - node->y;

        if (node->horizontal)
        {
            if (left->x + left->w - 1 < right->x || right->x + right->w - 1 < left->x)
            {
                int x1 = TCOD_random_get_int(NULL, left->x, left->x + left->w - 1);
                int x2 = TCOD_random_get_int(NULL, right->x, right->x + right->w - 1);
                int y = TCOD_random_get_int(NULL, left->y + left->h, right->y);

                vline_up(map, x1, y - 1);
                hline(map, x1, y, x2);
                vline_down(map, x2, y + 1);
            }
            else
            {
                int min_x = MAX(left->x, right->x);
                int max_x = MIN(left->x + left->w - 1, right->x + right->w - 1);
                int x = TCOD_random_get_int(NULL, min_x, max_x);

                while (x > MAP_WIDTH - 1)
                {
                    x--;
                }

                vline_down(map, x, right->y);
                vline_up(map, x, right->y - 1);
            }
        }
        else
        {
            if (left->y + left->h - 1 < right->y || right->y + right->h - 1 < left->y)
            {
                int y1 = TCOD_random_get_int(NULL, left->y, left->y + left->h - 1);
                int y2 = TCOD_random_get_int(NULL, right->y, right->y + right->h - 1);
                int x = TCOD_random_get_int(NULL, left->x + left->w, right->x);

                hline_left(map, x - 1, y1);
                vline(map, x, y1, y2);
                hline_right(map, x + 1, y2);
            }
            else
            {
                int min_y = MAX(left->y, right->y);
                int max_y = MIN(left->y + left->h - 1, right->y + right->h - 1);
                int y = TCOD_random_get_int(NULL, min_y, max_y);

                while (y > MAP_HEIGHT - 1)
                {
                    y--;
                }

                hline_left(map, right->x - 1, y);
                hline_right(map, right->x, y);
            }
        }
    }

    return true;
}

internal void
vline(map_t *map, int x, int y1, int y2)
{
    if (y1 > y2)
    {
        int t = y1;
        y1 = y2;
        y2 = t;
    }

    for (int y = y1; y < y2 + 1; y++)
    {
        tile_t *tile = &map->tiles[x][y];

        tile->type = TILE_FLOOR;
    }
}

internal void
vline_up(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y >= 0 && tile->type != TILE_FLOOR)
    {
        tile->type = TILE_FLOOR;

        y--;
    }
}

internal void
vline_down(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y < MAP_HEIGHT && tile->type != TILE_FLOOR)
    {
        tile->type = TILE_FLOOR;

        y++;
    }
}

internal void
hline(map_t *map, int x1, int y, int x2)
{
    if (x1 > x2)
    {
        int t = x1;
        x1 = x2;
        x2 = t;
    }

    for (int x = x1; x < x2 + 1; x++)
    {
        tile_t *tile = &map->tiles[x][y];

        tile->type = TILE_FLOOR;
    }
}

internal void
hline_left(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (x >= 0 && tile->type != TILE_FLOOR)
    {
        tile->type = TILE_FLOOR;

        x--;
    }
}

internal void
hline_right(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (x < MAP_WIDTH && tile->type != TILE_FLOOR)
    {
        tile->type = TILE_FLOOR;

        x++;
    }
}

internal bool
map_is_inside(int x, int y)
{
    return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
}

internal room_t *
map_get_random_room(map_t *map)
{
    return TCOD_list_get(map->rooms, TCOD_random_get_int(NULL, 0, TCOD_list_size(map->rooms) - 1));
}

internal TCOD_map_t
map_to_TCOD_map(map_t *map)
{
    TCOD_map_t TCOD_map = TCOD_map_new(MAP_WIDTH, MAP_HEIGHT);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            bool is_walkable = map->game->tile_info[tile->type].is_walkable;
            bool is_transparent = map->game->tile_info[tile->type].is_transparent;

            for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
            {
                entity_t *entity = *iterator;

                component_t *opaque = component_get(entity, COMPONENT_OPAQUE);

                if (opaque != NULL)
                {
                    is_transparent = false;
                }

                component_t *solid = component_get(entity, COMPONENT_SOLID);

                if (solid != NULL)
                {
                    is_walkable = false;
                }
            }

            TCOD_map_set_properties(TCOD_map, x, y, is_transparent, is_walkable);
        }
    }

    return TCOD_map;
}

internal TCOD_map_t
map_to_fov_map(map_t *map, int x, int y, int radius)
{
    TCOD_map_t fov_map = map_to_TCOD_map(map);

    TCOD_map_compute_fov(fov_map, x, y, radius, true, FOV_BASIC);

    return fov_map;
}

internal void
map_reset(map_t *map)
{
    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile_reset(tile);
        }
    }

    for (void **iterator = TCOD_list_begin(map->rooms); iterator != TCOD_list_end(map->rooms); iterator++)
    {
        room_t *room = *iterator;

        room_destroy(room);
    }

    TCOD_list_delete(map->rooms);

    TCOD_list_delete(map->entities);
}

/* Spell Function Definitions */

/* Entity Function Definitions */
internal void
entity_init(entity_t *entity, int id, game_t *game)
{
    entity->id = id;
    entity->game = game;
}

internal entity_t *
entity_create(game_t *game)
{
    entity_t *entity = NULL;

    for (int id = 0; id < NUM_ENTITIES; id++)
    {
        entity_t *current = &game->entities[id];

        if (current->id == ID_UNUSED)
        {
            entity_init(current, id, game);

            entity = current;

            break;
        }
    }

    assert(entity);

    return entity;
}

internal void
entity_path_towards(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        TCOD_map_t TCOD_map = map_to_TCOD_map(&entity->game->maps[position->level]);
        TCOD_map_set_properties(TCOD_map, x, y, TCOD_map_is_transparent(TCOD_map, x, y), true);

        TCOD_path_t path = TCOD_path_new_using_map(TCOD_map, 1.0f);
        TCOD_path_compute(path, position->x, position->y, x, y);

        {
            int next_x, next_y;
            if (!TCOD_path_is_empty(path) && TCOD_path_walk(path, &next_x, &next_y, false))
            {
                entity_move(entity, next_x, next_y);
            }
            else
            {
                entity_move_towards(entity, x, y);
            }
        }

        TCOD_path_delete(path);

        TCOD_map_delete(TCOD_map);
    }
}

internal void
entity_move_towards(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        int dx = x - position->x;
        int dy = y - position->y;
        float d = distance(position->x, position->y, x, y);

        if (d > 0)
        {
            dx = round(dx / d);
            dy = round(dy / d);

            entity_move(entity, position->x + dx, position->y + dy);
        }
    }
}

internal void
entity_move_random(entity_t *entity)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        int x = position->x + TCOD_random_get_int(NULL, -1, 1);
        int y = position->y + TCOD_random_get_int(NULL, -1, 1);

        entity_move(entity, x, y);
    }
}

internal void
entity_move(entity_t *entity, int x, int y)
{
    if (map_is_inside(x, y))
    {
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

        if (position)
        {
            map_t *map = &entity->game->maps[position->level];
            tile_t *tile = &map->tiles[position->x][position->y];
            tile_t *next_tile = &map->tiles[x][y];
            tile_info_t *next_tile_info = &entity->game->tile_info[next_tile->type];

            bool can_move = true;

            if (!next_tile_info->is_walkable)
            {
                can_move = false;
            }

            for (void **iterator = TCOD_list_begin(next_tile->entities); iterator != TCOD_list_end(next_tile->entities); iterator++)
            {
                entity_t *other = *iterator;

                component_t *other_solid = component_get(other, COMPONENT_SOLID);

                if (other_solid)
                {
                    can_move = false;

                    {
                        health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);

                        if (other_health)
                        {
                            alignment_t *alignment = (alignment_t *)component_get(entity, COMPONENT_ALIGNMENT);

                            alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);

                            // TODO: what if there are multiple entities on this tile?
                            if (alignment && other_alignment &&
                                alignment->type == other_alignment->type)
                            {
                                // TODO: only the player can swap?
                                if (entity == entity->game->player)
                                {
                                    entity_swap(entity, other);
                                }
                            }
                            else
                            {
                                entity_attack(entity, other);
                            }
                        }
                    }
                }
            }

            if (can_move)
            {
                position->x = x;
                position->y = y;

                TCOD_list_remove(tile->entities, entity);
                TCOD_list_push(next_tile->entities, entity);
            }
        }
    }
}

internal void
entity_swap(entity_t *entity, entity_t *other)
{
    if (entity != other)
    {
        position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

        position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

        if (position && other_position)
        {
            {
                map_t *map = &entity->game->maps[position->level];
                tile_t *tile = &map->tiles[position->x][position->y];

                map_t *other_map = &other->game->maps[other_position->level];
                tile_t *other_tile = &other_map->tiles[other_position->x][other_position->y];

                TCOD_list_remove(tile->entities, entity);
                TCOD_list_push(other_tile->entities, entity);

                TCOD_list_remove(other_tile->entities, other);
                TCOD_list_push(tile->entities, other);
            }

            {
                int x = position->x;
                int y = position->y;

                position->x = other_position->x;
                position->y = other_position->y;

                other_position->x = x;
                other_position->y = y;
            }

            {
                appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

                appearance_t *other_appearance = (appearance_t *)component_get(other, COMPONENT_APPEARANCE);

                if (appearance && other_appearance)
                {
                    game_log(entity->game, position, TCOD_white, "%s swaps with %s", appearance->name, other_appearance->name);
                }
            }
        }
    }
}

internal void
entity_pick(entity_t *entity, entity_t *other)
{
    inventory_t *inventory = (inventory_t *)component_get(entity, COMPONENT_INVENTORY);
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    pickable_t *other_pickable = (pickable_t *)component_get(other, COMPONENT_PICKABLE);
    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

    if (inventory && position && other_pickable && other_position)
    {
        {
            map_t *other_map = &other->game->maps[other_position->level];
            tile_t *other_tile = &other_map->tiles[other_position->x][other_position->y];

            TCOD_list_remove(other_tile->entities, other);
            TCOD_list_push(inventory->items, other);
        }

        component_remove(other, COMPONENT_POSITION);

        {
            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

            appearance_t *other_appearance = (appearance_t *)component_get(other, COMPONENT_APPEARANCE);

            if (appearance && other_appearance)
            {
                game_log(entity->game, position, TCOD_white, "%s picks up %s", appearance->name, other_appearance->name);
            }
        }
    }
}

internal void
entity_swing(entity_t *entity, int x, int y)
{
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (position)
    {
        map_t *map = &entity->game->maps[position->level];
        tile_t *other_tile = &map->tiles[x][y];

        bool hit = false;

        for (void **iterator = TCOD_list_begin(other_tile->entities); iterator != TCOD_list_end(other_tile->entities); iterator++)
        {
            entity_t *other = *iterator;

            health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);

            if (other_health)
            {
                hit = true;

                entity_attack(entity, other);
            }
        }

        if (!hit)
        {
            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

            if (appearance)
            {
                game_log(entity->game, position, TCOD_white, "%s swings at the air", appearance->name);
            }
        }
    }
}

internal void
entity_shoot(entity_t *entity, int x, int y)
{
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (appearance && position)
    {
        if (position->x == x && position->y == y)
        {
            game_log(entity->game, position, TCOD_white, "%s thinks that's a bad idea!", appearance->name);
        }
        else
        {
            game_log(entity->game, position, TCOD_white, "%s shoots", appearance->name);

            tile_t *other_tile = &entity->game->maps[position->level].tiles[x][y];

            bool hit = false;

            for (void **iterator = TCOD_list_begin(other_tile->entities); iterator != TCOD_list_end(other_tile->entities); iterator++)
            {
                entity_t *other = *iterator;

                health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);

                if (other_health)
                {
                    hit = true;

                    entity_attack(entity, other);
                }
            }

            if (!hit)
            {
                game_log(entity->game, position, TCOD_white, "%s's arrow flies through the air", appearance->name);
            }
        }
    }
}

internal void
entity_attack(entity_t *entity, entity_t *other)
{
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    appearance_t *other_appearance = (appearance_t *)component_get(other, COMPONENT_APPEARANCE);
    health_t *other_health = (health_t *)component_get(other, COMPONENT_HEALTH);
    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

    if (appearance && position && other_appearance && other_health && other_position)
    {
        int attack_roll = roll(1, 20);
        int attack_bonus = 1;
        int total_attack = attack_roll + attack_bonus;
        int other_armor_class = 5;
        bool hit = attack_roll == 1
                       ? false
                       : attack_roll == 20
                             ? true
                             : total_attack >= other_armor_class;

        if (hit)
        {
            // 1d8 19-20x2
            int weapon_a = 1;
            int weapon_x = 8;
            int weapon_threat_range = 19;
            int weapon_crit_multiplier = 2;

            int damage_rolls = 1;

            bool crit = false;
            if (attack_roll >= weapon_threat_range)
            {
                int threat_roll = roll(1, 20);
                int total_threat = threat_roll + attack_bonus;

                if (total_threat >= other_armor_class)
                {
                    crit = true;
                    damage_rolls *= weapon_crit_multiplier;
                }
            }

            int total_damage = 0;
            int damage_bonus = 0;
            for (int i = 0; i < damage_rolls; i++)
            {
                int damage_roll = roll(weapon_a, weapon_x);
                int damage = damage_roll + damage_bonus;

                total_damage += damage;
            }

            game_log(entity->game, position, crit ? TCOD_yellow : TCOD_white, "%s %s %s for %d", appearance->name, crit ? "crits" : "hits", other_appearance->name, total_damage);

            other_health->current -= total_damage;

            flash_t *other_flash = (flash_t *)component_add(other, COMPONENT_FLASH);
            other_flash->color = TCOD_red;
            other_flash->fade = 1.0f;

            if (other_health->current <= 0)
            {
                entity_die(other, entity);
            }
        }
        else
        {
            game_log(entity->game, position, TCOD_white, "%s misses", appearance->name);
        }
    }
}

internal void
entity_cast_spell(entity_t *entity)
{
    caster_t *caster = (caster_t *)component_get(entity, COMPONENT_CASTER);

    if (caster)
    {
        spell_t *spell = &caster->spells[caster->current];

        switch (spell->type)
        {
        case SPELL_HEAL_SELF:
        {
            health_t *health = (health_t *)component_get(entity, COMPONENT_HEALTH);

            if (health)
            {
                int heal_amount = health->max - health->current;

                health->current += heal_amount;

                flash_t *flash = (flash_t *)component_add(entity, COMPONENT_FLASH);
                flash->color = TCOD_green;
                flash->fade = 1.0f;

                appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
                position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

                if (appearance != NULL && position != NULL)
                {
                    game_log(entity->game, position, TCOD_purple, "%s casts Heal Self, restoring %d health", appearance->name, heal_amount);
                }
            }
        }
        break;
        case SPELL_INSTAKILL:
        {
            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
            fov_t *fov = (fov_t *)component_get(entity, COMPONENT_FOV);
            position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);
            targeting_t *targeting = (targeting_t *)component_get(entity, COMPONENT_TARGETING);

            if (appearance && fov && targeting && position)
            {
                tile_t *tile = &entity->game->maps[position->level].tiles[targeting->x][targeting->y];

                entity_t *target = NULL;

                for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
                {
                    entity_t *other = *iterator;

                    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                    if (other_position)
                    {
                        if (TCOD_map_is_in_fov(fov->fov_map, other_position->x, other_position->y))
                        {
                            target = other;
                        }
                    }
                }

                if (target)
                {
                    if (target == entity)
                    {
                        game_log(entity->game, position, TCOD_white, "%s thinks that's a bad idea!", appearance->name);
                    }
                    else
                    {
                        entity_die(target, entity);
                    }
                }
                else
                {
                    game_log(entity->game, position, TCOD_purple, "%s casts Instakill", appearance->name);
                }
            }
        }
        break;
        }
    }
}

internal void
entity_die(entity_t *entity, entity_t *killer)
{
    appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);
    position_t *position = (position_t *)component_get(entity, COMPONENT_POSITION);

    if (appearance && position)
    {
        game_log(entity->game, position, TCOD_red, "%s dies", appearance->name);

        appearance->glyph = '%';
        appearance->layer = LAYER_0;

        component_remove(entity, COMPONENT_ALIGNMENT);
        component_remove(entity, COMPONENT_AI);
        component_remove(entity, COMPONENT_HEALTH);
        component_remove(entity, COMPONENT_SOLID);

        if (killer)
        {
            appearance_t *killer_appearance = (appearance_t *)component_get(killer, COMPONENT_APPEARANCE);
            position_t *killer_position = (position_t *)component_get(killer, COMPONENT_POSITION);

            if (killer_appearance && killer_position)
            {
                game_log(killer->game, killer_position, TCOD_azure, "%s gains %d experience", killer_appearance->name, TCOD_random_get_int(NULL, 50, 100));
            }
        }

        if (entity == entity->game->player)
        {
            entity->game->game_over = true;

            TCOD_sys_delete_file("../saves/save.gz");

            game_log(entity->game, position, TCOD_green, "Game over! Press 'r' to restart");
        }
    }
}

internal void
entity_destroy(entity_t *entity)
{
    if (entity && entity->id != ID_UNUSED)
    {
        for (component_type_t component_type = 0; component_type < NUM_COMPONENTS; component_type++)
        {
            component_remove(entity, component_type);
        }

        entity_reset(entity);
    }
}

internal void
entity_reset(entity_t *entity)
{
    entity->id = ID_UNUSED;
    entity->game = NULL;
}

/* Component Function Definitions */
internal void
component_init(component_t *component, int id, component_type_t component_type)
{
    component->id = id;
    component->type = component_type;

    switch (component->type)
    {
    case COMPONENT_AI:
    {
        ai_t *ai = (ai_t *)component;

        ai->energy = 0.0f;
        ai->energy_per_turn = 0.0f;
        ai->follow_target = NULL;
    }
    break;
    case COMPONENT_ALIGNMENT:
    {
        alignment_t *alignment = (alignment_t *)component;

        alignment->type = 0;
    }
    break;
    case COMPONENT_APPEARANCE:
    {
        appearance_t *appearance = (appearance_t *)component;

        appearance->name = NULL;
        appearance->glyph = ' ';
        appearance->color = TCOD_white;
        appearance->layer = 0;
    }
    break;
    case COMPONENT_CASTER:
    {
        caster_t *caster = (caster_t *)component;

        for (spell_type_t spell_type = 0; spell_type < NUM_SPELL_TYPES; spell_type++)
        {
            // TODO: why
            caster->spells[spell_type].type = spell_type;
            caster->spells[spell_type].known = false;
        }
        caster->current = 0;
    }
    break;
    case COMPONENT_FLASH:
    {
        flash_t *flash = (flash_t *)component;

        flash->color = TCOD_white;
        flash->fade = 0.0f;
    }
    break;
    case COMPONENT_FOV:
    {
        fov_t *fov = (fov_t *)component;

        fov->radius = 0;
        fov->fov_map = NULL;
    }
    break;
    case COMPONENT_HEALTH:
    {
        health_t *health = (health_t *)component;

        health->max = 0;
        health->current = 0;
    }
    break;
    case COMPONENT_INVENTORY:
    {
        inventory_t *inventory = (inventory_t *)component;

        inventory->items = TCOD_list_new();
        for (equipment_slot_t equipment_slot = 0; equipment_slot < NUM_EQUIPMENT_SLOTS; equipment_slot++)
        {
            inventory->equipment[equipment_slot] = NULL;
        }
    }
    break;
    case COMPONENT_LIGHT:
    {
        light_t *light = (light_t *)component;

        light->radius = 0;
        light->color = TCOD_white;
        light->flicker = false;
        light->priority = 0;
        light->fov_map = NULL;
    }
    break;
    case COMPONENT_OPAQUE:
    {
    }
    break;
    case COMPONENT_PICKABLE:
    {
        pickable_t *pickable = (pickable_t *)component;

        pickable->weight = 0.0f;
    }
    break;
    case COMPONENT_POSITION:
    {
        position_t *position = (position_t *)component;

        position->level = 0;
        position->x = 0;
        position->y = 0;
    }
    break;
    case COMPONENT_TARGETING:
    {
        targeting_t *targeting = (targeting_t *)component;

        targeting->type = 0;
        targeting->x = 0;
        targeting->y = 0;
    }
    break;
    case COMPONENT_SOLID:
    {
    }
    break;
    }
}

internal component_t *
component_add(entity_t *entity, component_type_t component_type)
{
    component_t *component = NULL;

    if (entity && entity->id != ID_UNUSED)
    {
        component = &entity->game->components[component_type][entity->id];

        if (component->id != ID_UNUSED)
        {
            component_remove(entity, component_type);
        }

        component_init(component, entity->id, component_type);
    }

    return component;
}

internal component_t *
component_get(entity_t *entity, component_type_t component_type)
{
    component_t *component = NULL;

    if (entity && entity->id != ID_UNUSED)
    {
        component = &entity->game->components[component_type][entity->id];

        if (component->id == ID_UNUSED)
        {
            component = NULL;
        }
    }

    return component;
}

internal void
component_remove(entity_t *entity, component_type_t component_type)
{
    if (entity && entity->id != ID_UNUSED)
    {
        component_t *component = &entity->game->components[component_type][entity->id];

        component_reset(component);
    }
}

internal void
component_reset(component_t *component)
{
    component->id = ID_UNUSED;

    switch (component->type)
    {
    case COMPONENT_AI:
    {
        ai_t *ai = (ai_t *)component;
    }
    break;
    case COMPONENT_ALIGNMENT:
    {
        alignment_t *alignment = (alignment_t *)component;

        break;
    }
    case COMPONENT_APPEARANCE:
    {
        appearance_t *appearance = (appearance_t *)component;
    }
    break;
    case COMPONENT_FLASH:
    {
        flash_t *flash = (flash_t *)component;
    }
    break;
    case COMPONENT_FOV:
    {
        fov_t *fov = (fov_t *)component;

        if (fov->fov_map != NULL)
        {
            TCOD_map_delete(fov->fov_map);
        }
    }
    break;
    case COMPONENT_HEALTH:
    {
        health_t *health = (health_t *)component;
    }
    break;
    case COMPONENT_INVENTORY:
    {
        inventory_t *inventory = (inventory_t *)component;

        if (inventory->items != NULL)
        {
            TCOD_list_delete(inventory->items);
        }
    }
    break;
    case COMPONENT_LIGHT:
    {
        light_t *light = (light_t *)component;

        if (light->fov_map != NULL)
        {
            TCOD_map_delete(light->fov_map);
        }
    }
    break;
    case COMPONENT_OPAQUE:
    {
    }
    break;
    case COMPONENT_PICKABLE:
    {
        pickable_t *pickable = (pickable_t *)component;
    }
    break;
    case COMPONENT_POSITION:
    {
        position_t *position = (position_t *)component;
    }
    break;
    case COMPONENT_TARGETING:
    {
        targeting_t *targeting = (targeting_t *)component;
    }
    break;
    case COMPONENT_SOLID:
    {
    }
    break;
    }
}

/* Message Function Definitions */
internal message_t *
message_create(char *text, TCOD_color_t color)
{
    message_t *message = (message_t *)malloc(sizeof(message_t));

    message->text = strdup(text);
    message->color = color;

    return message;
}

internal void
message_destroy(message_t *message)
{
    free(message->text);

    free(message);
}

/* Game Function Definitions */
extern void
game_run(void)
{
    local game_t game;

    game_init(&game);

    if (TCOD_sys_file_exists("../saves/save.gz"))
    {
        game_load(&game);
    }
    else
    {
        game_new(&game);
    }

    while (!TCOD_console_is_window_closed())
    {
        game_update(&game);

        if (game.should_restart)
        {
            game_reset(&game);
            game_init(&game);
            game_new(&game);
        }

        if (game.should_quit)
        {
            game_reset(&game);

            break;
        }
    }
}

internal void
game_init(game_t *game)
{
    for (int id = 0; id < NUM_ENTITIES; id++)
    {
        entity_t *entity = &game->entities[id];

        entity_init(entity, ID_UNUSED, game);

        for (component_type_t component_type = 0; component_type < NUM_COMPONENTS; component_type++)
        {
            component_t *component = &game->components[component_type][id];

            component_init(component, ID_UNUSED, component_type);
        }
    }

    for (int level = 0; level < NUM_MAPS; level++)
    {
        map_t *map = &game->maps[level];

        map_init(map, game, level);
    }

    game->tile_common = (tile_common_t){
        .shadow_color = TCOD_color_RGB(16, 16, 32)};

    game->tile_info[TILE_FLOOR] = (tile_info_t){
        .name = "Floor",
        .glyph = '.',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};
    game->tile_info[TILE_WALL] = (tile_info_t){
        .name = "Wall",
        .glyph = '#',
        .color = TCOD_white,
        .is_transparent = false,
        .is_walkable = false};
    game->tile_info[TILE_STAIR_DOWN] = (tile_info_t){
        .name = "Stair Down",
        .glyph = '>',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};
    game->tile_info[TILE_STAIR_UP] = (tile_info_t){
        .name = "Stair Up",
        .glyph = '<',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};

    game->player = NULL;

    game->turn = 0;
    game->turn_available = true;
    game->should_update = true;
    game->should_render = true;
    game->should_restart = false;
    game->should_quit = false;
    game->game_over = false;

    game->messages = TCOD_list_new();

    game->message_log_visible = true;
    game->panel_visible = false;
}

internal void
game_new(game_t *game)
{
    TCOD_sys_delete_file("../saves/save.gz");

    {
        entity_t *entity = entity_create(game);
        game->player = entity;

        alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
        alignment->type = ALIGNMENT_GOOD;

        appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
        appearance->name = "Blinky";
        appearance->glyph = '@';
        appearance->color = TCOD_white;
        appearance->layer = LAYER_1;

        component_add(entity, COMPONENT_CASTER);

        fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
        fov->radius = 1;

        health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
        health->max = 20;
        health->current = health->max;

        component_add(entity, COMPONENT_INVENTORY);

        light_t *light = (light_t *)component_add(entity, COMPONENT_LIGHT);
        light->radius = 5;
        light->color = TCOD_white;
        light->flicker = false;
        light->priority = LIGHT_PRIORITY_0;

        position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
        position->level = 0;
        map_t *map = &game->maps[position->level];
        TCOD_list_push(map->entities, entity);
        position->x = map->stair_up_x;
        position->y = map->stair_up_y;
        tile_t *tile = &map->tiles[position->x][position->y];
        TCOD_list_push(tile->entities, entity);

        component_add(entity, COMPONENT_SOLID);
    }

    {
        entity_t *entity = entity_create(game);

        ai_t *ai = (ai_t *)component_add(entity, COMPONENT_AI);
        ai->energy = 1.0f;
        ai->energy_per_turn = 0.5f;
        ai->follow_target = game->player;

        alignment_t *alignment = (alignment_t *)component_add(entity, COMPONENT_ALIGNMENT);
        alignment->type = ALIGNMENT_GOOD;

        appearance_t *appearance = (appearance_t *)component_add(entity, COMPONENT_APPEARANCE);
        appearance->name = "Spot";
        appearance->glyph = 'd';
        appearance->color = TCOD_white;
        appearance->layer = LAYER_1;

        fov_t *fov = (fov_t *)component_add(entity, COMPONENT_FOV);
        fov->radius = 1;

        health_t *health = (health_t *)component_add(entity, COMPONENT_HEALTH);
        health->max = 20;
        health->current = health->max;

        light_t *light = (light_t *)component_add(entity, COMPONENT_LIGHT);
        light->radius = 5;
        light->color = TCOD_white;
        light->flicker = false;
        light->priority = LIGHT_PRIORITY_0;

        position_t *position = (position_t *)component_add(entity, COMPONENT_POSITION);
        position->level = 0;
        map_t *map = &game->maps[position->level];
        TCOD_list_push(map->entities, entity);
        position->x = map->stair_up_x + 1;
        position->y = map->stair_up_y;
        tile_t *tile = &map->tiles[position->x][position->y];
        TCOD_list_push(tile->entities, entity);

        component_add(entity, COMPONENT_SOLID);
    }

    {
        appearance_t *player_appearance = (appearance_t *)component_get(game->player, COMPONENT_APPEARANCE);

        if (player_appearance)
        {
            game_log(game, NULL, TCOD_white, "Hail, %s!", player_appearance->name);
        }
    }
}

internal void
game_save(game_t *game)
{
    TCOD_zip_t zip = TCOD_zip_new();

    for (int level = 0; level < NUM_MAPS; level++)
    {
        map_t *map = &game->maps[level];

        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                tile_t *tile = &map->tiles[x][y];

                TCOD_zip_put_int(zip, tile->type);
                TCOD_zip_put_int(zip, tile->seen);
            }
        }
    }

    for (int id = 0; id < NUM_ENTITIES; id++)
    {
        entity_t *entity = &game->entities[id];

        TCOD_zip_put_int(zip, entity->id);

        for (component_type_t component_type = 0; component_type < NUM_COMPONENTS; component_type++)
        {
            component_t *component = &game->components[component_type][id];

            TCOD_zip_put_int(zip, component->id);

            switch (component->type)
            {
            case COMPONENT_AI:
            {
                ai_t *ai = (ai_t *)component;

                TCOD_zip_put_float(zip, ai->energy);
                TCOD_zip_put_float(zip, ai->energy_per_turn);
                int follow_target_id = ID_UNUSED;
                if (ai->follow_target != NULL)
                {
                    follow_target_id = ai->follow_target->id;
                }
                TCOD_zip_put_int(zip, follow_target_id);
            }
            break;
            case COMPONENT_ALIGNMENT:
            {
                alignment_t *alignment = (alignment_t *)component;

                TCOD_zip_put_int(zip, alignment->type);
            }
            break;
            case COMPONENT_APPEARANCE:
            {
                appearance_t *appearance = (appearance_t *)component;

                TCOD_zip_put_string(zip, appearance->name);
                TCOD_zip_put_char(zip, appearance->glyph);
                TCOD_zip_put_color(zip, appearance->color);
                TCOD_zip_put_int(zip, appearance->layer);
            }
            break;
            case COMPONENT_CASTER:
            {
                caster_t *caster = (caster_t *)component;

                for (spell_type_t spell_type = 0; spell_type < NUM_SPELL_TYPES; spell_type++)
                {
                    TCOD_zip_put_int(zip, caster->spells[spell_type].type);
                    TCOD_zip_put_int(zip, caster->spells[spell_type].known);
                }
                TCOD_zip_put_int(zip, caster->current);
            }
            break;
            case COMPONENT_FLASH:
            {
                flash_t *flash = (flash_t *)component;

                TCOD_zip_put_color(zip, flash->color);
                TCOD_zip_put_float(zip, flash->fade);
            }
            break;
            case COMPONENT_FOV:
            {
                fov_t *fov = (fov_t *)component;

                TCOD_zip_put_int(zip, fov->radius);
            }
            break;
            case COMPONENT_HEALTH:
            {
                health_t *health = (health_t *)component;

                TCOD_zip_put_int(zip, health->max);
                TCOD_zip_put_int(zip, health->current);
            }
            break;
            case COMPONENT_INVENTORY:
            {
                inventory_t *inventory = (inventory_t *)component;
            }
            break;
            case COMPONENT_LIGHT:
            {
                light_t *light = (light_t *)component;

                TCOD_zip_put_int(zip, light->radius);
                TCOD_zip_put_color(zip, light->color);
                TCOD_zip_put_int(zip, light->flicker);
                TCOD_zip_put_int(zip, light->priority);
            }
            break;
            case COMPONENT_OPAQUE:
            {
            }
            break;
            case COMPONENT_PICKABLE:
            {
                pickable_t *pickable = (pickable_t *)component;

                TCOD_zip_put_float(zip, pickable->weight);
            }
            break;
            case COMPONENT_POSITION:
            {
                position_t *position = (position_t *)component;

                TCOD_zip_put_int(zip, position->level);
                TCOD_zip_put_int(zip, position->x);
                TCOD_zip_put_int(zip, position->y);
            }
            break;
            case COMPONENT_TARGETING:
            {
                targeting_t *targeting = (targeting_t *)component;

                TCOD_zip_put_int(zip, targeting->type);
                TCOD_zip_put_int(zip, targeting->x);
                TCOD_zip_put_int(zip, targeting->y);
            }
            break;
            case COMPONENT_SOLID:
            {
            }
            break;
            }
        }
    }

    TCOD_zip_put_int(zip, game->player->id);
    TCOD_zip_put_int(zip, game->turn);

    // int num_messages = TCOD_list_size(game->messages);
    // int num_bytes = sizeof(message_t) * num_messages;
    // message_t *messages = malloc(num_bytes);
    // int i = 0;

    // for (void **iterator = TCOD_list_begin(game->messages); iterator != TCOD_list_end(game->messages); iterator++)
    // {
    //     message_t *message = *iterator;

    //     messages[i] = *message;

    //     i++;
    // }

    // TCOD_zip_put_int(zip, num_messages);
    // TCOD_zip_put_data(zip, num_bytes, messages);

    TCOD_zip_save_to_file(zip, "../saves/save.gz");

    TCOD_zip_delete(zip);

    game_log(game, NULL, TCOD_green, "Game saved!");
}

internal void
game_load(game_t *game)
{
    game_reset(game);
    game_init(game);

    TCOD_zip_t zip = TCOD_zip_new();

    TCOD_zip_load_from_file(zip, "../saves/save.gz");

    for (int level = 0; level < NUM_MAPS; level++)
    {
        map_t *map = &game->maps[level];

        for (int x = 0; x < MAP_WIDTH; x++)
        {
            for (int y = 0; y < MAP_HEIGHT; y++)
            {
                tile_t *tile = &map->tiles[x][y];

                tile->type = TCOD_zip_get_int(zip);
                tile->seen = TCOD_zip_get_int(zip);
            }
        }
    }

    for (int id = 0; id < NUM_ENTITIES; id++)
    {
        entity_t *entity = &game->entities[id];

        entity->id = TCOD_zip_get_int(zip);

        for (component_type_t component_type = 0; component_type < NUM_COMPONENTS; component_type++)
        {
            component_t *component = &game->components[component_type][id];

            component->id = TCOD_zip_get_int(zip);

            switch (component->type)
            {
            case COMPONENT_AI:
            {
                ai_t *ai = (ai_t *)component;

                ai->energy = TCOD_zip_get_float(zip);
                ai->energy_per_turn = TCOD_zip_get_float(zip);
                int follow_target_id = TCOD_zip_get_int(zip);
                if (follow_target_id != ID_UNUSED)
                {
                    ai->follow_target = &game->entities[follow_target_id];
                }
            }
            break;
            case COMPONENT_ALIGNMENT:
            {
                alignment_t *alignment = (alignment_t *)component;

                alignment->type = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_APPEARANCE:
            {
                appearance_t *appearance = (appearance_t *)component;

                appearance->name = TCOD_zip_get_string(zip);
                appearance->glyph = TCOD_zip_get_char(zip);
                appearance->color = TCOD_zip_get_color(zip);
                appearance->layer = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_CASTER:
            {
                caster_t *caster = (caster_t *)component;

                for (spell_type_t spell_type = 0; spell_type < NUM_SPELL_TYPES; spell_type++)
                {
                    caster->spells[spell_type].type = TCOD_zip_get_int(zip);
                    caster->spells[spell_type].known = TCOD_zip_get_int(zip);
                }
                caster->current = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_FLASH:
            {
                flash_t *flash = (flash_t *)component;

                flash->color = TCOD_zip_get_color(zip);
                flash->fade = TCOD_zip_get_float(zip);
            }
            break;
            case COMPONENT_FOV:
            {
                fov_t *fov = (fov_t *)component;

                fov->radius = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_HEALTH:
            {
                health_t *health = (health_t *)component;

                health->max = TCOD_zip_get_int(zip);
                health->current = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_INVENTORY:
            {
                inventory_t *inventory = (inventory_t *)component;
            }
            break;
            case COMPONENT_LIGHT:
            {
                light_t *light = (light_t *)component;

                light->radius = TCOD_zip_get_int(zip);
                light->color = TCOD_zip_get_color(zip);
                light->flicker = TCOD_zip_get_int(zip);
                light->priority = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_OPAQUE:
            {
            }
            break;
            case COMPONENT_PICKABLE:
            {
                pickable_t *pickable = (pickable_t *)component;

                pickable->weight = TCOD_zip_get_float(zip);
            }
            break;
            case COMPONENT_POSITION:
            {
                position_t *position = (position_t *)component;

                position->level = TCOD_zip_get_int(zip);
                position->x = TCOD_zip_get_int(zip);
                position->y = TCOD_zip_get_int(zip);

                TCOD_list_push(game->maps[position->level].entities, entity);
                TCOD_list_push(game->maps[position->level].tiles[position->x][position->y].entities, entity);
            }
            break;
            case COMPONENT_TARGETING:
            {
                targeting_t *targeting = (targeting_t *)component;

                targeting->type = TCOD_zip_get_int(zip);
                targeting->x = TCOD_zip_get_int(zip);
                targeting->y = TCOD_zip_get_int(zip);
            }
            break;
            case COMPONENT_SOLID:
            {
            }
            break;
            }
        }
    }

    game->player = &game->entities[TCOD_zip_get_int(zip)];
    game->turn = TCOD_zip_get_int(zip);

    // int num_messages = TCOD_zip_get_int(zip);
    // int num_bytes = sizeof(message_t) * num_messages;
    // message_t *messages = malloc(num_bytes);

    // TCOD_zip_get_data(zip, num_bytes, messages);

    // for (int i = 0; i < num_messages; i++)
    // {
    //     TCOD_list_push(game->messages, &messages[i]);
    // }

    TCOD_zip_delete(zip);
}

internal void
game_update(game_t *game)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    switch (ev)
    {
    case TCOD_EVENT_KEY_PRESS:
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
        {
            game->should_quit = true;
        }
        break;
        case TCODK_ENTER:
        {
            if (key.lalt)
            {
                fullscreen = !fullscreen;

                TCOD_console_set_fullscreen(fullscreen);
            }
        }
        break;
        case TCODK_CHAR:
        {
            switch (key.c)
            {
            case 'r':
            {
                game->should_restart = true;
            }
            break;
            }
        }
        break;
        }
    }
    break;
    }

    if (game->turn_available)
    {
        switch (ev)
        {
        case TCOD_EVENT_KEY_PRESS:
        {
            switch (key.vk)
            {
            case TCODK_KP5:
            {
                game->should_update = true;
                game->should_render = true;
            }
            break;
            }
        }
        break;
        }

        if (!game->game_over)
        {
            switch (ev)
            {
            case TCOD_EVENT_KEY_PRESS:
            {
                switch (key.vk)
                {
                case TCODK_KP1:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        game->should_render = true;

                        player_targeting->x--;
                        player_targeting->y++;
                    }
                    else if (player_position)
                    {
                        game->should_update = true;
                        game->should_render = true;

                        int x = player_position->x - 1;
                        int y = player_position->y + 1;

                        if (key.lctrl)
                        {
                            entity_swing(game->player, x, y);
                        }
                        else
                        {
                            entity_move(game->player, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP2:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        game->should_render = true;

                        player_targeting->y++;
                    }
                    else if (player_position)
                    {
                        game->should_update = true;
                        game->should_render = true;

                        int x = player_position->x;
                        int y = player_position->y + 1;

                        if (key.lctrl)
                        {
                            entity_swing(game->player, x, y);
                        }
                        else
                        {
                            entity_move(game->player, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP3:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        game->should_render = true;

                        player_targeting->x++;
                        player_targeting->y++;
                    }
                    else if (player_position)
                    {
                        game->should_update = true;
                        game->should_render = true;

                        int x = player_position->x + 1;
                        int y = player_position->y + 1;

                        if (key.lctrl)
                        {
                            entity_swing(game->player, x, y);
                        }
                        else
                        {
                            entity_move(game->player, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP4:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        game->should_render = true;

                        player_targeting->x--;
                        player_targeting->y;
                    }
                    else if (player_position)
                    {
                        game->should_update = true;
                        game->should_render = true;

                        int x = player_position->x - 1;
                        int y = player_position->y;

                        if (key.lctrl)
                        {
                            entity_swing(game->player, x, y);
                        }
                        else
                        {
                            entity_move(game->player, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP6:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        game->should_render = true;

                        player_targeting->x++;
                        player_targeting->y;
                    }
                    else if (player_position)
                    {
                        game->should_update = true;
                        game->should_render = true;

                        int x = player_position->x + 1;
                        int y = player_position->y;

                        if (key.lctrl)
                        {
                            entity_swing(game->player, x, y);
                        }
                        else
                        {
                            entity_move(game->player, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP7:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        game->should_render = true;

                        player_targeting->x--;
                        player_targeting->y--;
                    }
                    else if (player_position)
                    {
                        game->should_update = true;
                        game->should_render = true;

                        int x = player_position->x - 1;
                        int y = player_position->y - 1;

                        if (key.lctrl)
                        {
                            entity_swing(game->player, x, y);
                        }
                        else
                        {
                            entity_move(game->player, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP8:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        game->should_render = true;

                        player_targeting->x;
                        player_targeting->y--;
                    }
                    else if (player_position)
                    {
                        game->should_update = true;
                        game->should_render = true;

                        int x = player_position->x;
                        int y = player_position->y - 1;

                        if (key.lctrl)
                        {
                            entity_swing(game->player, x, y);
                        }
                        else
                        {
                            entity_move(game->player, x, y);
                        }
                    }
                }
                break;
                case TCODK_KP9:
                {
                    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);
                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                    if (player_targeting)
                    {
                        game->should_render = true;

                        player_targeting->x++;
                        player_targeting->y--;
                    }
                    else if (player_position)
                    {
                        game->should_update = true;
                        game->should_render = true;

                        int x = player_position->x + 1;
                        int y = player_position->y - 1;

                        if (key.lctrl)
                        {
                            entity_swing(game->player, x, y);
                        }
                        else
                        {
                            entity_move(game->player, x, y);
                        }
                    }
                }
                break;
                case TCODK_CHAR:
                {
                    switch (key.c)
                    {
                    case 'c':
                    {
                        game->should_render = true;

                        game->panel_visible = !game->panel_visible;
                    }
                    break;
                    case 'f':
                    {
                        position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                        game->should_render = true;

                        if (player_position)
                        {
                            targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                            if (player_targeting && player_targeting->type == TARGETING_SHOOT)
                            {
                                game->should_update = true;

                                entity_shoot(game->player, player_targeting->x, player_targeting->y);

                                component_remove(game->player, COMPONENT_TARGETING);
                            }
                            else
                            {
                                player_targeting = (targeting_t *)component_add(game->player, COMPONENT_TARGETING);
                                player_targeting->type = TARGETING_SHOOT;

                                bool target_found = false;

                                {
                                    alignment_t *player_alignment = (alignment_t *)component_get(game->player, COMPONENT_ALIGNMENT);
                                    fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);

                                    if (player_alignment && player_fov)
                                    {
                                        for (void **iterator = TCOD_list_begin(game->maps[player_position->level].entities); iterator != TCOD_list_end(game->maps[player_position->level].entities); iterator++)
                                        {
                                            entity_t *other = *iterator;

                                            alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);
                                            position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                                            if (other_alignment && other_position)
                                            {
                                                if (TCOD_map_is_in_fov(player_fov->fov_map, other_position->x, other_position->y) &&
                                                    other_alignment->type != player_alignment->type)
                                                {
                                                    target_found = true;

                                                    player_targeting->x = other_position->x;
                                                    player_targeting->y = other_position->y;

                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }

                                if (!target_found)
                                {
                                    player_targeting->x = player_position->x;
                                    player_targeting->y = player_position->y;
                                }
                            }
                        }
                    }
                    break;
                    case 'g':
                    {
                        inventory_t *player_inventory = (inventory_t *)component_get(game->player, COMPONENT_INVENTORY);
                        position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                        if (player_inventory && player_position)
                        {
                            game->should_render = true;

                            tile_t *tile = &game->maps[player_position->level].tiles[player_position->x][player_position->y];

                            bool item_found = false;

                            for (void **iterator = TCOD_list_begin(tile->entities); iterator != TCOD_list_end(tile->entities); iterator++)
                            {
                                entity_t *other = *iterator;

                                pickable_t *pickable = (pickable_t *)component_get(other, COMPONENT_PICKABLE);

                                if (pickable != NULL)
                                {
                                    game->should_update = true;

                                    item_found = true;

                                    entity_pick(game->player, other);

                                    break;
                                }
                            }

                            if (!item_found)
                            {
                                game_log(game->player->game, player_position, TCOD_white, "There is nothing here!");
                            }
                        }
                    }
                    break;
                    case 'i':
                    {
                        game->should_render = true;

                        game->panel_visible = !game->panel_visible;
                    }
                    break;
                    case 'l':
                    {
                        position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                        game->should_render = true;

                        if (player_position)
                        {
                            targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                            if (player_targeting)
                            {
                                component_remove(game->player, COMPONENT_TARGETING);
                            }
                            else
                            {
                                player_targeting = (targeting_t *)component_add(game->player, COMPONENT_TARGETING);
                                player_targeting->type = TARGETING_LOOK;
                                player_targeting->x = player_position->x;
                                player_targeting->y = player_position->y;
                            }
                        }
                    }
                    break;
                    case 'm':
                    {
                        game->should_render = true;

                        game->message_log_visible = !game->message_log_visible;
                    }
                    break;
                    case 's':
                    {
                        if (key.lctrl)
                        {
                            game->should_render = true;

                            game_save(game);
                        }
                    }
                    break;
                    case 't':
                    {
                        light_t *player_light = (light_t *)component_get(game->player, COMPONENT_LIGHT);

                        if (player_light)
                        {
                            game->should_update = true;
                            game->should_render = true;

                            local bool torch = false;

                            torch = !torch;

                            if (torch)
                            {
                                player_light->radius = 10;
                                player_light->color = TCOD_light_amber;
                                player_light->flicker = true;
                                player_light->priority = LIGHT_PRIORITY_2;
                            }
                            else
                            {
                                player_light->radius = 5;
                                player_light->color = TCOD_white;
                                player_light->flicker = false;
                                player_light->priority = LIGHT_PRIORITY_0;
                            }
                        }
                    }
                    break;
                    case 'z':
                    {
                        caster_t *player_caster = (caster_t *)component_get(game->player, COMPONENT_CASTER);

                        if (player_caster)
                        {
                            spell_t *spell = &player_caster->spells[player_caster->current];

                            switch (spell->type)
                            {
                            case SPELL_HEAL_SELF:
                            {
                                game->should_update = true;
                                game->should_render = true;

                                entity_cast_spell(game->player);
                            }
                            break;
                            case SPELL_INSTAKILL:
                            {
                                position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

                                if (player_position)
                                {
                                    targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                                    game->should_render = true;

                                    if (player_targeting && player_targeting->type == TARGETING_ZAP)
                                    {
                                        game->should_update = true;

                                        entity_cast_spell(game->player);

                                        component_remove(game->player, COMPONENT_TARGETING);
                                    }
                                    else
                                    {
                                        player_targeting = (targeting_t *)component_add(game->player, COMPONENT_TARGETING);
                                        player_targeting->type = TARGETING_ZAP;

                                        bool target_found = false;

                                        {
                                            alignment_t *player_alignment = (alignment_t *)component_get(game->player, COMPONENT_ALIGNMENT);
                                            fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);

                                            if (player_alignment && player_fov)
                                            {
                                                for (void **iterator = TCOD_list_begin(game->maps[player_position->level].entities); iterator != TCOD_list_end(game->maps[player_position->level].entities); iterator++)
                                                {
                                                    entity_t *other = *iterator;

                                                    alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);
                                                    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                                                    if (other_alignment && other_position)
                                                    {
                                                        if (TCOD_map_is_in_fov(player_fov->fov_map, other_position->x, other_position->y) &&
                                                            other_alignment->type != player_alignment->type)
                                                        {
                                                            target_found = true;

                                                            player_targeting->x = other_position->x;
                                                            player_targeting->y = other_position->y;

                                                            break;
                                                        }
                                                    }
                                                }
                                            }
                                        }

                                        if (!target_found)
                                        {
                                            player_targeting->x = player_position->x;
                                            player_targeting->y = player_position->y;
                                        }
                                    }
                                }
                            }
                            break;
                            }
                        }
                    }
                    break;
                    }
                }
                break;
                }
            }
            break;
            }
        }
    }

    game->turn_available = true;

    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

    if (player_position)
    {
        map_t *player_map = &game->maps[player_position->level];

        for (void **iterator = TCOD_list_begin(player_map->entities); iterator != TCOD_list_end(player_map->entities); iterator++)
        {
            entity_t *current = *iterator;

            flash_t *current_flash = (flash_t *)component_get(current, COMPONENT_FLASH);

            if (current_flash)
            {
                current_flash->fade -= (1.0f / FPS) / 0.25f;

                if (current_flash->fade <= 0)
                {
                    component_remove(current, COMPONENT_FLASH);
                }

                {
                    fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);

                    position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                    if (player_fov && current_position)
                    {
                        if (TCOD_map_is_in_fov(player_fov->fov_map, current_position->x, current_position->y))
                        {
                            game->turn_available = false;
                            game->should_render = true;
                        }
                    }
                }
            }
        }

        if (game->should_update)
        {
            game->should_update = false;
            game->turn++;

            TCOD_list_t lights = TCOD_list_new();

            for (void **iterator = TCOD_list_begin(player_map->entities); iterator != TCOD_list_end(player_map->entities); iterator++)
            {
                entity_t *current = *iterator;

                light_t *current_light = (light_t *)component_get(current, COMPONENT_LIGHT);
                position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                if (current_light && current_position)
                {
                    if (current_light->fov_map != NULL)
                    {
                        TCOD_map_delete(current_light->fov_map);
                    }

                    map_t *current_map = &game->maps[current_position->level];

                    current_light->fov_map = map_to_fov_map(current_map, current_position->x, current_position->y, current_light->radius);

                    TCOD_list_push(lights, current);
                }
            }

            for (void **iterator = TCOD_list_begin(player_map->entities); iterator != TCOD_list_end(player_map->entities); iterator++)
            {
                entity_t *current = *iterator;

                position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                if (current_position)
                {
                    map_t *current_map = &game->maps[current_position->level];

                    fov_t *current_fov = (fov_t *)component_get(current, COMPONENT_FOV);

                    if (current_fov)
                    {
                        if (current_fov->fov_map != NULL)
                        {
                            TCOD_map_delete(current_fov->fov_map);
                        }

                        current_fov->fov_map = map_to_fov_map(current_map, current_position->x, current_position->y, current_fov->radius);

                        {
                            TCOD_map_t los_map = map_to_fov_map(current_map, current_position->x, current_position->y, 0);

                            for (int x = 0; x < MAP_WIDTH; x++)
                            {
                                for (int y = 0; y < MAP_HEIGHT; y++)
                                {
                                    if (TCOD_map_is_in_fov(los_map, x, y))
                                    {
                                        tile_t *tile = &current_map->tiles[x][y];

                                        for (void **iterator = TCOD_list_begin(lights); iterator != TCOD_list_end(lights); iterator++)
                                        {
                                            entity_t *other = *iterator;

                                            light_t *other_light = (light_t *)component_get(other, COMPONENT_LIGHT);
                                            position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                                            if (other_light && other_position)
                                            {
                                                if (TCOD_map_is_in_fov(other_light->fov_map, x, y))
                                                {
                                                    TCOD_map_set_in_fov(current_fov->fov_map, x, y, true);
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            TCOD_map_delete(los_map);
                        }
                    }

                    ai_t *current_ai = (ai_t *)component_get(current, COMPONENT_AI);

                    if (current_ai)
                    {
                        current_ai->energy += current_ai->energy_per_turn;

                        while (current_ai->energy >= 1.0f)
                        {
                            current_ai->energy -= 1.0f;

                            bool target_found = false;

                            alignment_t *current_alignment = (alignment_t *)component_get(current, COMPONENT_ALIGNMENT);

                            if (current_alignment && current_fov)
                            {
                                for (void **iterator = TCOD_list_begin(current_map->entities); iterator != TCOD_list_end(current_map->entities); iterator++)
                                {
                                    entity_t *other = *iterator;

                                    alignment_t *other_alignment = (alignment_t *)component_get(other, COMPONENT_ALIGNMENT);
                                    position_t *other_position = (position_t *)component_get(other, COMPONENT_POSITION);

                                    if (other_alignment && other_position)
                                    {
                                        if (TCOD_map_is_in_fov(current_fov->fov_map, other_position->x, other_position->y) &&
                                            other_alignment->type != current_alignment->type)
                                        {
                                            target_found = true;

                                            if (distance(current_position->x, current_position->y, other_position->x, other_position->y) < 2.0f)
                                            {
                                                entity_attack(current, other);
                                            }
                                            else
                                            {
                                                entity_path_towards(current, other_position->x, other_position->y);
                                            }

                                            break;
                                        }
                                    }
                                }
                            }

                            if (current_fov && current_ai->follow_target)
                            {
                                position_t *follow_position = (position_t *)component_get(current_ai->follow_target, COMPONENT_POSITION);

                                if (follow_position)
                                {
                                    if (!TCOD_map_is_in_fov(current_fov->fov_map, follow_position->x, follow_position->y) ||
                                        distance(current_position->x, current_position->y, follow_position->x, follow_position->y) > 5.0f)
                                    {
                                        target_found = true;

                                        entity_path_towards(current, follow_position->x, follow_position->y);
                                    }
                                }
                            }

                            if (!target_found)
                            {
                                entity_move_random(current);
                            }
                        }
                    }
                }
            }

            TCOD_list_delete(lights);
        }

        if (game->should_render)
        {
            game->should_render = false;

            TCOD_console_set_default_background(NULL, TCOD_black);
            TCOD_console_set_default_foreground(NULL, TCOD_white);
            TCOD_console_clear(NULL);

            int message_log_x = 0;
            int message_log_height = console_height / 4;
            int message_log_y = console_height - message_log_height;
            int message_log_width = console_width;

            int panel_width = console_width / 2;
            int panel_x = console_width - panel_width;
            int panel_y = 0;
            int panel_height = console_height - (game->message_log_visible ? message_log_height : 0);

            fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);

            if (player_fov)
            {
                map_t *player_map = &game->maps[player_position->level];

                int view_width = console_width - (game->panel_visible ? panel_width : 0);
                int view_height = console_height - (game->message_log_visible ? message_log_height : 0);
                int view_x = player_position->x - view_width / 2;
                int view_y = player_position->y - view_height / 2;

                if (view_width < MAP_WIDTH && view_height < MAP_HEIGHT)
                {
                    view_x = view_x < 0
                                 ? 0
                                 : view_x + view_width > MAP_WIDTH
                                       ? MAP_WIDTH - view_width
                                       : view_x;
                    view_y = view_y < 0
                                 ? 0
                                 : view_y + view_height > MAP_HEIGHT
                                       ? MAP_HEIGHT - view_height
                                       : view_y;
                }

                {
                    TCOD_list_t entities_by_layer[NUM_LAYERS];
                    TCOD_list_t lights_by_priority[NUM_LIGHT_PRIORITIES];

                    for (int i = 0; i < NUM_LAYERS; i++)
                    {
                        entities_by_layer[i] = TCOD_list_new();
                    }

                    for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
                    {
                        lights_by_priority[i] = TCOD_list_new();
                    }

                    for (void **iterator = TCOD_list_begin(player_map->entities); iterator != TCOD_list_end(player_map->entities); iterator++)
                    {
                        entity_t *current = *iterator;

                        appearance_t *current_appearance = (appearance_t *)component_get(current, COMPONENT_APPEARANCE);

                        if (current_appearance)
                        {
                            TCOD_list_push(entities_by_layer[current_appearance->layer], current);
                        }

                        light_t *current_light = (light_t *)component_get(current, COMPONENT_LIGHT);
                        position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                        if (current_light && current_position)
                        {
                            TCOD_list_push(lights_by_priority[current_light->priority], current);
                        }
                    }

                    {
                        // local TCOD_noise_t noise = NULL;
                        // if (noise == NULL)
                        // {
                        //     noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
                        // }

                        // local float noise_x = 0.0f;

                        // noise_x += 0.2f;
                        // float noise_dx = noise_x + 20.0f;
                        // float dx = TCOD_noise_get(noise, &noise_dx) * 0.5f;
                        // noise_dx += 30.0f;
                        // float dy = TCOD_noise_get(noise, &noise_dx) * 0.5f;
                        // float di = 0.2f * TCOD_noise_get(noise, &noise_x);

                        for (int x = view_x; x < view_x + view_width; x++)
                        {
                            for (int y = view_y; y < view_y + view_height; y++)
                            {
                                if (map_is_inside(x, y))
                                {
                                    tile_t *tile = &player_map->tiles[x][y];
                                    tile_info_t *tile_info = &game->tile_info[tile->type];

                                    if (TCOD_map_is_in_fov(player_fov->fov_map, x, y))
                                    {
                                        tile->seen = true;
                                    }

                                    for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
                                    {
                                        for (void **iterator = TCOD_list_begin(lights_by_priority[i]); iterator != TCOD_list_end(lights_by_priority[i]); iterator++)
                                        {
                                            entity_t *current = *iterator;

                                            light_t *current_light = (light_t *)component_get(current, COMPONENT_LIGHT);

                                            if (current_light)
                                            {
                                                if (TCOD_map_is_in_fov(current_light->fov_map, x, y))
                                                {
                                                    tile->seen = true;
                                                }
                                            }
                                        }
                                    }

                                    TCOD_color_t color = game->tile_common.shadow_color;

                                    if (TCOD_map_is_in_fov(player_fov->fov_map, x, y) || tile->seen)
                                    {
                                        for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
                                        {
                                            for (void **iterator = TCOD_list_begin(lights_by_priority[i]); iterator != TCOD_list_end(lights_by_priority[i]); iterator++)
                                            {
                                                entity_t *current = *iterator;

                                                light_t *current_light = (light_t *)component_get(current, COMPONENT_LIGHT);
                                                position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                                                if (current_light && current_position)
                                                {
                                                    if (TCOD_map_is_in_fov(current_light->fov_map, x, y))
                                                    {
                                                        // float r2 = pow(current_light->radius, 2);
                                                        // float d = pow(x - current_position->x + (current_light->flicker ? dx : 0), 2) + pow(y - current_position->y + (current_light->flicker ? dy : 0), 2);
                                                        // float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + (current_light->flicker ? di : 0));

                                                        float r2 = pow(current_light->radius, 2);
                                                        float d = pow(x - current_position->x, 2) + pow(y - current_position->y, 2);
                                                        float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2);

                                                        color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info->color, current_light->color, l), l);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if (!tile->seen)
                                        {
                                            continue;
                                        }
                                    }

                                    TCOD_console_set_char_foreground(NULL, x - view_x, y - view_y, color);
                                    TCOD_console_set_char(NULL, x - view_x, y - view_y, tile_info->glyph);
                                }
                            }
                        }
                    }

                    for (int i = 0; i < NUM_LAYERS; i++)
                    {
                        for (void **iterator = TCOD_list_begin(entities_by_layer[i]); iterator != TCOD_list_end(entities_by_layer[i]); iterator++)
                        {
                            entity_t *current = *iterator;

                            appearance_t *current_appearance = (appearance_t *)component_get(current, COMPONENT_APPEARANCE);
                            position_t *current_position = (position_t *)component_get(current, COMPONENT_POSITION);

                            if (current_appearance && current_position)
                            {
                                if (current_position->level == player_position->level &&
                                    TCOD_map_is_in_fov(player_fov->fov_map, current_position->x, current_position->y))
                                {
                                    TCOD_color_t color = current_appearance->color;

                                    flash_t *current_flash = (flash_t *)component_get(current, COMPONENT_FLASH);

                                    if (current_flash)
                                    {
                                        color = TCOD_color_lerp(current_appearance->color, current_flash->color, current_flash->fade);
                                    }

                                    TCOD_console_set_char_foreground(NULL, current_position->x - view_x, current_position->y - view_y, color);
                                    TCOD_console_set_char(NULL, current_position->x - view_x, current_position->y - view_y, current_appearance->glyph);
                                }
                            }
                        }
                    }

                    for (int i = 0; i < NUM_LIGHT_PRIORITIES; i++)
                    {
                        TCOD_list_delete(lights_by_priority[i]);
                    }

                    for (int i = 0; i < NUM_LAYERS; i++)
                    {
                        TCOD_list_delete(entities_by_layer[i]);
                    }
                }

                targeting_t *player_targeting = (targeting_t *)component_get(game->player, COMPONENT_TARGETING);

                if (player_targeting)
                {
                    TCOD_console_set_char_foreground(NULL, player_targeting->x - view_x, player_targeting->y - view_y, TCOD_red);
                    TCOD_console_set_char(NULL, player_targeting->x - view_x, player_targeting->y - view_y, 'X');

                    switch (player_targeting->type)
                    {
                    case TARGETING_LOOK:
                    {
                        tile_t *tile = &game->maps[player_position->level].tiles[player_targeting->x][player_targeting->y];

                        if (TCOD_map_is_in_fov(player_fov->fov_map, player_targeting->x, player_targeting->y))
                        {
                            entity_t *entity = TCOD_list_peek(tile->entities);
                            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_APPEARANCE);

                            if (entity == NULL && appearance == NULL)
                            {
                                TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, game->tile_info[tile->type].name);
                            }
                            else
                            {
                                TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, appearance->name);
                            }
                        }
                        else
                        {
                            if (tile->seen)
                            {
                                TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, "%s (remembered)", game->tile_info[tile->type].name);
                            }
                            else
                            {
                                TCOD_console_print_ex(NULL, console_width / 2, message_log_y - 2, TCOD_BKGND_NONE, TCOD_CENTER, "Unknown");
                            }
                        }
                        break;
                    }
                    }
                }
            }

            if (game->message_log_visible)
            {
                local TCOD_console_t message_log = NULL;

                if (message_log == NULL)
                {
                    message_log = TCOD_console_new(console_width, console_height);
                }

                TCOD_console_set_default_background(message_log, TCOD_black);
                TCOD_console_set_default_foreground(message_log, TCOD_white);
                TCOD_console_clear(message_log);

                int y = 1;
                for (void **i = TCOD_list_begin(game->messages); i != TCOD_list_end(game->messages); i++)
                {
                    message_t *message = *i;

                    TCOD_console_set_default_foreground(message_log, message->color);
                    TCOD_console_print(message_log, message_log_x + 1, y, message->text);

                    y++;
                }

                TCOD_console_set_default_foreground(message_log, TCOD_white);
                TCOD_console_print_frame(message_log, 0, 0, message_log_width, message_log_height, false, TCOD_BKGND_SET, "Log");

                TCOD_console_blit(message_log, 0, 0, message_log_width, message_log_height, NULL, message_log_x, message_log_y, 1, 1);
            }

            if (game->panel_visible)
            {
                local TCOD_console_t panel = NULL;

                if (panel == NULL)
                {
                    panel = TCOD_console_new(console_height, console_width);
                }

                TCOD_console_print_frame(panel, 0, 0, panel_width, panel_height, false, TCOD_BKGND_SET, "Panel");

                TCOD_console_blit(panel, 0, 0, panel_width, panel_height, NULL, panel_x, panel_y, 1, 1);
            }

            TCOD_console_print(NULL, 0, 0, "Turn: %d", game->turn);

            TCOD_console_flush();
        }
    }
}

internal void
game_log(game_t *game, position_t *position, TCOD_color_t color, char *text, ...)
{
    fov_t *player_fov = (fov_t *)component_get(game->player, COMPONENT_FOV);
    position_t *player_position = (position_t *)component_get(game->player, COMPONENT_POSITION);

    if (player_fov && player_position)
    {
        if (!position ||
            (position->level == player_position->level &&
             ((position->x == player_position->x && position->y == player_position->y) ||
              TCOD_map_is_in_fov(player_fov->fov_map, position->x, position->y))))
        {
            va_list ap;
            char buf[128];
            va_start(ap, text);
            vsprintf(buf, text, ap);
            va_end(ap);

            char *line_begin = buf;
            char *line_end;

            do
            {
                if (TCOD_list_size(game->messages) == (console_height / 4) - 2)
                {
                    message_t *message = TCOD_list_get(game->messages, 0);

                    TCOD_list_remove(game->messages, message);

                    message_destroy(message);
                }

                line_end = strchr(line_begin, '\n');

                if (line_end)
                {
                    *line_end = '\0';
                }

                message_t *message = message_create(line_begin, color);

                TCOD_list_push(game->messages, message);

                line_begin = line_end + 1;
            } while (line_end);
        }
    }
}

internal void
game_reset(game_t *game)
{
    for (int i = 0; i < NUM_ENTITIES; i++)
    {
        entity_t *entity = &game->entities[i];

        entity_reset(entity);

        for (int j = 0; j < NUM_COMPONENTS; j++)
        {
            component_t *component = &game->components[j][i];

            component_reset(component);
        }
    }

    for (int i = 0; i < NUM_MAPS; i++)
    {
        map_t *map = &game->maps[i];

        map_reset(map);
    }

    for (void **iterator = TCOD_list_begin(game->messages); iterator != TCOD_list_end(game->messages); iterator++)
    {
        message_t *message = *iterator;

        message_destroy(message);
    }

    TCOD_list_delete(game->messages);
}