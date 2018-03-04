#ifndef PANEL_H
#define PANEL_H

typedef enum panel_type_e
{
    PANEL_INVENTORY,
    PANEL_CHARACTER,

    NUM_PANELS
} panel_type_t;

typedef struct panel_status_s
{
    int current;
    int scroll;
} panel_status_t;

#endif
