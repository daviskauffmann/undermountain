#ifndef GAME_H
#define GAME_H

#include <libtcod/libtcod.h>

#include "map.h"

#define SAVE_PATH "save.gz"

#define NUM_MAPS 60

struct program;

enum game_state
{
    GAME_STATE_PLAY,
    GAME_STATE_WAIT,
    GAME_STATE_LOSE
};

struct game
{
    enum game_state state;
    struct map maps[NUM_MAPS];
    TCOD_list_t messages;
    struct actor *player;
    int turn;
    bool should_update;
};

extern struct game *game;

void game_init(void);
void game_new(void);
void game_save();
void game_load(void);
void game_update(void);
void game_log(int level, int x, int y, TCOD_color_t color, char *fmt, ...);
void game_restart(void);
void game_quit(void);

#endif
