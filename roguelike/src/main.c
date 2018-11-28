#include <stdio.h>
#include <string.h>

#include <platform/platform.h>

int main(int argc, char *args[])
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0)
        {
            printf("Command      Shortcut    Description\n");
            printf("--help       -h          Print this message\n");
            printf("--version    -v          Print current version\n");
        }

        if (strcmp(args[i], "--version") == 0 || strcmp(args[i], "-v") == 0)
        {
            printf("%s v%s\n", TITLE, VERSION);
        }
    }

    return platform_run();
}
