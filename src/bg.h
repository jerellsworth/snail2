#ifndef BG_H
#define BG_H

#include "bh.h"

#define TILE_FLOOR_NORMAL 0
#define TILE_FLOOR_HOT 2
#define TILE_FLOOR_COLD 4
#define TILE_ROCK_1 6
#define TILE_ROCK_2 8
#define TILE_STARS 14
#define TILE_LADY 16
#define TILE_BAT 17
#define TLS_FG_COLS 18

#define COLLISION_NONE 0
#define COLLISION_WALL 1
#define COLLISION_HOT 2
#define COLLISION_COLD 3

typedef enum {
    BG_BEHAVIOR_NONE,
    BG_BEHAVIOR_STATIC
} BG_Behavior;

struct BG_s {
    u8 (*collision)[MAP_TILES_H][MAP_TILES_W];
    u8 (*collision_bak)[MAP_TILES_H][MAP_TILES_W];
    Map *map;

    u16 map_a_ind;
    Map *map_a;

    fixx x;
    fixy y;
    u16 frames;
    u16 tile_ind;

    bool dbg;

    u16 bg_tile_ind;
    u16 fg_tile_ind;

    u8 degradation;
    BG_Behavior behavior;
};

BG *BG_init(
    const MapDefinition* mapDef,
    const TileSet *tileset,
    u8 (*collision)[MAP_TILES_H][MAP_TILES_W],
    const u16* pal);
void BG_change_map(
    BG *bg,
    const MapDefinition* mapDef,
    const TileSet *tileset,
    BG_Behavior behavior
    );
void BG_scroll_to(BG *bg, fixx x, fixy y);
void BG_scroll_to_diff(BG *bg, fix16 dx, fix16 dy);
void BG_update(BG *bg);
void BG_reset_fx(BG *bg);
void BG_del(BG *bg);
bool BG_collide(BG *bg, fixx x, fixy y, fix16 dx, fix16 dy, bool ignore_walls);
bool BG_in_range(BG *bg, Phy *p);
fixx BG_x(BG *bg);
fixy BG_y(BG *bg);
void BG_pause_and_scroll_to(Enc *enc, BG *bg, fixy tgt_y);
void BG_roll(BG *bg, Enc *e, u8 hot_spots, u8 cold_spots, u8 rocks);
void BG_flood(BG *bg, s16 c, s16 r, u16 tile, u8 collision_type, u16 flood_fill_strength);
u8 BG_floor_type(BG *bg, fixx x, fixy y);

#endif
