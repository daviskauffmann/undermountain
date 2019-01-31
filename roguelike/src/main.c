#include <stdio.h>
#include <string.h>

#include <platform.h>

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
            printf("%s v%s\n", TITLE, VERSION);
        }
    }

    return platform_run();
}
