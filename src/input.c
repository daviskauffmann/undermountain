#include <libtcod.h>
#include "entity.h"
#include "input.h"
#include "map.h"

InputType input_handle(Map *map, Entity *player)
{
    TCOD_key_t key;
    TCOD_mouse_t mouse;
    TCOD_event_t ev = TCOD_sys_check_for_event(TCOD_EVENT_ANY, &key, &mouse);

    if (ev == TCOD_EVENT_KEY_PRESS)
    {
        switch (key.vk)
        {
        case TCODK_ESCAPE:
            return INPUT_QUIT;
        case TCODK_UP:
            entity_move(player, 0, -1);
            map_update(map);
            return INPUT_TRUE;
        case TCODK_LEFT:
            entity_move(player, -1, 0);
            map_update(map);
            return INPUT_TRUE;
        case TCODK_DOWN:
            entity_move(player, 0, 1);
            map_update(map);
            return INPUT_TRUE;
        case TCODK_RIGHT:
            entity_move(player, 1, 0);
            map_update(map);
            return INPUT_TRUE;
        }
    }

    return INPUT_FALSE;
}