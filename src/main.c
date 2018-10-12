#include "config.h"
#include "engine.h"

int main(void)
{
    config_load();

    struct engine *engine = engine_create();

    int error = engine_run(engine);

    engine_destroy(engine);

    config_save();

    return error;
}
