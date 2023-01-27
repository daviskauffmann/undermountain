#ifndef SCENES_GAME_TOOLTIP_H
#define SCENES_GAME_TOOLTIP_H

#include <stdbool.h>

struct tooltip_option
{
    char *text;
    bool (*on_click)(void);
};

struct tooltip_option *tooltip_option_new(char *text, bool (*on_click)(void));
void tooltip_option_delete(struct tooltip_option *tooltip_option);

#endif
