#ifndef ROGUELIKE_GAME_H
#define ROGUELIKE_GAME_H

#include "map.h"

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
    unsigned int turn;
};

extern struct game *game;

void game_init(void);
void game_new(void);
void game_save(const char *filename);
void game_load(const char *filename);
void game_update(void);
void game_turn(void);
void game_log(int floor, int x, int y, TCOD_color_t color, char *fmt, ...);
void game_quit(void);

#endif
