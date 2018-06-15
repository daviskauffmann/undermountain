#ifndef ACTOR_H
#define ACTOR_H

#include <libtcod/libtcod.h>

struct object;
struct item;

enum race
{
    // player races
    RACE_DWARF,
    RACE_ELF,
    RACE_GNOME,
    RACE_HALF_ELF,
    RACE_HALF_ORC,
    RACE_HALFLING,
    RACE_HUMAN,

    // monster races
    RACE_ABBERATION,
    RACE_ANIMAL,
    RACE_BEAST,
    RACE_CONSTRUCT,
    RACE_DRAGON,
    RACE_ELEMENTAL,
    RACE_FEY,
    RACE_GIANT,
    RACE_GOBLINOID,
    RACE_MAGICAL_BEAST,
    RACE_MONSTROUS_HUMANOID,
    RACE_OOZE,
    RACE_ORC,
    RACE_OUTSIDER,
    RACE_REPTILLIAN,
    RACE_SHAPECHANGER,
    RACE_UNDEAD,
    RACE_VERMIN,

    NUM_RACES
};

enum class
{
    // player classes
    CLASS_BARBARIAN,
    CLASS_BARD,
    CLASS_CLERIC,
    CLASS_DRUID,
    CLASS_FIGHTER,
    CLASS_MONK,
    CLASS_PALADIN,
    CLASS_RANGER,
    CLASS_ROGUE,
    CLASS_SORCERER,
    CLASS_WIZARD,

    // monster classes
    CLASS_ABBERATION,
    CLASS_ANIMAL,
    CLASS_BEAST,
    CLASS_COMMONER,
    CLASS_CONSTRUCT,
    CLASS_DRAGON,
    CLASS_ELEMENTAL,
    CLASS_FEY,
    CLASS_GIANT,
    CLASS_HUMANOID,
    CLASS_MAGICAL_BEAST,
    CLASS_MONSTROUS,
    CLASS_OOZE,
    CLASS_OUTSIDER,
    CLASS_SHAPESHIFTER,
    CLASS_UNDEAD,
    CLASS_VERMIN,

    NUM_CLASSES
};

enum monster
{
    MONSTER_BEHOLDER,
    MONSTER_GOBLIN,
    MONSTER_ORC_FIGHTER,
    MONSTER_ORC_CLERIC,

    NUM_MONSTERS
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

struct prototype
{
    const char *name;
    enum race race;
    enum class class;
};

struct actor
{
    struct game *game;
    const char *name;
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
    int kills;
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

struct actor *actor_create(struct game *game, const char *name, enum race race, enum class class, enum faction faction, int level, int x, int y);
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
