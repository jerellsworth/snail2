#include "bh.h"

BG *BG_init(
    const MapDefinition *mapDef_bg,
    const TileSet *tileset_bg,
    const MapDefinition *mapDef_fg,
    const TileSet *tileset_fg,
    u8 (*collision)[MAP_TILES_H][MAP_TILES_W],
    const u16* pal) {
    BG *bg = (BG *)ct_calloc(sizeof(BG), 1);

    BG_change_map(bg, mapDef_bg, tileset_bg, mapDef_fg, tileset_fg, BG_BEHAVIOR_NONE);

    bg->collision_bak = collision;
    bg->collision = ct_calloc(MAP_TILES_H * MAP_TILES_W, sizeof(u8));
    memcpy(bg->collision, bg->collision_bak, MAP_TILES_H * MAP_TILES_W * sizeof(u8));

    PAL_setPalette(PAL0, pal, DMA);
    VDP_setBackgroundColor(1);
    SYS_doVBlankProcess();

    return bg;
};

void BG_change_map(
    BG *bg,
    const MapDefinition* mapDef_bg,
    const TileSet *tileset_bg,
    const MapDefinition* mapDef_fg,
    const TileSet *tileset_fg,
    BG_Behavior behavior
    ) {

    if (bg->map) {
        MAP_release(bg->map);
        bg->map = NULL;
    }
    bg->behavior = behavior;
    bg->tile_ind = TILE_USER_INDEX;
    bg->bg_tile_ind = bg->tile_ind;
    VDP_loadTileSet(tileset_bg, bg->tile_ind, DMA);
    bg->map = MAP_create(mapDef_bg, BG_B, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, bg->tile_ind));
    bg->tile_ind += tileset_bg->numTile;
    BG_scroll_to(bg, 80, 0);

    bg->fg_tile_ind = bg->tile_ind;
    VDP_loadTileSet(tileset_fg, bg->fg_tile_ind, DMA);
    bg->tile_ind += tileset_bg->numTile;
    bg->map_fg = MAP_create(mapDef_fg, BG_A, TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, bg->fg_tile_ind));
    MAP_scrollTo(bg->map_fg, 0, 0);
    VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_COLUMN);

    SYS_doVBlankProcess();
}

void BG_scroll_to(BG *bg, fixx x, fixy y) {
    MAP_scrollTo(bg->map, fixxToRoundedInt(x), fixyToRoundedInt(y));
}

void BG_scroll_to_diff(BG *bg, fix16 dx, fix16 dy) {
    u32 x = bg->map->posX + fix16ToRoundedInt(dx);
    u32 y = bg->map->posY + fix16ToRoundedInt(dy);
    MAP_scrollTo(bg->map, x, y);
}

fixx BG_x(BG *bg) {
    return FIXX(bg->map->posX);
}

fixy BG_y(BG *bg) {
    return FIXY(bg->map->posY);
}

void BG_update(BG *bg) {
    
    ++bg->frames;

}

void BG_reset_fx(BG *bg) {
}

void BG_del(BG *bg) {
    free(bg->collision);
    MAP_release(bg->map);
    free(bg);
    VDP_init();
    SYS_doVBlankProcess();
    VDP_waitVSync();
    VDP_init(); // I don't know why you need to do this, but you do
}

bool BG_in_range(BG *bg, Phy *p) {
    u16 int_y = fixyToInt(p->y);
    u16 int_x = fixxToInt(p->x);
    return (
        int_y + p->h >= bg->map->posY &&
        int_y <= bg->map->posY + 224 &&
        int_x + p->w >= bg->map->posX &&
        int_x <= bg->map->posX + 320);
}

bool BG_collide(BG *bg, Phy *p) {
    fixx px = p->x + p->hbox_offset_x;
    fixy py = p->y + p->hbox_offset_y;
    bool ret = FALSE;
    if (px < FIXX(8)) {
        p->x = FIXX(8) - p->hbox_offset_x;
        if (p->elastic) p->dx = -(p->dx >> 1);
        ret = TRUE;
    } else if (px + FIXX(p->w) > FIXX(312)) {
        p->x = FIXX(312) - FIXX(p->w) - p->hbox_offset_x;
        if (p->elastic) p->dx = -(p->dx >> 1);
        ret = TRUE;
    }
    if (py < FIXY(16)) {
        p->y = FIXY(16) - p->hbox_offset_y;
        if (p->elastic) p->dy = -(p->dy >> 1);
        ret = TRUE;
    } else if (py + FIXY(p->h) > FIXY(216)) {
        p->y = FIXY(216) - FIXY(p->h) - p->hbox_offset_y;
        if (p->elastic) p->dy = -(p->dy >> 1);
        ret = TRUE;
    }
    return ret;
}
