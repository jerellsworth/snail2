#include "bh.h"

BG *BG_init(
    const MapDefinition* mapDef,
    const TileSet *tileset,
    u8 (*collision)[MAP_TILES_H][MAP_TILES_W],
    const u16* pal) {
    BG *bg = (BG *)ct_calloc(sizeof(BG), 1);

    BG_change_map(bg, mapDef, tileset, BG_BEHAVIOR_NONE);

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
    const MapDefinition* mapDef,
    const TileSet *tileset,
    BG_Behavior behavior
    ) {
    if (bg->map) {
        MAP_release(bg->map);
        bg->map = NULL;
    }
    bg->behavior = behavior;
    bg->tile_ind = TILE_USER_INDEX;
    bg->bg_tile_ind = bg->tile_ind;
    VDP_loadTileSet(tileset, bg->tile_ind, DMA);
    bg->map = MAP_create(mapDef, BG_B, TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, bg->tile_ind));
    bg->tile_ind += tileset->numTile;

    bg->fg_tile_ind = bg->tile_ind;
    VDP_loadTileSet(&TLS_FG, bg->fg_tile_ind, DMA);
    bg->tile_ind += TLS_FG.numTile;

    BG_scroll_to(bg, 0, 0);
    switch (behavior) {
        case BG_BEHAVIOR_STATIC:
            VDP_setScrollingMode(HSCROLL_LINE, VSCROLL_PLANE);
            break;
        default:
            VDP_setScrollingMode(HSCROLL_PLANE, VSCROLL_PLANE);
            break;
    }
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
    if (bg->degradation > 0 && !(bg->frames & 63)) {
        for (u8 i = 0; i < bg->degradation; ++i) {
            VDP_loadTileData(TLS_FG.tiles + 8 * (TILE_STARS + random_with_max(1) + random_with_max(1) * TLS_FG_COLS), bg->bg_tile_ind + i, 1, DMA_QUEUE);
        }
    }
    if (bg->behavior == BG_BEHAVIOR_STATIC && !(bg->frames & 3)) {
        s16 scroll_offsets[256];
        for (u16 i = 0; i < 256; ++i) {
            scroll_offsets[i] = random_with_max(63) - 32;
        }
        VDP_setHorizontalScrollLine(BG_B, 0, scroll_offsets, 256, DMA_QUEUE);
    }
    ++bg->frames;
}

void BG_reset_fx(BG *bg) {
    if (bg->behavior == BG_BEHAVIOR_STATIC) {
        s16 scroll_offsets[256];
        for (u16 i = 0; i < 256; ++i) {
            scroll_offsets[i] = 0;
        }
        VDP_setHorizontalScrollLine(BG_B, 0, scroll_offsets, 256, DMA_QUEUE);
    }
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

u8 BG_floor_type(BG *bg, fixx x, fixy y) {
    s16 row = ((fixyToRoundedInt(y)) >> 3);
    s16 col = ((fixxToRoundedInt(x)) >> 3);
    if (row < 0 || col < 0 || row >= MAP_TILES_H || col >= MAP_TILES_W ) return COLLISION_WALL;
    return (*(bg->collision))[row][col];
}

bool BG_collide(BG *bg, fixx x, fixy y, fix16 dx, fix16 dy, bool ignore_walls) {
    s16 row = ((fixyToRoundedInt(y)) >> 3);
    s16 col = ((fixxToRoundedInt(x)) >> 3);

    /*
    char buf[32];
    sprintf(buf, "%d,%d", row, col);
    VDP_drawText(buf, 0, 0);
    */

    if (row < 0 || col < 0 || row >= MAP_TILES_H || col >= MAP_TILES_W ) return TRUE;
    if (ignore_walls) return FALSE;

    u8 val = (*(bg->collision))[row][col];

    /*
    char buf2[32];
    sprintf(buf2, "%d", val);
    VDP_drawText(buf2, 20, 0);
    */

    switch (val) {
        case COLLISION_WALL:
            return TRUE;
        default:
            return FALSE;
    }
}

void BG_pause_and_scroll_to(Enc *enc, BG *bg, fixy tgt_y) {
    fixy current_y = FIXY(bg->map->posY);
    if (tgt_y >= current_y) {
        for (fixy y = current_y; y <= tgt_y; y += FIXY(8)) {
            BG_scroll_to(bg, 0, y);
            Physics_spr_set_position_all(enc);
            SPR_update();
            SYS_doVBlankProcess();
        }
    } else {
        for (fixy y = current_y; y >= tgt_y; y -= FIXY(8)) {
            BG_scroll_to(bg, 0, y);
            Physics_spr_set_position_all(enc);
            SPR_update();
            SYS_doVBlankProcess();
        }
    }
}

void BG_flood(BG *bg, s16 c, s16 r, u16 tile, u8 collision_type, u16 flood_fill_strength) {
    if (r <= 2 || c <= 1 || r >= MAP_TILES_H - 2 || c >= MAP_TILES_W - 1) return;
    u8 prev_col_type = (*(bg->collision))[r][c];
    if (prev_col_type != COLLISION_NONE) return;
    u16 row = random_with_max(1);
    u16 col = random_with_max(1);
    u16 ind = bg->fg_tile_ind + tile + row * TLS_FG_COLS + col;
    VDP_fillTileMapRectInc(
        BG_A,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind), 
        c,
        r,
        1,
        1
        );
    (*(bg->collision))[r][c] = collision_type;
    if (flood_fill_strength < random_with_max(127)) {
        BG_flood(bg, c, r - 1, tile, collision_type, flood_fill_strength);
    }
    if (flood_fill_strength < random_with_max(127)) {
        BG_flood(bg, c, r + 1, tile, collision_type, flood_fill_strength);
    }
    if (flood_fill_strength < random_with_max(127)) {
        BG_flood(bg, c - 1, r, tile, collision_type, flood_fill_strength);
    }
    if (flood_fill_strength < random_with_max(127)) {
        BG_flood(bg, c + 1, r, tile, collision_type, flood_fill_strength);
    }
}

void BG_add_obstacle(BG *bg, u16 tile, u8 collision_type, u16 flood_fill_strength) {
    u16 r = 2 + random_with_max(21);
    u16 c = 1 + random_with_max(35);
    if (flood_fill_strength > 0) {
        BG_flood(bg, c, r, tile, collision_type, flood_fill_strength);
    } else {
        u8 this_r = r;
        u8 this_c = c;
        VDP_fillTileMapRectInc(
            BG_A,
            TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, bg->fg_tile_ind + tile), 
            this_c,
            this_r,
            2,
            1
            );
        VDP_fillTileMapRectInc(
            BG_A,
            TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, bg->fg_tile_ind + tile + TLS_FG_COLS), 
            this_c,
            this_r + 1,
            2,
            1
            );
        (*(bg->collision))[this_r][this_c] = collision_type;
        (*(bg->collision))[this_r][this_c + 1] = collision_type;
        (*(bg->collision))[this_r + 1][this_c] = collision_type;
        (*(bg->collision))[this_r + 1][this_c + 1] = collision_type;
    }
}

void _BG_draw_wall(BG *bg, u8 left, u8 up, u8 right, u8 down) {
    u16 ind = bg->fg_tile_ind + TILE_ROCK_1;
    VDP_fillTileMapRect(
        BG_A,
        TILE_ATTR_FULL(PAL0, FALSE, FALSE, FALSE, ind),
        left,
        up,
        right - left + 1,
        down - up + 1
        );
    for (u8 r = up; r <= down; ++r) {
        for (u8 c = left; c <= right; ++c) {
            (*(bg->collision))[r][c] = COLLISION_WALL;
        }
    }
}

void BG_roll(BG *bg, Enc *e, u8 hot_spots, u8 cold_spots, u8 rocks) {
    // cells are 6 tiles tall
    // cells are 7,8,8,8,7 tiles wide
    memcpy(bg->collision, bg->collision_bak, MAP_TILES_H * MAP_TILES_W * sizeof(u8));
    VDP_clearPlane(BG_A, TRUE);

    Room *r = Room_new(0, 0, 12); // TODO different start depending on player position
    s16 left, up, right, down;
    left = 8;
    up = 2;
    right = 9;
    down = 8;
    for (u8 row = 0; row < ROOM_H; ++row) {
        for (u8 col = 0; col < ROOM_W; ++col) {
            Room_Cell *rc = r->cells[row][col];
            if (rc->right_wall && *(rc->right_wall)) {
                _BG_draw_wall(bg, left, up, right, down);
            }
            if (col < 3) {
                left += 8;
            } else {
                left += 7;
            }
            right = left  + 1;
        }
        up = down;
        left = 1;
        right = 8;
        for (u8 col = 0; col < ROOM_W; ++col) {
            Room_Cell *rc = r->cells[row][col];
            if (rc->down_wall && *(rc->down_wall)) {
                _BG_draw_wall(bg, left, up, right, down);
            }
            left = right;
            if (col < 3) {
                right += 8;
            } else {
                right += 7;
            }
        }
        down += 6;
        left = 8;
        right = 9;
    }
    Room_del(r);
}
