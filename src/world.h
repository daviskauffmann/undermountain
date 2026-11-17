#ifndef UM_GAME_WORLD_H
#define UM_GAME_WORLD_H

#include "map.h"
#include "event.h"
#include <libtcod.h>

#define NUM_MAPS 40

struct world
{
    size_t time;

    struct list *spawned_unique_item_types;

    struct map maps[NUM_MAPS];

    struct actor *player;

    struct actor *hero;
    bool doomed;

};

extern struct world *world;

void world_init(void);
void world_uninit(void);

void world_create(struct actor *hero, unsigned int seed);

void world_save(FILE *file);
bool world_load(FILE *file);

void world_update(float delta_time);

bool world_can_player_take_turn(void);

bool world_event_is_visible(const struct game_event *event);

#endif
