#ifndef TOOLTIP_OPTION_H
#define TOOLTIP_OPTION_H

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
    struct tooltip_data tooltip_data;
    bool (*fn)(struct tooltip_data tooltip_data);
};

struct tooltip_option *tooltip_option_create(char *text, struct tooltip_data tooltip_data, bool (*fn)(struct tooltip_data tooltip_data));
void tooltip_option_destroy(struct tooltip_option *tooltip_option);

#endif
