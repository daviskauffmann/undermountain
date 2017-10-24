#ifndef INPUT_H
#define INPUT_H

#include "world.h"

typedef enum {
    INPUT_NONE,
    INPUT_UPDATE,
    INPUT_RESTART,
    INPUT_LOAD,
    INPUT_QUIT
} input_t;

input_t input_handle(world_t *world);

#endif