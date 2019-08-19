#ifndef PLATFORM_SCENE_H
#define PLATFORM_SCENE_H

#include <libtcod.h>

struct scene
{
    void(*init)(struct scene *previous_scene);
    struct scene *(*handle_event)(TCOD_event_t ev, TCOD_key_t key, TCOD_mouse_t mouse);
    struct scene *(*update)(float delta);
    void(*render)(TCOD_console_t console);
    void(*quit)(void);
};

#endif
