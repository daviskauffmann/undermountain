#ifndef GAME_ITEM_H
#define GAME_ITEM_H

#include <libtcod.h>

enum race_size;

struct item_common
{
    char __placeholder;
};

enum base_item
{
    BASE_ITEM_SWORD,
    BASE_ITEM_BOW,
    BASE_ITEM_POTION,

    NUM_BASE_ITEMS
};

enum equip_slot
{
    EQUIP_SLOT_NONE,
    EQUIP_SLOT_MAIN_HAND,
    EQUIP_SLOT_OFF_HAND,

    NUM_EQUIP_SLOTS
};

struct equip_slot_data
{
    char *name;
    char *label; // TODO: not the biggest fan of this solution
};

struct base_item_data
{
    unsigned char glyph;
    TCOD_color_t color;
    enum equip_slot equip_slot;
    bool two_handed;
    bool ranged;
};

enum item_type
{
    ITEM_TYPE_LONGSWORD,
    ITEM_TYPE_LONGBOW,
    ITEM_TYPE_HEALING_POTION,

    NUM_ITEM_TYPES
};

struct item_data
{
    enum base_item base_item;
    const char *name;
    const char *description;
    int min_damage;
    int max_damage;
};

struct item
{
    enum item_type type;
    int floor;
    int x;
    int y;
};

struct item *item_new(enum item_type type, int floor, int x, int y);
void item_delete(struct item *item);

#endif
