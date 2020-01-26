#ifndef GAME_ITEM_H
#define GAME_ITEM_H

#include <libtcod.h>

struct item_common
{
    char __placeholder;
};

enum equip_slot
{
    EQUIP_SLOT_NONE,
    EQUIP_SLOT_AMMUNITION,
    EQUIP_SLOT_ARMOR,
    EQUIP_SLOT_MAIN_HAND,
    EQUIP_SLOT_OFF_HAND,

    NUM_EQUIP_SLOTS
};

struct equip_slot_datum
{
    char *name;
};

enum ammunition_type
{
    AMMUNITION_TYPE_NONE,
    AMMUNITION_TYPE_ARROW,
    AMMUNITION_TYPE_BOLT
};

enum item_type
{
    ITEM_TYPE_BODKIN_ARROW,
    ITEM_TYPE_BOLT,
    ITEM_TYPE_CROSSBOW,
    ITEM_TYPE_IRON_ARMOR,
    ITEM_TYPE_GREATSWORD,
    ITEM_TYPE_LONGSWORD,
    ITEM_TYPE_LONGBOW,
    ITEM_TYPE_KITE_SHIELD,
    ITEM_TYPE_HEALING_POTION,

    NUM_ITEM_TYPES
};

struct item_datum
{
    const char *name;
    const char *description;
    unsigned char glyph;
    TCOD_color_t color;
    enum equip_slot equip_slot;
    bool two_handed;
    int armor;
    float block_chance;
    int min_damage;
    int max_damage;
    bool ranged;
    int max_durability;
    bool quaffable;
    int max_stack;
    enum ammunition_type ammunition_type;
};

struct item
{
    enum item_type type;
    int floor;
    int x;
    int y;
    int current_durability;
    int current_stack;
};

struct item *item_new(enum item_type type, int floor, int x, int y, int stack);
void item_delete(struct item *item);

#endif
