#include "print.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void console_print(
    TCOD_Console *const console,
    const int x, const int y,
    const TCOD_ColorRGB *const fg,
    const TCOD_ColorRGB *const bg,
    const TCOD_bkgnd_flag_t flag,
    const TCOD_alignment_t alignment,
    char *const fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    TCOD_console_vprintf(
        console,
        x, y,
        fg,
        bg,
        flag,
        alignment,
        fmt,
        args);

    va_end(args);
}
