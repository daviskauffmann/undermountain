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
    bool should_restart;
};

struct game *game_create(void);
void game_new(struct game *game);
void game_save(struct game *game);
void game_load(struct game *game);
void game_update(struct game *game, struct program *program);
void game_log(struct game *game, int level, int x, int y, TCOD_color_t color, char *fmt, ...);
void game_destroy(struct game *game);

#endif
