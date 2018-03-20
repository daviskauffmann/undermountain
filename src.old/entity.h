#ifndef ENTITY_H
#define ENTITY_H

#include <libtcod/libtcod.h>

#define NUM_ENTITIES 65536
#define ID_UNUSED -1

typedef struct game_s game_t;
typedef struct tile_s tile_t;

typedef struct entity_s
{
    int id;
    game_t *game;
} entity_t;

typedef enum action_e {
    ACTION_NONE,
    ACTION_DESCEND,
    ACTION_ASCEND,
    ACTION_OPEN_DOOR,
    ACTION_CLOSE_DOOR
} action_t;

void entity_init(entity_t *entity, int id, game_t *game);
entity_t *entity_create(game_t *game);
void entity_map_place(entity_t *entity);
void entity_map_remove(entity_t *entity);
void entity_update_flash(entity_t *entity, bool flash_blocks_turn);
void entity_update_projectile(entity_t *entity);
void entity_update_light(entity_t *entity);
void entity_update_fov(entity_t *entity, TCOD_list_t lights);
void entity_ai(entity_t *entity);
void entity_path_towards(entity_t *entity, int x, int y);
void entity_move_towards(entity_t *entity, int x, int y);
bool entity_move(entity_t *entity, int x, int y);
bool entity_interact(entity_t *entity, int x, int y, action_t action);
bool entity_descend(entity_t *entity);
bool entity_ascend(entity_t *entity);
bool entity_close_door(entity_t *entity, tile_t *tile);
bool entity_open_door(entity_t *entity, tile_t *tile);
void entity_swap(entity_t *entity, entity_t *other);
void entity_pick(entity_t *entity, entity_t *other);
bool entity_swing(entity_t *entity, int x, int y);
void entity_shoot(entity_t *entity, int x, int y, void (*on_hit)(void *on_hit_params), void *on_hit_params);
void entity_attack(entity_t *entity, entity_t *other);
void entity_cast_spell(entity_t *entity);
void entity_die(entity_t *entity, entity_t *killer);
void entity_destroy(entity_t *entity);
void entity_reset(entity_t *entity);

#endif
