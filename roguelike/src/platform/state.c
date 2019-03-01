#include <platform/platform.h>

// TODO: keep states active in memory when switching?

struct state *state;

void state_set(struct state *new_state)
{
    if (state)
    {
        state->quit();
    }

    state = new_state;

    if (state)
    {
        state->init();
    }
}
