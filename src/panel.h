#ifndef PANEL_H
#define PANEL_H

enum panel_type
{
    PANEL_CHARACTER,
    PANEL_INVENTORY,
    PANEL_SPELLBOOK,

    NUM_PANELS
};

struct panel_status
{
    int current;
    int scroll;
};

#endif
