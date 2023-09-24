#ifndef UM_SCENES_MENU_OPTION_H
#define UM_SCENES_MENU_OPTION_H

enum option
{
    OPTION_START,
    OPTION_QUIT,

    NUM_OPTIONS
};

struct option_data
{
    char *text;
};

#endif
