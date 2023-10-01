#include "monster_pack.h"

const struct monster_pack_data monster_pack_database[] = {
    [MONSTER_PACK_TYPE_BATS] = {
        .min_floor = 0,
        .max_floor = 4,

        .monsters = {
            [MONSTER_TYPE_BAT] = {
                .min_count = 1,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_TYPE_BUGBEARS] = {
        .min_floor = 4,
        .max_floor = 8,

        .monsters = {
            [MONSTER_TYPE_BUGBEAR] = {
                .min_count = 1,
                .max_count = 3,
            },
            [MONSTER_TYPE_GOBLIN] = {
                .min_count = 0,
                .max_count = 3,
            },
        },
    },
    [MONSTER_PACK_TYPE_DIRE_RAT] = {
        .min_floor = 2,
        .max_floor = 6,

        .monsters = {
            [MONSTER_TYPE_DIRE_RAT] = {
                .min_count = 1,
                .max_count = 1,
            },
            [MONSTER_TYPE_RAT] = {
                .min_count = 0,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_TYPE_FIRE_BEETLES] = {
        .min_floor = 0,
        .max_floor = 4,

        .monsters = {
            [MONSTER_TYPE_FIRE_BEETLE] = {
                .min_count = 2,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_TYPE_GIANT_ANTS] = {
        .min_floor = 2,
        .max_floor = 6,

        .monsters = {
            [MONSTER_TYPE_GIANT_ANT] = {
                .min_count = 2,
                .max_count = 6,
            },
        },
    },
    [MONSTER_PACK_TYPE_GNOLLS] = {
        .min_floor = 4,
        .max_floor = 8,

        .monsters = {
            [MONSTER_TYPE_GNOLL] = {
                .min_count = 1,
                .max_count = 2,
            },
            [MONSTER_TYPE_HYENA] = {
                .min_count = 0,
                .max_count = 2,
            },
        },
    },
    [MONSTER_PACK_TYPE_GOBLINS] = {
        .min_floor = 2,
        .max_floor = 6,

        .monsters = {
            [MONSTER_TYPE_GOBLIN] = {
                .min_count = 1,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_TYPE_HOBGOBLINS] = {
        .min_floor = 4,
        .max_floor = 8,

        .monsters = {
            [MONSTER_TYPE_HOBGOBLIN] = {
                .min_count = 1,
                .max_count = 3,
            },
            [MONSTER_TYPE_GOBLIN] = {
                .min_count = 0,
                .max_count = 3,
            },
        },
    },
    [MONSTER_PACK_TYPE_HYENAS] = {
        .min_floor = 2,
        .max_floor = 6,

        .monsters = {
            [MONSTER_TYPE_HYENA] = {
                .min_count = 1,
                .max_count = 3,
            },
        },
    },
    [MONSTER_PACK_TYPE_KOBOLDS] = {
        .min_floor = 2,
        .max_floor = 6,

        .monsters = {
            [MONSTER_TYPE_KOBOLD] = {
                .min_count = 1,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_TYPE_KRENSHARS] = {
        .min_floor = 2,
        .max_floor = 6,

        .monsters = {
            [MONSTER_TYPE_KRENSHAR] = {
                .min_count = 1,
                .max_count = 2,
            },
        },
    },
    [MONSTER_PACK_TYPE_OGRES] = {
        .min_floor = 6,
        .max_floor = 10,

        .monsters = {
            [MONSTER_TYPE_OGRE] = {
                .min_count = 1,
                .max_count = 4,
            },
        },
    },
    [MONSTER_PACK_TYPE_ORCS] = {
        .min_floor = 2,
        .max_floor = 6,

        .monsters = {
            [MONSTER_TYPE_OGRE] = {
                .min_count = 2,
                .max_count = 4,
            },
        },
    },
    [MONSTER_PACK_TYPE_RAKSHASAS] = {
        .min_floor = 8,
        .max_floor = 12,

        .monsters = {
            [MONSTER_TYPE_RAKSHASA] = {
                .min_count = 1,
                .max_count = 1,
            },
        },
    },
    [MONSTER_PACK_TYPE_RATS] = {
        .min_floor = 0,
        .max_floor = 4,

        .monsters = {
            [MONSTER_TYPE_RAT] = {
                .min_count = 1,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_TYPE_RED_DRAGON] = {
        .min_floor = 12,
        .max_floor = 40,

        .monsters = {
            [MONSTER_TYPE_RED_DRAGON_WYRMLING] = {
                .min_count = 1,
                .max_count = 1,
            },
            [MONSTER_TYPE_RED_DRAGON_ADULT] = {
                .min_count = 0,
                .max_count = 1,
            },
        },
    },
    [MONSTER_PACK_TYPE_SKELETONS] = {
        .min_floor = 2,
        .max_floor = 6,

        .monsters = {
            [MONSTER_TYPE_SKELETON_WARRIOR] = {
                .min_count = 1,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_TYPE_SNAKE] = {
        .min_floor = 0,
        .max_floor = 4,

        .monsters = {
            [MONSTER_TYPE_SNAKE] = {
                .min_count = 1,
                .max_count = 1,
            },
        },
    },
    [MONSTER_PACK_TYPE_SPIDERS] = {
        .min_floor = 0,
        .max_floor = 4,

        .monsters = {
            [MONSTER_TYPE_SPIDER] = {
                .min_count = 2,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_TYPE_TROLLS] = {
        .min_floor = 8,
        .max_floor = 12,

        .monsters = {
            [MONSTER_TYPE_TROLL] = {
                .min_count = 1,
                .max_count = 4,
            },
        },
    },
    [MONSTER_PACK_TYPE_WOLVES] = {
        .min_floor = 2,
        .max_floor = 6,

        .monsters = {
            [MONSTER_TYPE_WOLF] = {
                .min_count = 1,
                .max_count = 5,
            },
        },
    },
};
