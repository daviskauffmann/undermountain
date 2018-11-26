#include <platform/platform.h>

// TODO: remove all platform specific code from the game

int main(int argc, char *args[])
{
    for (int i = 1; i < argc; i++)
    {
        // TODO: process command line args
    }

    return platform_run();
}
