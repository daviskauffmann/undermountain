#ifndef ROGUELIKE_GAME_H
#define ROGUELIKE_GAME_H

#include "map.h"

#define SAVE_PATH "save.gz"
#define NUM_MAPS 60

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
};

extern struct game *game;

void game_init(void);
void game_new(void);
void game_save(void);
void game_load(void);
void game_update(void);
void game_turn(void);
void game_log(int floor, int x, int y, TCOD_color_t color, char *fmt, ...);
void game_quit(void);

#endif
