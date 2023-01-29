#include "corpse.h"

#include <malloc.h>
#include <string.h>

struct corpse *corpse_new(
    const char *const name,
    const uint8_t level,
    const uint8_t floor,
    const uint8_t x,
    const uint8_t y)
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
