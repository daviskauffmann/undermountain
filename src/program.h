#ifndef PROGRAM_H
#define PROGRAM_H

#include <libtcod/libtcod.h>

enum program_state
{
    PROGRAM_STATE_MENU,
    PROGRAM_STATE_GAME
};

struct program
{
    enum program_state state;
    bool should_quit;
};

struct program *program_create(void);
void program_destroy(struct program *program);

#endif
