#ifndef UM_PRINT_H
#define UM_PRINT_H

#include <libtcod.h>

void console_print(
    TCOD_Console *console,
    int x, int y,
    const TCOD_ColorRGB *fg,
    const TCOD_ColorRGB *bg,
    TCOD_bkgnd_flag_t flag,
    TCOD_alignment_t alignment,
    char *fmt, ...);

#endif
