#ifndef GAME_H
#define GAME_H

#include <libtcod/libtcod.h>

#include "component.h"
#include "entity.h"
#include "map.h"
#include "panel.h"
#include "tile.h"

typedef struct game_s
{
    tile_common_t tile_common;
    tile_info_t tile_info[NUM_TILES];
    map_t maps[NUM_MAPS];
    entity_t entities[NUM_ENTITIES];
    component_t components[NUM_COMPONENTS][NUM_ENTITIES];
    entity_t *player;
    TCOD_list_t messages;
    panel_type_t current_panel;
    panel_status_t panel_status[NUM_PANELS];
    int turn;
    bool turn_available;
    bool should_update;
    bool should_restart;
    bool should_quit;
    bool game_over;
    bool message_log_visible;
    bool panel_visible;
} game_t;

void game_run(void);
void game_init(game_t *game);
void game_new(game_t *game);
void game_save(game_t *game);
void game_load(game_t *game);
void game_input(game_t *game);
void game_update(game_t *game);
void game_render(game_t *game);
void game_log(game_t *game, position_t *position, TCOD_color_t color, char *text, ...);
void game_reset(game_t *game);

#endif
