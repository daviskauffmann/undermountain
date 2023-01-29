#include "tooltip_option.h"

#include <malloc.h>
#include <string.h>

struct tooltip_option *tooltip_option_new(char *const text, bool (*const on_click)(void))
{
    struct tooltip_option *const tooltip_option = malloc(sizeof(*tooltip_option));

    tooltip_option->text = strdup(text);
    tooltip_option->on_click = on_click;

    return tooltip_option;
}

void tooltip_option_delete(struct tooltip_option *tooltip_option)
{
    free(tooltip_option->text);

    free(tooltip_option);
}
