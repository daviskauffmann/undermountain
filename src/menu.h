#ifndef MENU_H
#define MENU_H

typedef enum menu_input_e {
    MENU_INPUT_NONE,
    MENU_INPUT_START,
    MENU_INPUT_LOAD,
    MENU_INPUT_QUIT
} menu_input_t;

menu_input_t menu_input(void);

#endif