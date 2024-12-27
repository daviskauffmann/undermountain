#include "item.h"

#include "base_item.h"
#include "color.h"
#include "list.h"
#include "spell.h"
#include "world.h"
#include <malloc.h>

const struct item_data item_database[] = {
    [ITEM_TYPE_ADAMANTINE_BREASTPLATE] = {
        .type = BASE_ITEM_TYPE_BREASTPLATE,

        .name = "Adamantine Breastplate",
        .description = "",
        .color = {COLOR_DARK_AZURE},

        .level = 1,

        .unique = true,
    },
    [ITEM_TYPE_ARROW] = {
        .type = BASE_ITEM_TYPE_ARROW,

        .name = "Arrow",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_BATTLEAXE] = {
        .type = BASE_ITEM_TYPE_BATTLEAXE,

        .name = "Battleaxe",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_BREASTPLATE] = {
        .type = BASE_ITEM_TYPE_BREASTPLATE,

        .name = "Breastplate",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_BOLT] = {
        .type = BASE_ITEM_TYPE_BOLT,

        .name = "Bolt",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_BULLET] = {
        .type = BASE_ITEM_TYPE_BULLET,

        .name = "Bullet",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_CLUB] = {
        .type = BASE_ITEM_TYPE_CLUB,

        .name = "Club",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_DAGGER] = {
        .type = BASE_ITEM_TYPE_DAGGER,

        .name = "Dagger",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_FALCHION] = {
        .type = BASE_ITEM_TYPE_FALCHION,

        .name = "Falchion",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_FOOD] = {
        .type = BASE_ITEM_TYPE_FOOD,

        .name = "Food",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_FULL_PLATE] = {
        .type = BASE_ITEM_TYPE_FULL_PLATE,

        .name = "Full Plate",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_GOLD] = {
        .type = BASE_ITEM_TYPE_GOLD,

        .name = "Gold",
        .description = "",
        .color = {COLOR_GOLD},

        .level = 1,
    },
    [ITEM_TYPE_GREATCLUB] = {
        .type = BASE_ITEM_TYPE_GREATCLUB,

        .name = "Greatclub",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_GREATSWORD] = {
        .type = BASE_ITEM_TYPE_GREATSWORD,

        .name = "Greatsword",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_HALF_PLATE] = {
        .type = BASE_ITEM_TYPE_HALF_PLATE,

        .name = "Half Plate",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_HEAVY_CROSSBOW] = {
        .type = BASE_ITEM_TYPE_HEAVY_CROSSBOW,

        .name = "Heavy Crossbow",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_HEAVY_MACE] = {
        .type = BASE_ITEM_TYPE_HEAVY_MACE,

        .name = "Heavy Mace",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_HEAVY_SHIELD] = {
        .type = BASE_ITEM_TYPE_HEAVY_SHIELD,

        .name = "Heavy Shield",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_HIDE_ARMOR] = {
        .type = BASE_ITEM_TYPE_HIDE_ARMOR,

        .name = "Hide Armor",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_JAVELIN] = {
        .type = BASE_ITEM_TYPE_JAVELIN,

        .name = "Javelin",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_LEATHER_ARMOR] = {
        .type = BASE_ITEM_TYPE_LEATHER_ARMOR,

        .name = "Leather Armor",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_LIGHT_CROSSBOW] = {
        .type = BASE_ITEM_TYPE_LIGHT_CROSSBOW,

        .name = "Light Crossbow",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_LIGHT_MACE] = {
        .type = BASE_ITEM_TYPE_LIGHT_MACE,

        .name = "Light Mace",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_LIGHT_SHIELD] = {
        .type = BASE_ITEM_TYPE_LIGHT_SHIELD,

        .name = "Light Shield",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_LONGBOW] = {
        .type = BASE_ITEM_TYPE_LONGBOW,

        .name = "Longbow",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_LONGSWORD] = {
        .type = BASE_ITEM_TYPE_LONGSWORD,

        .name = "Longsword",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_MORNINGSTAR] = {
        .type = BASE_ITEM_TYPE_MORNINGSTAR,

        .name = "Morningstar",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_POTION_CURE_LIGHT_WOUNDS] = {
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Cure Light Wounds",
        .description = "",
        .color = {COLOR_LIGHT_RED},

        .spell_type = SPELL_TYPE_CURE_LIGHT_WOUNDS,

        .level = 1,
    },
    [ITEM_TYPE_POTION_CURE_MODERATE_WOUNDS] = {
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Cure Moderate Wounds",
        .description = "",
        .color = {COLOR_LIGHT_RED},

        .spell_type = SPELL_TYPE_CURE_MODERATE_WOUNDS,

        .level = 3,
    },
    [ITEM_TYPE_POTION_CURE_SERIOUS_WOUNDS] = {
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Cure Serious Wounds",
        .description = "",
        .color = {COLOR_LIGHT_RED},

        .spell_type = SPELL_TYPE_CURE_SERIOUS_WOUNDS,

        .level = 5,
    },
    [ITEM_TYPE_POTION_CURE_CRITICAL_WOUNDS] = {
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Cure Critical Wounds",
        .description = "",
        .color = {COLOR_LIGHT_RED},

        .spell_type = SPELL_TYPE_CURE_CRITICAL_WOUNDS,

        .level = 7,
    },
    [ITEM_TYPE_POTION_HEAL] = {
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Heal",
        .description = "",
        .color = {COLOR_LIGHT_RED},

        .spell_type = SPELL_TYPE_HEAL,

        .level = 9,
    },
    [ITEM_TYPE_POTION_RESTORE_MANA] = {
        .type = BASE_ITEM_TYPE_POTION,

        .name = "Potion of Restore Mana",
        .description = "",
        .color = {COLOR_LIGHT_AZURE},

        .spell_type = SPELL_TYPE_RESTORE_MANA,

        .level = 1,
    },
    [ITEM_TYPE_SCIMITAR] = {
        .type = BASE_ITEM_TYPE_SCIMITAR,

        .name = "Scimitar",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_SCROLL_CHAIN_LIGHTNING] = {
        .type = BASE_ITEM_TYPE_SCROLL,

        .name = "Scroll of Chain Lightning",
        .description = "",
        .color = {COLOR_WHITE},

        .spell_type = SPELL_TYPE_CHAIN_LIGHTNING,

        .level = 1,
    },
    [ITEM_TYPE_SCROLL_FIREBALL] = {
        .type = BASE_ITEM_TYPE_SCROLL,

        .name = "Scroll of Fireball",
        .description = "",
        .color = {COLOR_WHITE},

        .spell_type = SPELL_TYPE_FIREBALL,

        .level = 1,
    },
    [ITEM_TYPE_SHORTBOW] = {
        .type = BASE_ITEM_TYPE_SHORTBOW,

        .name = "Shortbow",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_SHORTSPEAR] = {
        .type = BASE_ITEM_TYPE_SHORTSPEAR,

        .name = "Shortspear",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_SLING] = {
        .type = BASE_ITEM_TYPE_SLING,

        .name = "Sling",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_SPEAR] = {
        .type = BASE_ITEM_TYPE_SPEAR,

        .name = "Spear",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,

        .breakable = true,
    },
    [ITEM_TYPE_STUDDED_LEATHER_ARMOR] = {
        .type = BASE_ITEM_TYPE_STUDDED_LEATHER_ARMOR,

        .name = "Studded Leather Armor",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_TOWER_SHIELD] = {
        .type = BASE_ITEM_TYPE_TOWER_SHIELD,

        .name = "Tower Shield",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
    [ITEM_TYPE_WIZARDS_ROBE] = {
        .type = BASE_ITEM_TYPE_CLOTHING,

        .name = "Wizard's Robe",
        .description = "",
        .color = {COLOR_WHITE},

        .level = 1,
    },
};

struct item *item_new(
    const enum item_type type,
    const int floor,
    const int x,
    const int y,
    const int stack)
{
    struct item *const item = malloc(sizeof(*item));

    item->type = type;

    item->floor = floor;
    item->x = x;
    item->y = y;

    item->stack = CLAMP(0, base_item_database[item_database[type].type].max_stack, stack);

    if (item_database[type].unique)
    {
        if (list_contains(world->spawned_unique_item_types, (void *)(size_t)type))
        {
            item_delete(item);

            return NULL;
        }

        list_add(world->spawned_unique_item_types, (void *)(size_t)type);
    }

    return item;
}

void item_delete(struct item *const item)
{
    free(item);
}
