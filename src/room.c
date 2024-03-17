#include "bh.h"

void _Set_push(_Set *s, void *obj) {
    ++s->len;
    for (u8 i = 0; i < ROOM_MAX_WALLS; ++i) {
        if (!(s->data[i])) {
            s->data[i] = obj;
            return;
        }
    }
}

void *_Set_random_pop(_Set *s) {
    u8 picked = random_with_max(s->len - 1);
    --s->len;
    u8 found = 0;
    for (u8 i = 0; i < ROOM_MAX_WALLS; ++i) {
        if (s->data[i]) {
            if (found == picked) {
                void *ret = s->data[i];
                s->data[i] = NULL;
                return ret;
            }
            ++found;
        }
    }
    return NULL;
}

bool _region_split(Room *r, u8 src_region, u8 tgt_region) {

    u8 ct = 0;
    for (u8 row = 0; row < ROOM_H; ++row) {
        for (u8 col = 0; col < ROOM_W; ++col) {
            Room_Cell *c = r->cells[row][col];
            if (c->region == src_region) ++ct;
        }
    }
    if (ct <= 1) return FALSE;
    _Set *S = ct_calloc(sizeof(_Set), 1);
    u8 seed1 = random_with_max(ct - 1);
    u8 seed2 = random_with_max(ct - 1);
    while (seed2 == seed1) {
        seed2 = random_with_max(ct - 1);
    }

    u8 i = 0;
    for (u8 row = 0; row < ROOM_H; ++row) {
        for (u8 col = 0; col < ROOM_W; ++col) {
            if (i == seed1) {
                Room_Cell *c = r->cells[row][col];
                c->region = tgt_region;
                _Set_push(S, c);
            } else if (i == seed2) {
                Room_Cell *c = r->cells[row][col];
                c->region = tgt_region + 1;
                _Set_push(S, c);
            }
            ++i;
        }
    }
    // grow seeds into regions, delete some random walls between them
    while (S->len > 0) {
        Room_Cell *c = _Set_random_pop(S);
        if (c->up && c->up->region == src_region) {
            c->up->region = c->region;
            _Set_push(S, c->up);
        }
        if (c->down && c->down->region == src_region) {
            c->down->region = c->region;
            _Set_push(S, c->down);
        }
        if (c->left && c->left->region == src_region) {
            c->left->region = c->region;
            _Set_push(S, c->left);
        }
        if (c->right && c->right->region == src_region) {
            c->right->region = c->region;
            _Set_push(S, c->right);
        }
    }

    for (u8 row = 0; row < ROOM_H; ++row) {
        for (u8 col = 0; col < ROOM_W; ++col) {
            Room_Cell *c = r->cells[row][col];
            if (c->right && \
                (c->region == tgt_region || c->region == tgt_region + 1) && \
                (c->right->region == tgt_region || c->right->region == tgt_region + 1) && \
                c->region != c->right->region
                ) {
                _Set_push(S, c->right_wall);
            }
            if (c->down && \
                (c->region == tgt_region || c->region == tgt_region + 1) && \
                (c->down->region == tgt_region || c->down->region == tgt_region + 1) && \
                c->region != c->down->region
                ) {
                _Set_push(S, c->down_wall);
            }
        }
    }

    while (S->len > 1) {
        *((bool *)_Set_random_pop(S)) = TRUE;
    }

    free(S);

    return TRUE;
}

bool _Room_partitioned(Room *r) {
    _Set *S = ct_calloc(sizeof(_Set), 1);
    _Set_push(S, r->cells[0][0]);
    while (S->len > 0) {
        Room_Cell *s = _Set_random_pop(S);
        if(s->marked) continue;
        s->marked = TRUE;
        if (s->right_wall && !(*(s->right_wall)) && (!(s->right->marked))) {
            _Set_push(S, s->right);
        }
        if (s->down_wall && !(*(s->down_wall)) && (!(s->down->marked))) {
            _Set_push(S, s->down);
        }
        if (s->left_wall && !(*(s->left_wall)) && (!(s->left->marked))) {
            _Set_push(S, s->left);
        }
        if (s->up_wall && !(*(s->up_wall)) && (!(s->up->marked))) {
            _Set_push(S, s->up);
        }
    }
    free(S);
    for (u8 row = 0; row < ROOM_H; ++row) {
        for (u8 col = 0; col < ROOM_W; ++col) {
            Room_Cell *c = r->cells[row][col];
            if (!(c->marked)) return TRUE;
        }
    }
    return FALSE;
}

Room *Room_new(u8 start_row, u8 start_col, u8 n_splits) {
    // https://weblog.jamisbuck.org/2015/1/15/better-recursive-division-algorithm.html
    Room *r;
    new_room_start:
    r = ct_calloc(1, sizeof(Room));
    for (u8 row = 0; row < ROOM_H; ++row) {
        for (u8 col = 0; col < ROOM_W; ++col) {
            Room_Cell *c = ct_calloc(1, sizeof(Room_Cell));
            c->row = row;
            c->col = col;
            c->down_wall = NULL;
            c->right_wall = NULL;
            c->up_wall = NULL;
            c->left_wall = NULL;
            if (row < ROOM_H - 1) c->down_wall = ct_calloc(1, sizeof(bool));
            if (col < ROOM_W - 1) c->right_wall = ct_calloc(1, sizeof(bool));
            r->cells[row][col] = c;
        }
    }
    for (u8 row = 0; row < ROOM_H; ++row) {
        for (u8 col = 0; col < ROOM_W; ++col) {
            Room_Cell *c = r->cells[row][col];
            if (row > 0) {
                c->up = r->cells[row - 1][col];
                c->up_wall = c->up->down_wall;
            }
            if (row < ROOM_H - 1) {
                c->down = r->cells[row + 1][col];
            }
            if (col > 0) {
                c->left = r->cells[row][col - 1];
                c->left_wall = c->left->right_wall;
            }
            if (col < ROOM_W - 1) {
                c->right = r->cells[row][col + 1];
            }
        }
    }

    u8 tgt_region = 1;
    for (u8 src_region = 0; src_region < n_splits; ++src_region) {
        _region_split(r, src_region, tgt_region);
        tgt_region += 2;
    }

    if (_Room_partitioned(r)) {
        // final check to make sure this is actually solvable
        Room_del(r);
        goto new_room_start;
    }
    return r;
}

void Room_del(Room *r) {
    for (u8 row = 0; row < ROOM_H; ++row) {
        for (u8 col = 0; col < ROOM_W; ++col) {
            Room_Cell *c = r->cells[row][col];
            if (c->down_wall) free(c->down_wall);
            if (c->right_wall) free(c->right_wall);
            free(c);
        }
    }
    free(r);
}
