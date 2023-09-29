#include "monster_pack.h"

const struct monster_pack_data monster_pack_database[] = {
    [MONSTER_PACK_BATS] = {
        .min_floor = 0,
        .max_floor = 4,

        .monsters = {
            [MONSTER_BAT] = {
                .min_count = 1,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_BUGBEARS] = {
        .min_floor = 2,
        .max_floor = 6,

        .monsters = {
            [MONSTER_BUGBEAR] = {
                .min_count = 1,
                .max_count = 3,
            },
            [MONSTER_GOBLIN] = {
                .min_count = 0,
                .max_count = 3,
            },
        },
    },
    [MONSTER_PACK_DIRE_RAT] = {
        .min_floor = 1,
        .max_floor = 6,

        .monsters = {
            [MONSTER_DIRE_RAT] = {
                .min_count = 1,
                .max_count = 1,
            },
            [MONSTER_RAT] = {
                .min_count = 0,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_HYENAS] = {
        .min_floor = 2,
        .max_floor = 7,

        .monsters = {
            [MONSTER_HYENA] = {
                .min_count = 1,
                .max_count = 3,
            },
        },
    },
    [MONSTER_PACK_GNOLLS] = {
        .min_floor = 3,
        .max_floor = 8,

        .monsters = {
            [MONSTER_GNOLL] = {
                .min_count = 1,
                .max_count = 3,
            },
            [MONSTER_HYENA] = {
                .min_count = 1,
                .max_count = 3,
            },
        },
    },
    [MONSTER_PACK_GOBLINS] = {
        .min_floor = 0,
        .max_floor = 6,

        .monsters = {
            [MONSTER_GOBLIN] = {
                .min_count = 1,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_KOBOLDS] = {
        .min_floor = 0,
        .max_floor = 6,

        .monsters = {
            [MONSTER_KOBOLD] = {
                .min_count = 1,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_RATS] = {
        .min_floor = 0,
        .max_floor = 4,

        .monsters = {
            [MONSTER_RAT] = {
                .min_count = 1,
                .max_count = 5,
            },
        },
    },
    [MONSTER_PACK_RED_DRAGON] = {
        .min_floor = 7,
        .max_floor = 100,

        .monsters = {
            [MONSTER_RED_DRAGON_WYRMLING] = {
                .min_count = 1,
                .max_count = 1,
            },
            [MONSTER_RED_DRAGON_ADULT] = {
                .min_count = 0,
                .max_count = 1,
            },
        },
    },
    [MONSTER_PACK_TROLLS] = {
        .min_floor = 5,
        .max_floor = 10,

        .monsters = {
            [MONSTER_TROLL] = {
                .min_count = 1,
                .max_count = 4,
            },
        },
    },
    [MONSTER_PACK_SKELETONS] = {
        .min_floor = 0,
        .max_floor = 6,

        .monsters = {
            [MONSTER_SKELETON_WARRIOR] = {
                .min_count = 1,
                .max_count = 5,
            },
        },
    },
};
