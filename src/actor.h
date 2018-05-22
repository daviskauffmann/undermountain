#ifndef ACTOR_H
#define ACTOR_H

#include <libtcod/libtcod.h>

struct object;
struct item;

enum race
{
    RACE_HUMAN,
    RACE_ELF,
    RACE_DWARF,
    RACE_ORC,
    RACE_BUGBEAR,
    RACE_ZOMBIE,

    NUM_RACES
};

enum class
{
    CLASS_FIGHTER,
    CLASS_RANGER,
    CLASS_WIZARD,
    CLASS_CLERIC,

    NUM_CLASSES
};

enum faction
{
    FACTION_GOOD,
    FACTION_EVIL,

    NUM_FACTIONS
};

enum action
{
    ACTION_NONE,
    ACTION_DESCEND,
    ACTION_ASCEND,
    ACTION_OPEN_DOOR,
    ACTION_CLOSE_DOOR
};

enum targeting
{
    TARGETING_NONE,
    TARGETING_LOOK,
    TARGETING_SHOOT,
    TARGETING_SPELL
};

struct actor_common
{
    int glow_radius;
    TCOD_color_t glow_color;
    int torch_radius;
    TCOD_color_t torch_color;
};

struct race_info
{
    const char *name;
    unsigned char glyph;
    float energy_per_turn;
};

struct class_info
{
    const char *name;
    TCOD_color_t color;
};

struct actor
{
    struct game *game;
    enum race race;
    enum class class;
    enum faction faction;
    int level;
    int x;
    int y;
    int health;
    float energy;
    int last_seen_x;
    int last_seen_y;
    bool glow;
    TCOD_map_t glow_fov;
    bool torch;
    TCOD_map_t torch_fov;
    TCOD_map_t fov;
    TCOD_list_t items;
    TCOD_color_t flash_color;
    float flash_fade;
    bool dead;
};

struct actor *actor_create(struct game *game, enum race race, enum class class, enum faction faction, int level, int x, int y);
const char *actor_get_name(struct actor *actor);
void actor_update_flash(struct actor *actor);
void actor_calc_light(struct actor *actor);
void actor_calc_fov(struct actor *actor);
void actor_ai(struct actor *actor);
bool actor_path_towards(struct actor *actor, int x, int y);
bool actor_move_towards(struct actor *actor, int x, int y);
bool actor_move(struct actor *actor, int x, int y);
bool actor_swap(struct actor *actor, struct actor *other);
bool actor_interact(struct actor *actor, int x, int y, enum action action);
bool actor_open_door(struct actor *actor, int x, int y);
bool actor_close_door(struct actor *actor, int x, int y);
bool actor_descend(struct actor *actor);
bool actor_ascend(struct actor *actor);
bool actor_grab(struct actor *actor, int x, int y);
bool actor_drop(struct actor *actor, struct item *item);
bool actor_bash(struct actor *actor, struct object *object);
bool actor_shoot(struct actor *actor, int x, int y, void (*on_hit)(void *on_hit_params), void *on_hit_params);
bool actor_swing(struct actor *actor, int x, int y);
bool actor_attack(struct actor *actor, struct actor *other);
bool actor_cast_spell(struct actor *actor);
void actor_die(struct actor *actor, struct actor *killer);
void actor_destroy(struct actor *actor);

#endif
