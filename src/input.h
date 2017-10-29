#ifndef INPUT_H
#define INPUT_H

typedef enum input_e {
    INPUT_TICK,
    INPUT_TURN,
    INPUT_DRAW,
    INPUT_QUIT
} input_t;

input_t input_handle(void);

#endif