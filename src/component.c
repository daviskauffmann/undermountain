#include <libtcod/libtcod.h>

#include "component.h"
#include "entity.h"
#include "game.h"

void component_init(component_t *component, int id, component_type_t component_type)
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
        ai->last_seen_x = -1;
        ai->last_seen_y = -1;
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
            // FIXME: why
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
    case COMPONENT_PROJECTILE:
    {
        projectile_t *projectile = (projectile_t *)component;

        projectile->x = 0.0f;
        projectile->y = 0.0f;
        projectile->dx = 0.0f;
        projectile->dy = 0.0f;
        projectile->shooter = NULL;
        projectile->on_hit = NULL;
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

component_t *component_add(entity_t *entity, component_type_t component_type)
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

component_t *component_get(entity_t *entity, component_type_t component_type)
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

void component_remove(entity_t *entity, component_type_t component_type)
{
    if (entity && entity->id != ID_UNUSED)
    {
        component_t *component = &entity->game->components[component_type][entity->id];

        component_reset(component);
    }
}

void component_reset(component_t *component)
{
    component->id = ID_UNUSED;

    switch (component->type)
    {
    case COMPONENT_AI:
    {
        // ai_t *ai = (ai_t *)component;
    }
    break;
    case COMPONENT_ALIGNMENT:
    {
        // alignment_t *alignment = (alignment_t *)component;

        break;
    }
    case COMPONENT_APPEARANCE:
    {
        // appearance_t *appearance = (appearance_t *)component;
    }
    break;
    case COMPONENT_FLASH:
    {
        // flash_t *flash = (flash_t *)component;
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
        // health_t *health = (health_t *)component;
    }
    break;
    case COMPONENT_INVENTORY:
    {
        inventory_t *inventory = (inventory_t *)component;

        TCOD_list_clear(inventory->items);
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
        // pickable_t *pickable = (pickable_t *)component;
    }
    break;
    case COMPONENT_PROJECTILE:
    {
        // projectile_t *projectile = (projectile_t *)component;
    }
    break;
    case COMPONENT_POSITION:
    {
        // position_t *position = (position_t *)component;
    }
    break;
    case COMPONENT_TARGETING:
    {
        // targeting_t *targeting = (targeting_t *)component;
    }
    break;
    case COMPONENT_SOLID:
    {
    }
    break;
    }
}
