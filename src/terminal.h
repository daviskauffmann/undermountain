#ifndef UM_TERMINAL_H
#define UM_TERMINAL_H

#include "game/color.h"
#include <stdbool.h>

bool terminal_init(void);
void terminal_shutdown(void);
void terminal_set_color(struct color foreground, struct color background, int attributes);
void terminal_reset_color(void);

#endif
