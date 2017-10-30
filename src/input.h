#ifndef INPUT_H
#define INPUT_H

#define AUTOMOVE_DELAY 0.1f

typedef enum input_e {
    INPUT_TICK,
    INPUT_TURN,
    INPUT_DRAW,
    INPUT_QUIT
} input_t;

input_t input_handle(void);

#endif