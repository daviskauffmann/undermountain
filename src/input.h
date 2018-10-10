#ifndef INPUT_H
#define INPUT_H

#include "actor.h"

struct game;
struct ui;

enum action
{
    ACTION_NONE,
    ACTION_DESCEND,
    ACTION_ASCEND,
    ACTION_OPEN_DOOR,
    ACTION_CLOSE_DOOR,
    ACTION_PRAY,
    ACTION_DRINK,
    ACTION_SIT
};

enum targeting
{
    TARGETING_NONE,
    TARGETING_LOOK,
    TARGETING_EXAMINE,
    TARGETING_SHOOT,
    TARGETING_SPELL
};

struct input
{
    enum action action;
    enum targeting targeting;
    int target_x;
    int target_y;
    bool automoving;
    int automove_x;
    int automove_y;
};

struct input *input_create(void);
void input_handle(struct input *input, struct game *game, struct ui *ui);
void input_destroy(struct input *input);

#endif
