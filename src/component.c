#include <libtcod.h>

#include "CMemLeak.h"
#include "game.h"

component_t *component_add(entity_t *entity, component_type_t component_type)
{
    component_t *component = &components[component_type][entity->id];

    component->id = entity->id;

    return component;
}

component_t *component_get(entity_t *entity, component_type_t component_type)
{
    if (entity->id == ID_UNUSED)
    {
        return NULL;
    }

    component_t *component = &components[component_type][entity->id];

    if (component->id != ID_UNUSED)
    {
        return component;
    }

    return NULL;
}

void component_remove(entity_t *entity, component_type_t component_type)
{
    component_t *component = &components[component_type][entity->id];

    component->id = ID_UNUSED;
}