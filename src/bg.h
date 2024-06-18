#ifndef BG_H
#define BG_H

#include "bh.h"

#define COLLISION_NONE 0
#define COLLISION_WALL 1


typedef enum {
    BG_BEHAVIOR_NONE
} BG_Behavior;

struct BG_s {
    u8 (*collision)[MAP_TILES_H][MAP_TILES_W];
    u8 (*collision_bak)[MAP_TILES_H][MAP_TILES_W];
    Map *map;
    Map *map_fg;

    fixx x;
    fixy y;
    u16 frames;
    u16 tile_ind;

    bool dbg;

    u16 bg_tile_ind;
    u16 fg_tile_ind;

    BG_Behavior behavior;

    u16 theta;

    s16 v_offsets[20];

};

BG *BG_init(
    const MapDefinition *mapDef_bg,
    const TileSet *tileset_bg,
    const MapDefinition *mapDef_fg,
    const TileSet *tileset_fg,
    u8 (*collision)[MAP_TILES_H][MAP_TILES_W],
    const u16* pal);
void BG_change_map(
    BG *bg,
    const MapDefinition *mapDef_bg,
    const TileSet *tileset_bg,
    const MapDefinition *mapDef_fg,
    const TileSet *tileset_fg,
    BG_Behavior behavior
    );
void BG_scroll_to(BG *bg, fixx x, fixy y);
void BG_scroll_to_diff(BG *bg, fix16 dx, fix16 dy);
void BG_update(BG *bg);
void BG_reset_fx(BG *bg);
void BG_del(BG *bg);
bool BG_collide(BG *bg, Phy *p);
bool BG_in_range(BG *bg, Phy *p);
fixx BG_x(BG *bg);
fixy BG_y(BG *bg);

#endif
