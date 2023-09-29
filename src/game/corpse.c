#include "corpse.h"

#include "color.h"
#include <malloc.h>
#include <string.h>

const struct corpse_metadata corpse_metadata = {
    .glyph = '%',
    .color = {COLOR_DARK_RED},
};

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
