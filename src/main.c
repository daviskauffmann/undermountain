#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <libtcod.h>

#include "CMemLeak.h"
#include "system.h"

/* Game */
typedef enum game_status_e {
    STATUS_WAITING,
    STATUS_UPDATE,
    STATUS_QUIT
} game_status_t;

game_status_t game_status;
int turn;
TCOD_list_t maps;
struct entity_s *player;

/* Maps */
#define MAP_WIDTH 50
#define MAP_HEIGHT 50
#define BSP_DEPTH 10
#define MIN_ROOM_SIZE 5
#define FULL_ROOMS 1

typedef enum tile_type_e {
    TILE_TYPE_FLOOR,
    TILE_TYPE_WALL,

    NUM_TILE_TYPES
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
    struct entity_s *entity;
} tile_t;

tile_common_t tile_common;
tile_info_t tile_info[NUM_TILE_TYPES];

typedef struct map_s
{
    int level;
    tile_t tiles[MAP_WIDTH][MAP_HEIGHT];
} map_t;

static bool traverse_node(TCOD_bsp_t *node, map_t *map);
static void vline(map_t *map, int x, int y1, int y2);
static void vline_up(map_t *map, int x, int y);
static void vline_down(map_t *map, int x, int y);
static void hline(map_t *map, int x1, int y, int x2);
static void hline_left(map_t *map, int x, int y);
static void hline_right(map_t *map, int x, int y);

map_t *map_create(int level)
{
    map_t *map = (map_t *)malloc(sizeof(map_t));

    map->level = level;

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            tile->type = TILE_TYPE_WALL;
            tile->seen = false;
            tile->entity = NULL;
        }
    }

    TCOD_bsp_t *bsp = TCOD_bsp_new_with_size(0, 0, MAP_WIDTH, MAP_HEIGHT);
    TCOD_bsp_split_recursive(bsp, NULL, BSP_DEPTH, MIN_ROOM_SIZE + 1, MIN_ROOM_SIZE + 1, 1.5f, 1.5f);
    TCOD_bsp_traverse_inverted_level_order(bsp, traverse_node, map);
    TCOD_bsp_delete(bsp);

    return map;
}

static bool traverse_node(TCOD_bsp_t *node, map_t *map)
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

                tile->type = TILE_TYPE_FLOOR;
            }
        }
    }
    else
    {
        TCOD_bsp_t *left = TCOD_bsp_left(node);
        TCOD_bsp_t *right = TCOD_bsp_right(node);

        node->x = min(left->x, right->x);
        node->y = min(left->y, right->y);
        node->w = max(left->x + left->w, right->x + right->w) - node->x;
        node->h = max(left->y + left->h, right->y + right->h) - node->y;

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
                int min_x = max(left->x, right->x);
                int max_x = min(left->x + left->w - 1, right->x + right->w - 1);
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
                int min_y = max(left->y, right->y);
                int max_y = min(left->y + left->h - 1, right->y + right->h - 1);
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

static void vline(map_t *map, int x, int y1, int y2)
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

        tile->type = TILE_TYPE_FLOOR;
    }
}

static void vline_up(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y >= 0 && tile->type != TILE_TYPE_FLOOR)
    {
        tile->type = TILE_TYPE_FLOOR;

        y--;
    }
}

static void vline_down(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (y < MAP_HEIGHT && tile->type != TILE_TYPE_FLOOR)
    {
        tile->type = TILE_TYPE_FLOOR;

        y++;
    }
}

static void hline(map_t *map, int x1, int y, int x2)
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

        tile->type = TILE_TYPE_FLOOR;
    }
}

static void hline_left(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (x >= 0 && tile->type != TILE_TYPE_FLOOR)
    {
        tile->type = TILE_TYPE_FLOOR;

        x--;
    }
}

static void hline_right(map_t *map, int x, int y)
{
    tile_t *tile = &map->tiles[x][y];

    while (x < MAP_WIDTH && tile->type != TILE_TYPE_FLOOR)
    {
        tile->type = TILE_TYPE_FLOOR;

        x++;
    }
}

bool map_is_inside(int x, int y)
{
    return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
}

TCOD_map_t map_to_TCOD_map(map_t *map)
{
    TCOD_map_t TCOD_map = TCOD_map_new(MAP_WIDTH, MAP_HEIGHT);

    for (int x = 0; x < MAP_WIDTH; x++)
    {
        for (int y = 0; y < MAP_HEIGHT; y++)
        {
            tile_t *tile = &map->tiles[x][y];

            TCOD_map_set_properties(TCOD_map, x, y, tile_info[tile->type].is_transparent, tile_info[tile->type].is_walkable);
        }
    }

    return TCOD_map;
}

TCOD_map_t map_to_fov_map(map_t *map, int x, int y, int radius)
{
    TCOD_map_t fov_map = map_to_TCOD_map(map);

    TCOD_map_compute_fov(fov_map, x, y, radius, true, FOV_BASIC);

    return fov_map;
}

/* Components */
#define MAX_ENTITIES 65536
#define ID_UNUSED -1

typedef enum component_type_e {
    COMPONENT_TYPE_POSITION,
    COMPONENT_TYPE_PHYSICS,
    COMPONENT_TYPE_LIGHT,
    COMPONENT_TYPE_FOV,
    COMPONENT_TYPE_APPEARANCE,

    NUM_COMPONENTS
} component_type_t;

typedef struct position_s
{
    map_t *map;
    int x;
    int y;
    int next_x;
    int next_y;
} position_t;

typedef struct physics_s
{
    bool is_walkable;
    bool is_transparent;
} physics_t;

typedef struct light_s
{
    int radius;
    TCOD_color_t color;
    bool flicker;
    TCOD_map_t fov_map;
} light_t;

typedef struct fov_s
{
    int radius;
    TCOD_map_t fov_map;
} fov_t;

typedef struct appearance_s
{
    char *name;
    unsigned char glyph;
    TCOD_color_t color;
} appearance_t;

typedef struct component_s
{
    int id;
    component_type_t type;
    union {
        position_t position;
        physics_t physics;
        light_t light;
        fov_t fov;
        appearance_t appearance;
    };
} component_t;

component_t component_lists[NUM_COMPONENTS][MAX_ENTITIES];

/* Entities */
typedef struct entity_s
{
    int id;
} entity_t;

entity_t entities[MAX_ENTITIES];

entity_t *entity_create(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (entities[i].id == ID_UNUSED)
        {
            entities[i].id = i;

            return &entities[i];
        }
    }

    return NULL;
}

void entity_destroy(entity_t *entity)
{
    for (int i = 0; i < NUM_COMPONENTS; i++)
    {
        component_lists[i][entity->id].id = ID_UNUSED;
    }

    entity->id = ID_UNUSED;
}

component_t *component_add(entity_t *entity, component_type_t component_type)
{
    component_t *component = &component_lists[component_type][entity->id];

    component->id = entity->id;

    return component;
}

component_t *component_get(entity_t *entity, component_type_t component_type)
{
    component_t *component = &component_lists[component_type][entity->id];

    if (component->id != ID_UNUSED)
    {
        return component;
    }

    return NULL;
}

/* Systems */
void input_system(void)
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
            game_status = STATUS_QUIT;

            break;
        }
        case TCODK_KP1:
        {
            game_status = STATUS_UPDATE;

            position_t *position = (position_t *)component_get(player, COMPONENT_TYPE_POSITION);

            if (position != NULL)
            {
                position->next_x = position->x - 1;
                position->next_y = position->y + 1;
            }

            break;
        }
        case TCODK_KP2:
        {
            game_status = STATUS_UPDATE;

            position_t *position = (position_t *)component_get(player, COMPONENT_TYPE_POSITION);

            if (position != NULL)
            {
                position->next_x = position->x;
                position->next_y = position->y + 1;
            }

            break;
        }
        case TCODK_KP3:
        {
            game_status = STATUS_UPDATE;

            position_t *position = (position_t *)component_get(player, COMPONENT_TYPE_POSITION);

            if (position != NULL)
            {
                position->next_x = position->x + 1;
                position->next_y = position->y + 1;
            }

            break;
        }
        case TCODK_KP4:
        {
            game_status = STATUS_UPDATE;

            position_t *position = (position_t *)component_get(player, COMPONENT_TYPE_POSITION);

            if (position != NULL)
            {
                position->next_x = position->x - 1;
                position->next_y = position->y;
            }

            break;
        }
        case TCODK_KP5:
        {
            game_status = STATUS_UPDATE;

            break;
        }
        case TCODK_KP6:
        {
            game_status = STATUS_UPDATE;

            position_t *position = (position_t *)component_get(player, COMPONENT_TYPE_POSITION);

            if (position != NULL)
            {
                position->next_x = position->x + 1;
                position->next_y = position->y;
            }

            break;
        }
        case TCODK_KP7:
        {
            game_status = STATUS_UPDATE;

            position_t *position = (position_t *)component_get(player, COMPONENT_TYPE_POSITION);

            if (position != NULL)
            {
                position->next_x = position->x - 1;
                position->next_y = position->y - 1;
            }

            break;
        }
        case TCODK_KP8:
        {
            game_status = STATUS_UPDATE;

            position_t *position = (position_t *)component_get(player, COMPONENT_TYPE_POSITION);

            if (position != NULL)
            {
                position->next_x = position->x;
                position->next_y = position->y - 1;
            }

            break;
        }
        case TCODK_KP9:
        {
            game_status = STATUS_UPDATE;

            position_t *position = (position_t *)component_get(player, COMPONENT_TYPE_POSITION);

            if (position != NULL)
            {
                position->next_x = position->x + 1;
                position->next_y = position->y - 1;
            }

            break;
        }
        }
    }
    }
}

void ai_system(void)
{
}

void movement_system(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
        {
            position_t *position = (position_t *)component_get(entity, COMPONENT_TYPE_POSITION);

            if (position != NULL)
            {
                if (position->next_x != -1 && position->next_y != -1)
                {
                    bool can_move = true;

                    tile_t *tile = &position->map->tiles[position->x][position->y];
                    tile_t *next_tile = &position->map->tiles[position->next_x][position->next_y];

                    if (!tile_info[next_tile->type].is_walkable)
                    {
                        can_move = false;
                    }

                    // TODO: better way to check entity at position?
                    for (int j = 0; j < MAX_ENTITIES; j++)
                    {
                        entity_t *next_entity = &entities[j];

                        if (next_entity->id != ID_UNUSED)
                        {
                            position_t *next_position = (position_t *)component_get(&entities[j], COMPONENT_TYPE_POSITION);

                            if (next_position != NULL)
                            {
                                if (next_position->x == position->next_x && next_position->y == position->next_y)
                                {
                                    physics_t *physics = (physics_t *)component_get(&entities[j], COMPONENT_TYPE_PHYSICS);

                                    if (physics != NULL)
                                    {
                                        if (!physics->is_walkable)
                                        {
                                            can_move = false;
                                        }
                                    }
                                }
                            }
                        }
                    }

                    if (can_move)
                    {
                        position->x = position->next_x;
                        position->y = position->next_y;

                        // tile->entity = NULL;
                        // next_tile->entity = &entities[i];
                    }

                    position->next_x = -1;
                    position->next_y = -1;
                }
            }
        }
    }
}

void lighting_system(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
        {
            position_t *position = (position_t *)component_get(entity, COMPONENT_TYPE_POSITION);
            light_t *light = (light_t *)component_get(entity, COMPONENT_TYPE_LIGHT);

            if (position != NULL && light != NULL)
            {
                if (light->fov_map != NULL)
                {
                    TCOD_map_delete(light->fov_map);
                }

                light->fov_map = map_to_fov_map(position->map, position->x, position->y, light->radius);
            }
        }
    }
}

void fov_system(void)
{
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
        {
            position_t *position = (position_t *)component_get(entity, COMPONENT_TYPE_POSITION);
            fov_t *fov = (fov_t *)component_get(entity, COMPONENT_TYPE_FOV);

            if (position != NULL && fov != NULL)
            {
                if (fov->fov_map != NULL)
                {
                    TCOD_map_delete(fov->fov_map);
                }

                fov->fov_map = map_to_fov_map(position->map, position->x, position->y, fov->radius);

                TCOD_map_t los_map = map_to_fov_map(position->map, position->x, position->y, 0);

                TCOD_list_t light_entities = TCOD_list_new();

                for (int i = 0; i < MAX_ENTITIES; i++)
                {
                    entity_t *entity = &entities[i];

                    if (entity->id != ID_UNUSED)
                    {
                        position_t *position = (position_t *)component_get(entity, COMPONENT_TYPE_POSITION);
                        light_t *light = (light_t *)component_get(entity, COMPONENT_TYPE_LIGHT);

                        if (position != NULL && light != NULL)
                        {
                            TCOD_list_push(light_entities, entity);
                        }
                    }
                }

                for (int x = 0; x < MAP_WIDTH; x++)
                {
                    for (int y = 0; y < MAP_HEIGHT; y++)
                    {
                        if (TCOD_map_is_in_fov(los_map, x, y))
                        {
                            tile_t *tile = &position->map->tiles[x][y];

                            for (void **iterator = TCOD_list_begin(light_entities); iterator != TCOD_list_end(light_entities); iterator++)
                            {
                                entity_t *entity = *iterator;

                                position_t *position = (position_t *)component_get(entity, COMPONENT_TYPE_POSITION);
                                light_t *light = (light_t *)component_get(entity, COMPONENT_TYPE_LIGHT);

                                if (TCOD_map_is_in_fov(light->fov_map, x, y))
                                {
                                    TCOD_map_set_in_fov(fov->fov_map, x, y, true);
                                }
                            }
                        }
                    }
                }

                TCOD_list_delete(light_entities);

                TCOD_map_delete(los_map);
            }
        }
    }
}

#define CONSTRAIN_VIEW 1

void render_system(void)
{
    TCOD_console_set_default_background(NULL, TCOD_black);
    TCOD_console_set_default_foreground(NULL, TCOD_white);
    TCOD_console_clear(NULL);

    position_t *position = (position_t *)component_get(player, COMPONENT_TYPE_POSITION);

    static int view_x;
    static int view_y;
    static int view_width;
    static int view_height;

    view_width = screen_width;
    view_height = screen_height;
    view_x = position->x - view_width / 2;
    view_y = position->y - view_height / 2;

#if CONSTRAIN_VIEW
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
#endif

    static TCOD_noise_t noise = NULL;
    if (noise == NULL)
    {
        noise = TCOD_noise_new(1, TCOD_NOISE_DEFAULT_HURST, TCOD_NOISE_DEFAULT_LACUNARITY, NULL);
    }

    static float noise_x = 0.0f;

    noise_x += 0.2f;
    float noise_dx = noise_x + 20.0f;
    float dx = TCOD_noise_get(noise, &noise_dx) * 0.5f;
    noise_dx += 30.0f;
    float dy = TCOD_noise_get(noise, &noise_dx) * 0.5f;
    float di = 0.2f * TCOD_noise_get(noise, &noise_x);

    TCOD_list_t light_entities = TCOD_list_new();

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
        {
            position_t *position = (position_t *)component_get(entity, COMPONENT_TYPE_POSITION);
            light_t *light = (light_t *)component_get(entity, COMPONENT_TYPE_LIGHT);

            if (position != NULL && light != NULL)
            {
                TCOD_list_push(light_entities, entity);
            }
        }
    }

    fov_t *fov = (fov_t *)component_get(player, COMPONENT_TYPE_FOV);

    for (int x = view_x; x < view_x + view_width; x++)
    {
        for (int y = view_y; y < view_y + view_height; y++)
        {
            if (map_is_inside(x, y))
            {
                tile_t *tile = &position->map->tiles[x][y];

                if (TCOD_map_is_in_fov(fov->fov_map, x, y))
                {
                    tile->seen = true;
                }

                for (void **iterator = TCOD_list_begin(light_entities); iterator != TCOD_list_end(light_entities); iterator++)
                {
                    entity_t *entity = *iterator;

                    position_t *position = (position_t *)component_get(entity, COMPONENT_TYPE_POSITION);
                    light_t *light = (light_t *)component_get(entity, COMPONENT_TYPE_LIGHT);

                    if (TCOD_map_is_in_fov(light->fov_map, x, y))
                    {
                        tile->seen = true;
                    }
                }

                TCOD_color_t color = tile_common.shadow_color;

                if (TCOD_map_is_in_fov(fov->fov_map, x, y) || tile->seen)
                {
                    for (void **iterator = TCOD_list_begin(light_entities); iterator != TCOD_list_end(light_entities); iterator++)
                    {
                        entity_t *entity = *iterator;

                        position_t *position = (position_t *)component_get(entity, COMPONENT_TYPE_POSITION);
                        light_t *light = (light_t *)component_get(entity, COMPONENT_TYPE_LIGHT);

                        if (TCOD_map_is_in_fov(light->fov_map, x, y))
                        {
                            float r2 = pow(light->radius, 2);
                            float d = pow(x - position->x + (light->flicker ? dx : 0), 2) + pow(y - position->y + (light->flicker ? dy : 0), 2);
                            float l = CLAMP(0.0f, 1.0f, (r2 - d) / r2 + (light->flicker ? di : 0));

                            color = TCOD_color_lerp(color, TCOD_color_lerp(tile_info[tile->type].color, light->color, l), l);
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
                TCOD_console_set_char(NULL, x - view_x, y - view_y, tile_info[tile->type].glyph);
            }
        }
    }

    TCOD_list_delete(light_entities);

    // TODO: maybe store a list of all entities on a given map?
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entity_t *entity = &entities[i];

        if (entity->id != ID_UNUSED)
        {
            position_t *position = (position_t *)component_get(entity, COMPONENT_TYPE_POSITION);
            appearance_t *appearance = (appearance_t *)component_get(entity, COMPONENT_TYPE_APPEARANCE);

            if (position != NULL && appearance != NULL)
            {
                if (TCOD_map_is_in_fov(fov->fov_map, position->x, position->y))
                {
                    TCOD_console_set_char_foreground(NULL, position->x - view_x, position->y - view_y, appearance->color);
                    TCOD_console_put_char(NULL, position->x - view_x, position->y - view_y, appearance->glyph, TCOD_BKGND_NONE);
                }
            }
        }
    }

    TCOD_console_flush();
}

int main(int argc, char *argv[])
{
    system_init();

    tile_common = (tile_common_t){
        .shadow_color = TCOD_color_RGB(16, 16, 32)};

    tile_info[TILE_TYPE_FLOOR] = (tile_info_t){
        .glyph = '.',
        .color = TCOD_white,
        .is_transparent = true,
        .is_walkable = true};
    tile_info[TILE_TYPE_WALL] = (tile_info_t){
        .glyph = '#',
        .color = TCOD_white,
        .is_transparent = false,
        .is_walkable = false};

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        entities[i].id = ID_UNUSED;

        for (int j = 0; j < NUM_COMPONENTS; j++)
        {
            component_lists[j][i].id = ID_UNUSED;
        }
    }

    game_status = STATUS_UPDATE;
    turn = 0;

    maps = TCOD_list_new();
    map_t *map = map_create(0);
    TCOD_list_push(maps, map);

    player = entity_create();
    position_t *player_position = (position_t *)component_add(player, COMPONENT_TYPE_POSITION);
    player_position->map = map;
    player_position->x = 5;
    player_position->y = 5;
    player_position->next_x = -1;
    player_position->next_y = -1;
    physics_t *player_physics = (physics_t *)component_add(player, COMPONENT_TYPE_PHYSICS);
    player_physics->is_walkable = false;
    player_physics->is_transparent = true;
    light_t *player_light = (light_t *)component_add(player, COMPONENT_TYPE_LIGHT);
    player_light->radius = 5;
    player_light->color = TCOD_white;
    player_light->flicker = false;
    player_light->fov_map = NULL;
    fov_t *player_fov = (fov_t *)component_add(player, COMPONENT_TYPE_FOV);
    player_fov->radius = 5;
    player_fov->fov_map = NULL;
    appearance_t *player_appearance = (appearance_t *)component_add(player, COMPONENT_TYPE_APPEARANCE);
    player_appearance->name = "Blinky";
    player_appearance->glyph = '@';
    player_appearance->color = TCOD_white;

    entity_t *npc = entity_create();
    position_t *npc_position = (position_t *)component_add(npc, COMPONENT_TYPE_POSITION);
    npc_position->map = map;
    npc_position->x = 10;
    npc_position->y = 10;
    npc_position->next_x = -1;
    npc_position->next_y = -1;
    physics_t *npc_physics = (physics_t *)component_add(npc, COMPONENT_TYPE_PHYSICS);
    npc_physics->is_walkable = false;
    npc_physics->is_transparent = true;
    light_t *npc_light = (light_t *)component_add(npc, COMPONENT_TYPE_LIGHT);
    npc_light->radius = 5;
    npc_light->color = TCOD_light_amber;
    npc_light->flicker = true;
    npc_light->fov_map = NULL;
    appearance_t *npc_appearance = (appearance_t *)component_add(npc, COMPONENT_TYPE_APPEARANCE);
    npc_appearance->name = "NPC";
    npc_appearance->glyph = '@';
    npc_appearance->color = TCOD_yellow;

    while (!TCOD_console_is_window_closed())
    {
        input_system();

        if (game_status == STATUS_UPDATE)
        {
            ai_system();
            lighting_system();
            movement_system();
            fov_system();
        }

        render_system();

        if (game_status == STATUS_QUIT)
        {
            break;
        }

        game_status = STATUS_WAITING;
    }

    for (void **iterator = TCOD_list_begin(maps); iterator != TCOD_list_end(maps); iterator++)
    {
        map_t *map = *iterator;

        free(map);
    }

    TCOD_list_delete(maps);

    SDL_Quit();

    return 0;
}