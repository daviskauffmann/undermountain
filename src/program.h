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

extern struct program *program;

void program_init(void);
void program_quit(void);

#endif
