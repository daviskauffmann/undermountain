#ifndef INPUT_H
#define INPUT_H

#include "actor.h"

struct game;
struct ui;

enum targeting
{
    TARGETING_NONE,
    TARGETING_LOOK,
    TARGETING_SHOOT,
    TARGETING_SPELL
};

struct input
{
    enum action action;
    enum targeting targeting;
    int target_x;
    int target_y;
};

struct input *input_create(void);
void input_handle(struct input *input, struct game *game, struct ui *ui);
void input_destroy(struct input *input);

#endif
