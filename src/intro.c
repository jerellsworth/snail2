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
    //XGM_startPlay(XGM_MENU);
    //bool skip = m != NULL;
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
            //skip = TRUE;
            break;
        }
        SYS_doVBlankProcess();
    }
    PAL_fadeOut(0, 63, 30, FALSE);
    SYS_doVBlankProcess();
    SYS_doVBlankProcess();
    VDP_clearPlane(BG_B, TRUE);
    BG *bg = BG_init(
        &MAP_TITLE_BG,
        &TLS_TITLE_BG,
        &MAP_TITLE_FG,
        &TLS_TITLE_FG,
        &COLLISION_BG,
        PAL_BG.data
        );

    if (m == NULL) {
        m = Menu_new(19, 24);

        Menu_Item *mi_music = Menu_add_item(m, "Music");
        Menu_Item_add_option(mi_music, "On");
        Menu_Item_add_option(mi_music, "Off");
        mi_music->select_cb = &go_to_go_cb;

        Menu_Item *mi_go = Menu_add_item(m, "Go");
        mi_go->select_cb = &go_cb;
    }

    Menu_run(m, bg);
    BG_del(bg);
    return m;
}
