#include "bh.h"

int go_cb(Menu_Item *mi) {
    Menu *m = mi->parent;
    m->completed = TRUE;
    return 0;
}

int go_to_go_cb(Menu_Item *mi) {
    Menu *m = mi->parent;
    m->cursor = m->last;
    Menu_refresh_cursor(m);
    return 0;
}

Menu *INTRO_run(Menu *m) {

    bool skip = m != NULL;
    PAL_setPalette(PAL0, palette_black, FALSE);
    VDP_drawImage(
        BG_B,
        &IMG_PRODUCTION,
        0,
        0
        );
    SYS_doVBlankProcess();
    SYS_doVBlankProcess();
    PAL_fadeTo(0, 15, IMG_PRODUCTION.palette->data, 30, FALSE);
    for (u8 i = 0; i < 120; ++i) {
        u16 joy = JOY_readJoypad(JOY_ALL);
        if (joy & BUTTON_ALL) {
            skip = TRUE;
            break;
        }
        SYS_doVBlankProcess();
    }
    PAL_fadeOut(0, 63, 30, FALSE);
    XGM_startPlay(XGM_TITLE);
    SYS_doVBlankProcess();
    SYS_doVBlankProcess();
    VDP_clearPlane(BG_B, TRUE);
    SYS_doVBlankProcess();
    SYS_doVBlankProcess();
    VDP_drawImageEx(
        BG_B,
        &IMG_HUNTING,
        TILE_ATTR(PAL0, FALSE, FALSE, FALSE),
        0,
        0,
        FALSE,
        TRUE
    );
    SYS_doVBlankProcess();
    SYS_doVBlankProcess();
    SYS_doVBlankProcess();
    VDP_drawImageEx(
        BG_A,
        &IMG_MYTHDRAGON,
        TILE_ATTR_FULL(PAL1, FALSE, FALSE, FALSE, IMG_HUNTING.tileset->numTile),
        0,
        0,
        FALSE,
        TRUE
    );
    SYS_doVBlankProcess();
    SYS_doVBlankProcess();
    SYS_doVBlankProcess();
    PAL_fadeTo(0, 15, IMG_HUNTING.palette->data, 30, FALSE);
    if (!skip) {
        for (u8 i = 0; i < 120; ++i) {
            u16 joy = JOY_readJoypad(JOY_ALL);
            if (joy & BUTTON_ALL) {
                skip = TRUE;
                break;
            }
            SYS_doVBlankProcess();
        }
    }
    PAL_fadeTo(16, 31, IMG_MYTHDRAGON.palette->data, 30, FALSE);
    if (!skip) {
        for (u8 i = 0; i < 60; ++i) {
            u16 joy = JOY_readJoypad(JOY_ALL);
            if (joy & BUTTON_ALL) {
                skip = TRUE;
                break;
            }
            SYS_doVBlankProcess();
        }
    }

    if (!m) {
        m = Menu_new(19, 24);
        Menu_Item *mi_players = Menu_add_item(m, "Players");
        Menu_Item_add_option(mi_players, "1");
        Menu_Item_add_option(mi_players, "2");
        mi_players->select_cb = &go_to_go_cb;

        Menu_Item *mi_music = Menu_add_item(m, "Music");
        Menu_Item_add_option(mi_music, "On");
        Menu_Item_add_option(mi_music, "Off");
        mi_music->select_cb = &go_to_go_cb;

        Menu_Item *mi_go = Menu_add_item(m, "Go");
        mi_go->select_cb = &go_cb;
    }

    Menu_run(m);
    XGM_stopPlay();
    SYS_doVBlankProcess();
    SYS_doVBlankProcess();
    return m;
}
