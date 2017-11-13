#include <libtcod.h>

#include "game.h"

void spell_instakill(actor_t *caster, int x, int y)
{
    tile_t *tile = &caster->map->tiles[x][y];

    if (tile->actor != NULL)
    {
        tile->actor->mark_for_delete = true;
    }
}