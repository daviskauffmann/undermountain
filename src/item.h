#ifndef ITEM_H
#define ITEM_H

#include <libtcod/libtcod.h>

struct item_common
{
    char __placeholder;
};

enum item_type
{
    ITEM_BOW,
    ITEM_POTION,
    ITEM_SCROLL,
    ITEM_SHIELD,
    ITEM_SPEAR,
    ITEM_SWORD,

    NUM_ITEM_TYPES
};

struct item_info
{
    const char *name;
    unsigned char glyph;
    TCOD_color_t color;
};

struct item
{
    struct game *game;
    enum item_type type;
    int level;
    int x;
    int y;
};

struct item *item_create(enum item_type type, struct game *game, int level, int x, int y);
void item_destroy(struct item *item);

#endif
