#ifndef UM_SCENE_H
#define UM_SCENE_H

struct scene
{
    void (*init)(const struct scene *previous_scene);
    void (*uninit)(void);
    struct scene *(*update)(float delta_time);
};

#endif
