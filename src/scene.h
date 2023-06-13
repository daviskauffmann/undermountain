#ifndef SCENE_H
#define SCENE_H

#include <SDL2/SDL.h>
#include <libtcod.h>

struct scene
{
    void (*init)(const struct scene *previous_scene);
    void (*uninit)(void);

    struct scene *(*handle_event)(const SDL_Event *event);
    struct scene *(*update)(TCOD_Console *console, float delta_time);
};

#endif
