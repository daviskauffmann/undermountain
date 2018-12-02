#ifndef PLATFORM_INPUT_H
#define PLATFORM_INPUT_H

// TODO: prompts

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
    INVENTORY_ACTION_DROP,
    INVENTORY_ACTION_EQUIP,
    INVENTORY_ACTION_EXAMINE,
    INVENTORY_ACTION_QUAFF
};

enum character_action
{
    CHARACTER_ACTION_NONE,
    CHARACTER_ACTION_EXAMINE,
    CHARACTER_ACTION_UNEQUIP
};

struct input
{
    enum directional_action directional_action;
    enum inventory_action inventory_action;
    enum character_action character_action;
    bool automoving;
    int automove_x;
    int automove_y;
    bool should_quit;
};

extern struct input *input;

void input_init(void);
void input_handle(void);
void input_quit(void);

#endif
