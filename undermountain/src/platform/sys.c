#include <platform/platform.h>

bool file_exists(const char *filename)
{
    FILE *file;
    if ((file = fopen(filename, "r")))
    {
        fclose(file);

        return true;
    }
    else
    {
        return false;
    }
}
