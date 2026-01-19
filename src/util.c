#include "util.h"

#include <SDL3/SDL.h>
#include <stdio.h>

bool file_exists(const char *const filename)
{
    SDL_IOStream *const file = SDL_IOFromFile(filename, "r");

    if (!file)
    {
        return false;
    }

    SDL_CloseIO(file);

    return true;
}

void file_delete(const char *filename)
{
    remove(filename);
}
