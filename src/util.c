#include "util.h"

#include <SDL2/SDL.h>
#include <stdio.h>

bool file_exists(const char *const filename)
{
    SDL_RWops *const file = SDL_RWFromFile(filename, "r");

    if (!file)
    {
        return false;
    }

    SDL_RWclose(file);

    return true;
}

void file_delete(const char *filename)
{
    remove(filename);
}
