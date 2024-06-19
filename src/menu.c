#include "bh.h"

Menu *Menu_new(s16 x, s16 y) {
    Menu *m = ct_calloc(sizeof(Menu), 1);
    m->x = x;
    m->y = y;
    return m;
}

void Menu_del(Menu *m) {
    Menu_Item *mi = m->first;
    if (mi) {
        while (TRUE) {
            Menu_Item *mi_next = mi->next;
            Menu_Item_del(mi);
            if (mi_next == mi) break;
            mi = mi->next;
        }
    }
    if (m->spr_cursor) SPR_releaseSprite(m->spr_cursor);
    free(m);
}

void Menu_draw(Menu *m) {
    if (!m->first) return;
    Menu_Item *mi = m->first;
    while (TRUE) {
        Menu_Item_draw(mi);
        if (mi == m->last) return;
        mi = mi->next;
    }
}

void Menu_next_item(Menu *m) {
    Menu_Item *mi = m->cursor;
    if (mi) {
        m->cursor = mi->next;
        Menu_refresh_cursor(m);
    }
}

void Menu_prev_item(Menu *m) {
    Menu_Item *mi = m->cursor;
    if (mi) {
        m->cursor = mi->prev;
        Menu_refresh_cursor(m);
    }
}

void Menu_next_option(Menu *m) {
    Menu_Item *mi = m->cursor;
    if (mi) {
        Menu_Item_next_option(mi);
        Menu_Item_draw(mi);
    }
}

void Menu_prev_option(Menu *m) {
    Menu_Item *mi = m->cursor;
    if (mi) {
        Menu_Item_prev_option(mi);
        Menu_Item_draw(mi);
    }
}

void Menu_select(Menu *m) {
    Menu_Item *mi = m->cursor;
    if (mi) {
        if (mi->select_cb) mi->select_cb(mi);
    }
}

void Menu_input(Menu *m) {
    if (m->cooldown > 0) --m->cooldown;
    if (m->cooldown > 0) return;
    u16 joy = JOY_readJoypad(JOY_ALL);
    if (joy & BUTTON_ALL) m->cooldown = 10;
    if (joy & BUTTON_UP) {
        Menu_prev_item(m);
    } else if (joy & BUTTON_DOWN) {
        Menu_next_item(m);
    } else if (joy & BUTTON_LEFT) {
        Menu_prev_option(m);
    } else if (joy & BUTTON_RIGHT) {
        Menu_next_option(m);
    } else if (joy & (BUTTON_A | BUTTON_B | BUTTON_C | BUTTON_START)) {
        Menu_select(m);
    }
}

void Menu_run(Menu *m, BG *bg) {
    PAL_setPalette(PAL1, PAL_SPRITE1.data, FALSE);
    m->spr_cursor = SPR_addSprite(
            &SPR_CURSOR,
            m->x,
            m->y,
            TILE_ATTR(PAL1, TRUE, FALSE, FALSE)
            );
    m->completed = FALSE;
    Menu_draw(m);
    Menu_refresh_cursor(m);
    while (!m->completed) {
        random();
        Menu_input(m);
        SPR_update();

        BG_update(bg);

        SYS_doVBlankProcess();
    }

    for (u8 i = 0; i < MAX_N_PLAYERS; ++i) {
        Sprite *sp = m->pl_sprites[i];
        if (sp) {
            SPR_releaseSprite(sp);
            m->pl_sprites[i] = NULL;
        }
    }

    Menu_hide(m);
}

void Menu_refresh_cursor(Menu *m) {
    Menu_Item *mi = m->cursor;
    if (mi) {
        SPR_setPosition(m->spr_cursor, mi->x * 8, mi->y * 8);
    }
}

Menu_Item *Menu_add_item(Menu *m, char *label) {
    Menu_Item *mi = ct_calloc(sizeof(Menu_Item), 1);
    mi->parent = m;
    mi->label_len = strlen(label);
    mi->label = heap_str(label);
    mi->n_options = 0;
    if (!m->first) {
        m->first = mi;
        m->last = mi;
        m->cursor = mi;
        mi->x = m->x;
        mi->y = m->y;
        mi->prev = mi;
        mi->next = mi;
    } else {
        mi->prev = m->last;
        mi->next = m->first;
        mi->prev->next = mi;
        mi->next->prev = mi;
        mi->x = m->last->x;
        mi->y = m->last->y + 1;
        m->last = mi;
    }
    return mi;
}

void Menu_Item_del(Menu_Item *mi) {
    Menu *m = mi->parent;
    free(mi->label);
    for (u8 i = 0; i < mi->n_options; ++i) {
        free((mi->options)[i]);
    }
    if (mi->next == mi) {
        // if this is the only node, reset menu
        m->first = NULL;
        m->last = NULL;
        m->cursor = NULL;
    } else {
        if (mi == m->first) m->first = mi->next;
        if (mi == m->cursor) m->cursor = mi->next;
        if (mi == m->last) m->last = mi->prev;
        mi->prev->next = mi->next;
        mi->next->prev = mi->prev;
        // TODO change y coordinates of later items
    }
    free(mi);
}

void Menu_Item_add_option(Menu_Item *mi, char *option) {
    (mi->options)[mi->n_options] = heap_str(option);
    ++mi->n_options;
}

void Menu_Item_next_option(Menu_Item *mi) {
    if (mi->n_options <= 0) return;
    mi->option_selected = (mi->option_selected + 1) % mi->n_options;
}

void Menu_Item_prev_option(Menu_Item *mi) {
    if (mi->n_options <= 0) return;
    if (mi->option_selected <= 0) {
        mi->option_selected = mi->n_options - 1;
        return;
    }
    --mi->option_selected;

}

void Menu_Item_draw(Menu_Item *mi) {
    VDP_clearTextBG(BG_A, mi->x, mi->y, 40);
    // TODO could do this without clearing the whole line
    VDP_drawTextBG(BG_A, mi->label, mi->x + 1, mi->y);
    if (mi->n_options <= 0) return;
    VDP_drawTextBG(
        BG_A,
        (mi->options)[mi->option_selected],
        mi->x + 1 + mi->label_len + 1,
        mi->y);
}

void Menu_Item_select(Menu_Item *mi) {
    if (mi->select_cb) mi->select_cb(mi);
}

void Menu_hide(Menu *m) {
    SPR_releaseSprite(m->spr_cursor);
    VDP_init();
    SYS_doVBlankProcess();
    VDP_waitVSync();
    VDP_init(); // I don't know why you need to do this, but you do
}
