#ifndef ASSEMBLAGE_H
#define ASSEMBLAGE_H

typedef struct map_s map_t;
typedef struct entity_s entity_t;
typedef struct game_s game_t;

entity_t *create_player(map_t *map, int x, int y);
entity_t *create_pet(map_t *map, int x, int y);
entity_t *create_skeleton(map_t *map, int x, int y);
entity_t *create_skeleton_captain(map_t *map, int x, int y);
entity_t *create_zombie(map_t *map, int x, int y);
entity_t *create_jackal(map_t *map, int x, int y);
entity_t *create_adventurer(map_t *map, int x, int y);
entity_t *create_longsword(map_t *map, int x, int y);
entity_t *create_brazier(map_t *map, int x, int y);

#endif
