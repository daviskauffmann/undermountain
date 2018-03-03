#ifndef PANEL_H
#define PANEL_H

typedef enum panel_type_e panel_type_t;
typedef struct panel_info_s panel_info_t;

enum panel_type_e
{
    PANEL_INVENTORY,
    PANEL_CHARACTER,

    NUM_PANELS
};

struct panel_info_s
{
    int current;
    int scroll;
};

#endif
