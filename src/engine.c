#include <malloc.h>

#include "engine.h"

struct engine *engine_create(void)
{
    struct engine *engine = calloc(1, sizeof(struct engine));

    engine->state = ENGINE_STATE_MENU;
    engine->should_quit = false;

    return engine;
}

void engine_destroy(struct engine *engine)
{
    free(engine);
}
