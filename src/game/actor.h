#ifndef UM_GAME_ACTOR_H
#define UM_GAME_ACTOR_H

#include "ability.h"
#include "feat.h"
#include "item.h"
#include "saving_throw.h"
#include "special_ability.h"
#include "special_attack.h"
#include "spell.h"
#include <libtcod.h>

struct object;

struct actor_metadata
{
    int turns_to_chase;
};

struct actor
{
    char *name;

    enum race race;
    enum class class;
    enum size size;
    enum faction faction;

    int level;
    int experience;

    int ability_points;
    int ability_scores[NUM_ABILITIES];

    bool special_abilities[NUM_SPECIAL_ABILITIES];

    bool special_attacks[NUM_SPECIAL_ATTACKS];

    bool feats[NUM_FEATS];

    int base_hit_points;
    int hit_points;

    int mana;
    bool spells[NUM_SPELL_TYPES];
    enum spell_type readied_spell;

    int gold;
    struct item *equipment[NUM_EQUIP_SLOTS];
    struct list *items;

    int floor;
    int x;
    int y;

    TCOD_Map *fov;

    bool took_turn;
    float energy;

    struct actor *current_target;
    int last_seen_x;
    int last_seen_y;
    int turns_chased;

    struct actor *last_attacked_target;

    struct actor *leader;

    enum light_type light_type;
    TCOD_Map *light_fov;

    TCOD_ColorRGB flash_color;
    float flash_alpha;

    bool controllable;

    bool dead;
};

struct actor_special_abilities
{
    bool has[NUM_SPECIAL_ABILITIES];
};

struct actor_special_attacks
{
    bool has[NUM_SPECIAL_ATTACKS];
};

struct actor_feats
{
    bool has[NUM_FEATS];
};

struct actor_spells
{
    bool has[NUM_SPELL_TYPES];
};

extern const struct actor_metadata actor_metadata;

struct actor *actor_new(
    const char *name,
    enum race race,
    enum class class,
    enum size size,
    enum faction faction,
    int level,
    const int ability_scores[NUM_ABILITIES],
    const bool special_abilities[NUM_SPECIAL_ABILITIES],
    const bool special_attacks[NUM_SPECIAL_ATTACKS],
    const bool feats[NUM_FEATS],
    int floor,
    int x,
    int y);
void actor_delete(struct actor *actor);

int actor_get_experience_for_level(int level);
void actor_give_experience(struct actor *actor, int experience);
void actor_level_up(struct actor *actor);

int actor_get_ability_score(const struct actor *actor, enum ability ability);
int actor_get_ability_modifer(const struct actor *actor, enum ability ability);
void actor_spend_ability_point(struct actor *actor, enum ability ability);

struct actor_special_abilities actor_get_special_abilities(const struct actor *actor);

struct actor_special_attacks actor_get_special_attacks(const struct actor *actor);

struct actor_feats actor_get_feats(const struct actor *actor);
bool actor_has_prerequisites_for_feat(const struct actor *actor, enum feat feat);

struct actor_spells actor_get_spells(const struct actor *actor);

int actor_get_max_hit_points(const struct actor *actor);
void actor_restore_hit_points(struct actor *actor, int health);
bool actor_damage_hit_points(struct actor *actor, struct actor *attacker, int damage);

int actor_get_max_mana(const struct actor *actor);
void actor_restore_mana(struct actor *actor, int mana);
float actor_get_arcane_spell_failure(const struct actor *actor);
int actor_get_spell_mana_cost(const struct actor *actor, enum spell_type spell_type);

int actor_get_saving_throw(const struct actor *actor, enum saving_throw saving_throw);

int actor_get_armor_class(const struct actor *actor);

int actor_get_attacks_per_round(const struct actor *actor);
int actor_get_successive_attack_penalty(const struct actor *actor);
int actor_get_base_attack_bonus(const struct actor *actor);
int actor_get_attack_bonus(const struct actor *actor);
int actor_get_ranged_attack_penalty(const struct actor *actor, const struct actor *other);
int actor_get_threat_range(const struct actor *actor);
int actor_get_critical_multiplier(const struct actor *actor);
int actor_get_damage_bonus(const struct actor *actor);
const char *actor_get_damage(const struct actor *actor);

bool actor_melee_touch_attack(struct actor *actor, struct actor *other);
bool actor_ranged_touch_attack(struct actor *actor, struct actor *other);

enum equippability actor_get_item_equippability(const struct actor *actor, const struct item *item);

float actor_get_max_carry_weight(const struct actor *actor);
float actor_get_carry_weight(const struct actor *actor);
float actor_get_speed(const struct actor *actor);

void actor_update_light(struct actor *actor);
void actor_update_fade(struct actor *actor, float delta_time);

int actor_get_sight_radius(const struct actor *actor);
void actor_update_fov(struct actor *actor);

bool actor_can_take_turn(const struct actor *actor);

bool actor_is_enemy(const struct actor *actor, const struct actor *other);
bool actor_is_enemy_nearby(const struct actor *actor);
struct actor *actor_find_nearest_enemy(const struct actor *actor);

bool actor_is_proficient(const struct actor *actor, const struct item *item);
struct item *actor_find_melee_weapon(const struct actor *actor);
bool actor_has_ranged_weapon(const struct actor *actor);
struct item *actor_find_ranged_weapon(const struct actor *actor);
bool actor_has_ammunition(const struct actor *actor, const struct item *weapon);
struct item *actor_find_ammunition(const struct actor *actor, const struct item *weapon);

bool actor_can_make_attack_of_opportunity(const struct actor *actor, const struct actor *other);
void actor_provoke_attack_of_opportunity(struct actor *actor, struct list **damages);

bool actor_ai(struct actor *actor);
bool actor_rest(struct actor *actor);
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
bool actor_interact(struct actor *actor, struct object *object);
bool actor_open_door(
    struct actor *actor,
    int x, int y);
bool actor_close_door(
    struct actor *actor,
    int x, int y);
bool actor_descend(struct actor *actor);
bool actor_ascend(struct actor *actor);
bool actor_open_chest(
    struct actor *actor,
    int x, int y);
bool actor_pray(
    struct actor *actor,
    int x, int y);
bool actor_drink(
    struct actor *actor,
    int x, int y);
bool actor_dip(
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
bool actor_learn(
    struct actor *actor,
    struct item *item);
bool actor_bash(struct actor *actor, struct object *object);
bool actor_shoot(
    struct actor *actor,
    int x, int y);
bool actor_attack(struct actor *actor, struct actor *other, bool opportunity, int *damage_dealt);
bool actor_cast(
    struct actor *actor,
    enum spell_type spell_type,
    int x, int y,
    bool from_memory);

void actor_die(struct actor *actor, struct actor *killer);

#endif
