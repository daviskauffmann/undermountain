#include <platform/platform.h>

struct tooltip_option *tooltip_option_create(char *text, struct tooltip_data data, bool(*fn)(struct tooltip_data data))
{
    struct tooltip_option *tooltip_option = malloc(sizeof(struct tooltip_option));

    tooltip_option->text = _strdup(text);
    tooltip_option->tooltip_data = data;
    tooltip_option->fn = fn;

    return tooltip_option;
}

void tooltip_option_destroy(struct tooltip_option *tooltip_option)
{
    free(tooltip_option->text);
    free(tooltip_option);
}
