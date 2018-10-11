#ifndef INPUT_H
#define INPUT_H

#include "actor.h"

struct game;
struct ui;

enum directional_action
{
    DIRECTIONAL_ACTION_NONE,
    DIRECTIONAL_ACTION_CLOSE_DOOR,
    DIRECTIONAL_ACTION_DRINK,
    DIRECTIONAL_ACTION_OPEN_DOOR,
    DIRECTIONAL_ACTION_PRAY,
    DIRECTIONAL_ACTION_SIT
};

enum inventory_action
{
    INVENTORY_ACTION_NONE,
    INVENTORY_ACTION_EQUIP
};

struct input
{
    enum directional_action directional_action;
    enum inventory_action inventory_action;
    bool automoving;
    int automove_x;
    int automove_y;
};

struct input *input_create(void);
void input_handle(struct input *input, struct engine *engine, struct game *game, struct ui *ui);
void input_destroy(struct input *input);

#endif
