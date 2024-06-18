#ifndef BG_TILE_MANAGER_H
#define BG_TILE_MANAGER_H

#include "bh.h"

typedef struct _TM_Elmt_s {
    const SpriteDefinition *sprDef;
    u16 idx;
    struct _TM_Elmt_s *next;
} _TM_Elmt;

struct Tile_Manager_s {
    _TM_Elmt *first;
    u16 next_idx;
};

Tile_Manager *Tile_Manager_new(void);
void Tile_Manager_del(Tile_Manager *tm);
u16 SpriteDefinition_VDP_idx(Tile_Manager *tm, const SpriteDefinition *sprDef);
// if sprDef has already been loaded, return idx of tile in VDP
// otherwise, load tileset, then return idx


#endif
