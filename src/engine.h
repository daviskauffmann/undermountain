#ifndef ENGINE_H
#define ENGINE_H

#include <libtcod/libtcod.h>

enum engine_state
{
    ENGINE_STATE_MENU,
    ENGINE_STATE_PLAY
};

struct engine
{
    enum engine_state state;
    bool should_quit;
};

struct engine *engine_create(void);
void engine_destroy(struct engine *engine);

#endif
