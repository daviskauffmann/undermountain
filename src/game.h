#ifndef GAME_H
#define GAME_H

#include <libtcod/libtcod.h>

#include "actor.h"
#include "item.h"
#include "map.h"
#include "object.h"
#include "tile.h"

#define SAVE_PATH "save.gz"

#define NUM_MAPS 60

struct engine;

enum game_state
{
    GAME_STATE_PLAYING,
    GAME_STATE_LOSE
};

struct game
{
    enum game_state state;
    struct tile_common tile_common;
    struct tile_info tile_info[NUM_TILE_TYPES];
    struct object_common object_common;
    struct object_info object_info[NUM_OBJECT_TYPES];
    struct actor_common actor_common;
    struct race_info race_info[NUM_RACES];
    struct class_info class_info[NUM_CLASSES];
    struct prototype monster_prototypes[NUM_MONSTERS];
    struct item_common item_common;
    struct equip_slot_info equip_slot_info[NUM_EQUIP_SLOTS];
    struct base_item_info base_item_info[NUM_BASE_ITEMS];
    struct item_property_info item_property_info[NUM_ITEM_PROPERTIES];
    struct item_info item_info[NUM_ITEM_TYPES];
    struct map maps[NUM_MAPS];
    TCOD_list_t messages;
    struct actor *player;
    int turn;
    bool turn_available;
    bool should_update;
    bool should_restart;
};

struct game *game_create(void);
void game_new(struct game *game);
void game_save(struct game *game);
void game_load(struct game *game);
void game_update(struct game *game, struct engine *engine);
void game_log(struct game *game, int level, int x, int y, TCOD_color_t color, char *fmt, ...);
void game_destroy(struct game *game);

#endif
