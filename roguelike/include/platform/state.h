#ifndef PLATFORM_STATE_H
#define PLATFORM_STATE_H

struct state
{
    void(*init)(void);
    bool(*handleEvent)(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse);
    void(*update)(void);
    void(*render)(void);
    void(*quit)(void);
};

extern struct state *state;

void state_set(struct state *new_state);

#endif
