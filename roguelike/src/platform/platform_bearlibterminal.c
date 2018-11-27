#ifdef PLATFORM_BEARLIBTERMINAL

#include <platform/bearlibterminal/bearlibterminal.h>

int console_width = 80;
int console_height = 50;

int platform_run(void)
{
    terminal_open();

    // Printing text
    terminal_print(1, 1, "Hello, world!");
    terminal_refresh();

    // Wait until user close the window
    while (terminal_read() != TK_CLOSE);

    terminal_close();

    return 0;
}

#endif
