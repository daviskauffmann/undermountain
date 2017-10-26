#ifndef INPUT_H
#define INPUT_H

typedef enum input_e {
    INPUT_NONE,
    INPUT_UPDATE,
    INPUT_REDRAW,
    INPUT_QUIT
} input_t;

input_t input_handle(void);

#endif