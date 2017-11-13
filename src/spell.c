#include <libtcod.h>

#include "game.h"

void spell_instakill(cast_data_t data)
{
    tile_t *tile = &data.caster->map->tiles[data.x][data.y];

    if (tile->actor != NULL)
    {
        msg_log("actor instakills actor", data.caster->map, data.caster->x, data.caster->y);

        tile->actor->mark_for_delete = true;
    }
}