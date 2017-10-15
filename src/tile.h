#ifndef TILE_H
#define TILE_H

typedef enum {
    TILE_EMPTY = 0,
    TILE_FLOOR,
    TILE_WALL
} TileType;

typedef struct
{
    TileType type;
} Tile;

void tile_init(Tile *tile, TileType type);
void tile_draw(Tile *tile, int x, int y);

#endif