#ifndef GAME_H
#define GAME_H

#include <libtcod/libtcod.h>

#include "actor.h"
#include "item.h"
#include "map.h"
#include "object.h"
#include "panel.h"
#include "tile.h"

#define NUM_MAPS 10

struct game
{
    struct map maps[NUM_MAPS];
    struct tile_common tile_common;
    struct tile_info tile_info[NUM_TILE_TYPES];
    struct object_common object_common;
    struct object_info object_info[NUM_OBJECT_TYPES];
    struct actor_common actor_common;
    struct actor_info actor_info[NUM_ACTOR_TYPES];
    struct item_common item_common;
    struct item_info item_info[NUM_ITEM_TYPES];
    struct actor *player;
    int turn;
    bool turn_available;
    bool should_update;
    bool should_restart;
    bool should_quit;
    bool game_over;
    TCOD_list_t messages;
    enum panel_type current_panel;
    struct panel_status panel_status[NUM_PANELS];
    bool message_log_visible;
    bool panel_visible;
};

struct game *game_create(void);
void game_new(struct game *game);
void game_save(struct game *game);
void game_load(struct game *game);
void game_input(struct game *game);
void game_update(struct game *game);
void game_render(struct game *game);
void game_log(struct game *game, int x, int y, TCOD_color_t color, char *text, ...);
void game_destroy(struct game *game);

#endif
