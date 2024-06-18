#include "bh.h"

Tile_Manager *Tile_Manager_new(void) {
    Tile_Manager *tm = ct_calloc(1, sizeof(Tile_Manager));
    tm->next_idx = 128; // TODO figure out what this is actually supposed to be
    tm->first = ct_calloc(1, sizeof(struct _TM_Elmt_s));
    /* lazily allocating a first element that will always miss so we can avoid some special cases */
    return tm;
}

void _Tile_Manager_del_elmt(struct _TM_Elmt_s *elmt) {
    if (!elmt) return;
    if (!elmt->next) {
        free(elmt);
        return;
    }
    _Tile_Manager_del_elmt(elmt->next);
    free(elmt);
}

u8 _Tile_Manager_find_elmt(Tile_Manager *tm, struct _TM_Elmt_s *elmt, const SpriteDefinition *sprDef) {
    if (elmt->sprDef == sprDef) return elmt->idx;
    if (!elmt->next) {
        struct _TM_Elmt_s *new_elmt = ct_calloc(1, sizeof(struct _TM_Elmt_s));
        new_elmt->sprDef = sprDef;

        new_elmt->idx = tm->next_idx;
        u16 idx = new_elmt->idx;
        for (u16 i = 0; i < sprDef->numAnimation; ++i) {
            Animation* anim = sprDef->animations[i];
            for (u16 j = 0; j < anim->numFrame; ++j) {
                TileSet *spr_tileset = anim->frames[j]->tileset;
                VDP_loadTileData(spr_tileset->tiles, idx, spr_tileset->numTile, DMA_QUEUE);
                idx += spr_tileset->numTile;
            }
        }
        tm->next_idx = idx;
        elmt->next = new_elmt;
        return new_elmt->idx;
    }
    return _Tile_Manager_find_elmt(tm, elmt->next, sprDef);
}

void Tile_Manager_del(Tile_Manager *tm) {
    _Tile_Manager_del_elmt(tm->first);
    free(tm);
}

u16 SpriteDefinition_VDP_idx(Tile_Manager *tm, const SpriteDefinition *sprDef) {
    return _Tile_Manager_find_elmt(tm, tm->first, sprDef);
}
