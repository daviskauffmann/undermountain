#include <stdio.h>
#include <string.h>

#include <platform/platform.h>

int main(int argc, char *args[])
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            printf("Options:\n");
            printf("  -h, --help\tPrint this message\n");
            printf("  -v, --version\tPrint current version\n");
        }

        if (strcmp(args[i], "-v") == 0 || strcmp(args[i], "--version") == 0)
        {
            printf("%s %s\n", TITLE, VERSION);
            printf("%s\n", TCOD_STRVERSIONNAME);
        }
    }

    return platform_run();
}
