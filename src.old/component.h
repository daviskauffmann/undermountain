#ifndef COMPONENT_H
#define COMPONENT_H

#include <libtcod/libtcod.h>

#include "spell.h"

typedef struct entity_s entity_t;

typedef struct ai_s
{
    float energy;
    float energy_per_turn;
    entity_t *follow_target;
    int last_seen_x;
    int last_seen_y;
} ai_t;

typedef enum alignment_type_e {
    ALIGNMENT_GOOD,
    ALIGNMENT_EVIL,
} alignment_type_t;

typedef struct alignment_s
{
    alignment_type_t type;
} alignment_t;

typedef enum appearance_layer_e {
    LAYER_0,
    LAYER_1,
    LAYER_2,

    NUM_LAYERS,
} appearance_layer_t;

typedef struct appearance_s
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
    appearance_layer_t layer;
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

typedef struct equipment_s
{
    equipment_slot_t slot;
} equipment_t;

typedef struct inventory_s
{
    TCOD_list_t items;
    entity_t *equipment[NUM_EQUIPMENT_SLOTS];
} inventory_t;

typedef enum light_priority_e {
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

typedef struct projectile_s
{
    float x;
    float y;
    float dx;
    float dy;
    entity_t *shooter;
    void (*on_hit)(void *on_hit_params);
    void *on_hit_params;
} projectile_t;

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
    COMPONENT_PROJECTILE,
    COMPONENT_POSITION,
    COMPONENT_TARGETING,
    COMPONENT_SOLID,

    NUM_COMPONENTS
} component_type_t;

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

typedef struct component_s
{
    component_data_t data;
    int id;
    component_type_t type;
} component_t;

void component_init(component_t *component, int id, component_type_t component_type);
component_t *component_add(entity_t *entity, component_type_t component_type);
component_t *component_get(entity_t *entity, component_type_t component_type);
void component_remove(entity_t *entity, component_type_t component_type);
void component_reset(component_t *component);

#endif
