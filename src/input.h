#ifndef INPUT_H
#define INPUT_H

typedef enum {
    INPUT_FALSE,
    INPUT_TRUE,
    INPUT_QUIT
} InputType;

InputType input_handle();

#endif