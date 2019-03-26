#ifndef PLATFORM_STATE_H
#define PLATFORM_STATE_H

struct state
{
    void(*init)(struct state *previous_state);
    struct state *(*handleEvent)(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse);
    struct state *(*update)(float delta);
    void(*render)(TCOD_console_t console);
    void(*quit)(void);
};

#endif
