#ifndef GAME_ACTOR_H
#define GAME_ACTOR_H

#include <libtcod.h>

#include "item.h"

struct object;

enum race
{
    // player races
    RACE_DWARF,
    RACE_ELF,
    RACE_HUMAN,

    // monster races
    RACE_ANIMAL,
    RACE_BUGBEAR,
    RACE_ORC,
    RACE_SLIME,

    NUM_RACES
};

enum class
{
    // player classes
    CLASS_MAGE,
    CLASS_ROGUE,
    CLASS_WARRIOR,

    // monster classes
    CLASS_ANIMAL,
    CLASS_SLIME,

    NUM_CLASSES
};

enum monster
{
    MONSTER_BUGBEAR,
    MONSTER_ORC,
    MONSTER_RAT,
    MONSTER_SLIME,

    NUM_MONSTERS
};

enum faction
{
    FACTION_GOOD,
    FACTION_EVIL,

    NUM_FACTIONS
};

struct actor_common
{
    int turns_to_chase;
    int glow_radius;
    TCOD_color_t glow_color;
    float glow_intensity;
    int torch_radius;
    TCOD_color_t torch_color;
    float torch_intensity;
};

struct race_datum
{
    const char *name;
    unsigned char glyph;
};

struct class_datum
{
    const char *name;
    TCOD_color_t color;
};

struct actor_prototype
{
    const char *name;
    enum race race;
    enum class class;
    // TODO: level + stats
    // TODO: base equipment + inventory
};

struct actor
{
    char *name;
    enum race race;
    enum class class;
    enum faction faction;
    int level;
    int experience;
    int max_hp;
    int current_hp;
    struct item *equipment[NUM_EQUIP_SLOTS];
    TCOD_list_t items;
    int floor;
    int x;
    int y;
    TCOD_map_t fov;
    int last_seen_x;
    int last_seen_y;
    int turns_chased;
    struct actor *leader;
    bool glow;
    TCOD_map_t glow_fov;
    bool torch;
    TCOD_map_t torch_fov;
    TCOD_color_t flash_color;
    float flash_fade_coef;
    bool dead;
};

struct actor *actor_new(const char *name, enum race race, enum class class, enum faction faction, int level, int floor, int x, int y);
void actor_delete(struct actor *actor);
int actor_calc_experience_to_level(int level);
void actor_update_flash(struct actor *actor, float delta_time);
void actor_calc_light(struct actor *actor);
void actor_calc_fov(struct actor *actor);
void actor_ai(struct actor *actor);
void actor_give_experience(struct actor *actor, int experience);
void actor_level_up(struct actor *actor);
bool actor_path_towards(struct actor *actor, int x, int y);
bool actor_move_towards(struct actor *actor, int x, int y);
bool actor_move(struct actor *actor, int x, int y);
bool actor_swap(struct actor *actor, struct actor *other);
bool actor_open_door(struct actor *actor, int x, int y);
bool actor_close_door(struct actor *actor, int x, int y);
bool actor_descend(struct actor *actor, bool with_leader, void ***iterator);
bool actor_ascend(struct actor *actor, bool with_leader, void ***iterator);
bool actor_open_chest(struct actor *actor, int x, int y);
bool actor_pray(struct actor *actor, int x, int y);
bool actor_drink(struct actor *actor, int x, int y);
bool actor_sit(struct actor *actor, int x, int y);
bool actor_grab(struct actor *actor, int x, int y);
bool actor_drop(struct actor *actor, struct item *item);
bool actor_equip(struct actor *actor, struct item *item);
bool actor_unequip(struct actor *actor, enum equip_slot equip_slot);
bool actor_quaff(struct actor *actor, struct item *item);
bool actor_bash(struct actor *actor, struct object *object);
bool actor_shoot(struct actor *actor, int x, int y, void (*on_hit)(void *on_hit_params), void *on_hit_params);
bool actor_attack(struct actor *actor, struct actor *other, struct item *ammunition);
bool actor_cast_spell(struct actor *actor, int x, int y);
void actor_die(struct actor *actor, struct actor *killer);

#endif
