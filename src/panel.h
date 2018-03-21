#ifndef PANEL_H
#define PANEL_H

enum panel_type
{
    PANEL_INVENTORY,
    PANEL_CHARACTER,

    NUM_PANELS
};

struct panel_status
{
    int current;
    int scroll;
};

#endif
