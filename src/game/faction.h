#ifndef FACTION_H
#define FACTION_H

// TODO: more factions
enum faction
{
    FACTION_GOOD,
    FACTION_EVIL,

    NUM_FACTIONS
};

struct faction_datum
{
    const char *name;
    // TODO: faction relations
};

#endif
