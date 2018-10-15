#include <malloc.h>

#include "program.h"

struct program *program_create(void)
{
    struct program *program = calloc(1, sizeof(struct program));

    program->state = PROGRAM_STATE_MENU;
    program->should_quit = false;

    return program;
}

void program_destroy(struct program *program)
{
    free(program);
}
