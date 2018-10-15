#include <malloc.h>

#include "program.h"

struct program *program;

void program_init(void)
{
    program = calloc(1, sizeof(struct program));

    program->state = PROGRAM_STATE_MENU;
    program->should_quit = false;
}

void program_quit(void)
{
    free(program);
}
