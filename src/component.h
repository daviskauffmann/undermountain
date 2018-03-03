#ifndef COMPONENT_H
#define COMPONENT_H

#include <libtcod/libtcod.h>

#include "spell.h"

typedef struct entity_s entity_t;

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
typedef struct projectile_s projectile_t;
typedef struct position_s position_t;
typedef enum targeting_type_e targeting_type_t;
typedef struct targeting_s targeting_t;
typedef enum component_type_e component_type_t;
typedef struct component_s component_t;

struct ai_s
{
    float energy;
    float energy_per_turn;
    entity_t *follow_target;
    int last_seen_x;
    int last_seen_y;
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

struct projectile_s
{
    float x;
    float y;
    float dx;
    float dy;
    entity_t *shooter;
    void (*on_hit)(void *on_hit_params);
    void *on_hit_params;
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
    COMPONENT_PROJECTILE,
    COMPONENT_POSITION,
    COMPONENT_TARGETING,
    COMPONENT_SOLID,

    NUM_COMPONENTS
};

typedef union component_data_u {
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
    projectile_t projectile;
    position_t position;
    targeting_t targeting;
} component_data_t;

struct component_s
{
    component_data_t data;
    int id;
    component_type_t type;
};

void component_init(component_t *component, int id, component_type_t component_type);
component_t *component_add(entity_t *entity, component_type_t component_type);
component_t *component_get(entity_t *entity, component_type_t component_type);
void component_remove(entity_t *entity, component_type_t component_type);
void component_reset(component_t *component);

#endif
