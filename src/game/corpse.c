#include "corpse.h"

#include <assert.h>
#include <malloc.h>

struct corpse *corpse_new(const char *name, int level, int floor, int x, int y)
{
    struct corpse *corpse = malloc(sizeof(*corpse));
    assert(corpse);
    corpse->name = TCOD_strdup(name);
    corpse->level = level;
    corpse->floor = floor;
    corpse->x = x;
    corpse->y = y;
    return corpse;
}

void corpse_delete(struct corpse *corpse)
{
    free(corpse->name);
    free(corpse);
}
