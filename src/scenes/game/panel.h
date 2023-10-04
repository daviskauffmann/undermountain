#ifndef UM_SCENES_GAME_PANEL_H
#define UM_SCENES_GAME_PANEL_H

enum panel
{
    PANEL_CHARACTER,
    PANEL_EXAMINE,
    PANEL_HELP,
    PANEL_INVENTORY,
    PANEL_SPELLBOOK,

    NUM_PANELS
};

struct panel_state
{
    int scroll;
};

#endif
