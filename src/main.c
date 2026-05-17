#include "config.h"
#include "menu.h"
#include "scene.h"
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
        {
            printf("Options:\n");
            printf("  -c, --columns\tSet the width of the console in characters\n");
            printf("  -r, --rows\tSet the height of the console in characters\n");
            printf("  -h, --help\tPrint this message\n");
            printf("  -v, --version\tPrint current version\n");
            return 0;
        }
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
        {
            printf("%s\n", VERSION);
            return 0;
        }
    }

    const WINDOW *win = initscr();
    if (!win)
    {
        printf("Error: Couldn't initialize pdcurses\n");
        return 1;
    }

    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    curs_set(0);

    struct scene *current_scene = &menu_scene;
    current_scene->init(NULL);

    clock_t previous_time = clock();

    while (current_scene)
    {
        clock_t current_time = clock();
        float delta_time = (float)(current_time - previous_time) / CLOCKS_PER_SEC;
        previous_time = current_time;

        struct scene *next_scene = current_scene->update(delta_time);

        if (!next_scene)
        {
            break;
        }

        if (next_scene != current_scene)
        {
            if (current_scene)
            {
                current_scene->uninit();
            }

            next_scene->init(current_scene);

            current_scene = next_scene;
        }

        napms(16);
    }

    endwin();

    return 0;
}
