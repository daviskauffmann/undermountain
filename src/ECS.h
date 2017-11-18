#ifndef ECS_H
#define ECS_H

#include <libtcod.h>

void ECS_init(void);
void ECS_reset(void);

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
void entity_move(entity_t *entity, int x, int y);
void entity_swing(entity_t *entity, int x, int y);
void entity_attack(entity_t *entity, entity_t *other);
void entity_destroy(entity_t *entity);

/* Components */
typedef enum component_type_e {
    COMPONENT_POSITION,
    COMPONENT_PHYSICS,
    COMPONENT_LIGHT,
    COMPONENT_FOV,
    COMPONENT_APPEARANCE,
    COMPONENT_AI,

    NUM_COMPONENTS
} component_type_t;

typedef struct position_s
{
    struct map_s *map;
    int x;
    int y;
    int next_x;
    int next_y;
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
        ai_t ai;
        physics_t physics;
        light_t light;
        fov_t fov;
        appearance_t appearance;
    };
} component_t;

component_t components[NUM_COMPONENTS][MAX_ENTITIES];

component_t *component_add(entity_t *entity, component_type_t component_type);
component_t *component_get(entity_t *entity, component_type_t component_type);
void component_remove(entity_t *entity, component_type_t component_type);

/* Systems */
void input_system(void);
void ai_system(void);
void movement_system(void);
void lighting_system(void);
void fov_system(void);
void render_system(void);

#endif