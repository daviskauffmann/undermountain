#include "corpse.h"

#include <malloc.h>
#include <string.h>

struct corpse *corpse_new(
    const char *const name,
    const int level,
    const int floor,
    const int x,
    const int y)
{
    struct corpse *const corpse = malloc(sizeof(*corpse));

    corpse->name = strdup(name);

    corpse->level = level;

    corpse->floor = floor;
    corpse->x = x;
    corpse->y = y;

    return corpse;
}

void corpse_delete(struct corpse *const corpse)
{
    free(corpse->name);

    free(corpse);
}
