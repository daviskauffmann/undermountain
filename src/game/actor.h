#ifndef ACTOR_H
#define ACTOR_H

#include "item.h"
#include "spell.h"
#include <libtcod.h>

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

enum faction
{
    FACTION_ADVENTURER,
    FACTION_MONSTER
};

enum monster
{
    MONSTER_BUGBEAR,
    MONSTER_ORC,
    MONSTER_RAT,
    MONSTER_SLIME,

    NUM_MONSTERS
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
    float speed;
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

    uint8_t level;
    int experience;

    int health;
    int mana;

    int gold;
    struct item *equipment[NUM_EQUIP_SLOTS];
    TCOD_list_t items;
    TCOD_list_t known_spell_types;
    enum spell_type readied_spell_type;

    uint8_t floor;
    uint8_t x;
    uint8_t y;

    TCOD_map_t fov;

    bool took_turn;
    float energy;

    int last_seen_x;
    int last_seen_y;
    int turns_chased;

    struct actor *leader;

    int light_radius;
    TCOD_color_t light_color;
    float light_intensity;
    bool light_flicker;
    TCOD_map_t light_fov;

    TCOD_color_t flash_color;
    float flash_fade_coef;

    bool controllable;
};

struct actor *actor_new(
    const char *name,
    enum race race,
    enum class class,
    enum faction faction,
    uint8_t level,
    uint8_t floor,
    uint8_t x,
    uint8_t y,
    bool torch);
void actor_delete(struct actor *actor);

int actor_calc_max_health(const struct actor *actor);
int actor_calc_max_mana(const struct actor *actor);
int actor_calc_armor_class(const struct actor *actor);
int actor_calc_attack_bonus(const struct actor *actor);
int actor_calc_threat_range(const struct actor *actor);
int actor_calc_critical_multiplier(const struct actor *actor);
const char *actor_calc_damage(const struct actor *actor);
int actor_calc_damage_bonus(const struct actor *actor);
int actor_calc_experience_to_level(const struct actor *actor);

void actor_calc_light(struct actor *actor);
void actor_calc_fade(struct actor *actor, float delta_time);

void actor_calc_fov(struct actor *actor);

void actor_give_experience(struct actor *actor, int experience);
void actor_level_up(struct actor *actor);

bool actor_ai(struct actor *actor);
bool actor_path_towards(
    struct actor *actor,
    int target_x, int target_y);
bool actor_move_towards(
    struct actor *actor,
    int target_x, int target_y);
bool actor_move(
    struct actor *actor,
    int x, int y);
bool actor_swap(struct actor *actor, struct actor *other);
bool actor_open_door(
    struct actor *actor,
    int x, int y);
bool actor_close_door(
    struct actor *actor,
    int x, int y);
bool actor_descend(
    struct actor *actor,
    bool is_leader,
    void ***iterator);
bool actor_ascend(
    struct actor *actor,
    bool is_leader,
    void ***iterator);
bool actor_open_chest(
    struct actor *actor,
    int x, int y);
bool actor_pray(
    struct actor *actor,
    int x, int y);
bool actor_drink(
    struct actor *actor,
    int x, int y);
bool actor_sit(
    struct actor *actor,
    int x, int y);
bool actor_grab(
    struct actor *actor,
    int x, int y);
bool actor_drop(struct actor *actor, struct item *item);
bool actor_equip(struct actor *actor, struct item *item);
bool actor_unequip(struct actor *actor, enum equip_slot equip_slot);
bool actor_quaff(struct actor *actor, struct item *item);
bool actor_read(
    struct actor *actor,
    struct item *item,
    int x, int y);
bool actor_bash(struct actor *actor, struct object *object);
bool actor_shoot(
    struct actor *actor,
    int x, int y);
bool actor_attack(struct actor *actor, struct actor *other, struct item *ammunition);
bool actor_cast_spell(
    struct actor *actor,
    enum spell_type spell_type,
    int x, int y,
    bool from_memory);

void actor_restore_health(struct actor *actor, int health);
void actor_restore_mana(struct actor *actor, int mana);
bool actor_take_damage(struct actor *actor, struct actor *attacker, int damage);
void actor_die(struct actor *actor, struct actor *killer);

#endif
