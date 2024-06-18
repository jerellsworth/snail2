#ifndef MENU_H
#define MENU_H

#include "bh.h"

#define MENU_ITEM_MAX_OPTIONS 8

struct Menu_Item_s {

    s16 x; // these are all in tiles
    s16 y;

    char *label;
    u16 label_len;

    char *options[MENU_ITEM_MAX_OPTIONS];
    u8 n_options;
    u8 option_selected;

    Menu *parent;
    struct Menu_Item_s *prev;
    struct Menu_Item_s *next;

    Menu_Select_CB select_cb;
};

struct Menu_s {
    s16 x;
    s16 y;

    Menu_Item *first;
    Menu_Item *last;
    Menu_Item *cursor;

    u8 cooldown;

    Sprite *spr_cursor;

    Sprite *pl_sprites[MAX_N_PLAYERS];

    bool completed;
};

Menu *Menu_new(s16 x, s16 y);
void Menu_del(Menu *m);
void Menu_draw(Menu *m);
void Menu_next_item(Menu *m);
void Menu_prev_item(Menu *m);
void Menu_next_option(Menu *m);
void Menu_prev_option(Menu *m);
void Menu_select(Menu *m);
void Menu_run(Menu *m, BG *bg);
void Menu_refresh_cursor(Menu *m);
void Menu_hide(Menu *m);

Menu_Item *Menu_add_item(Menu *m, char *label);
void Menu_Item_del(Menu_Item *mi);
void Menu_Item_add_option(Menu_Item *mi, char *option);
void Menu_Item_next_option(Menu_Item *mi);
void Menu_Item_prev_option(Menu_Item *mi);
void Menu_Item_draw(Menu_Item *mi);
void Menu_Item_select(Menu_Item *mi);

#endif
