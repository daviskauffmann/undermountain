#include <stdio.h>
#include <string.h>

#include "platform/config.h"
#include "platform/platform.h"

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printf("Options:\n");
            printf("  -h, --help\tPrint this message\n");
            printf("  -v, --version\tPrint current version\n");
        }

        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            printf("%s %s\n", TITLE, VERSION);
        }
    }

    return platform_run();
}
