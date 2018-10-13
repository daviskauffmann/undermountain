#ifndef TOOLTIP_OPTION_H
#define TOOLTIP_OPTION_H

struct game;
struct input;

struct tooltip_data
{
    int x;
    int y;
    struct item *item;
    enum equip_slot equip_slot;
};

struct tooltip_option
{
    char *text;
    bool (*fn)(struct game *game, struct input *input, struct tooltip_data data);
    struct tooltip_data data;
};

struct tooltip_option *tooltip_option_create(char *text, bool (*fn)(struct game *game, struct input *input, struct tooltip_data data), struct tooltip_data data);
void tooltip_option_destroy(struct tooltip_option *tooltip_option);

#endif
