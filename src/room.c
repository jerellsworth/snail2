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

void _prims_add_to_maze(Room_Cell *c, _Set *frontier, bool first) {
    c->in_maze = TRUE;
    if (!first) {
        _Set *candidate_walls = ct_calloc(sizeof(_Set), 1);
        if ((c->up) && (c->up->in_maze)) {
            _Set_push(candidate_walls, c->up_wall);
        } 
        if ((c->down) && (c->down->in_maze)) {
            _Set_push(candidate_walls, c->down_wall);
        } 
        if ((c->left) && (c->left->in_maze)) {
            _Set_push(candidate_walls, c->left_wall);
        } 
        if ((c->right) && (c->right->in_maze)) {
            _Set_push(candidate_walls, c->right_wall);
        }
        *((bool *)_Set_random_pop(candidate_walls)) = FALSE;
        free(candidate_walls);
    }
    if ((c->up) && (!c->up->in_maze) && (!c->up->in_frontier)) {
        _Set_push(frontier, c->up);
    }
    if ((c->down) && (!c->down->in_maze) && (!c->down->in_frontier)) {
        _Set_push(frontier, c->down);
    }
    if ((c->left) && (!c->left->in_maze) && (!c->left->in_frontier)) {
        _Set_push(frontier, c->left);
    }
    if ((c->right) && (!c->right->in_maze) && (!c->right->in_frontier)) {
        _Set_push(frontier, c->right);
    }
}

void _prims(Room *r) {
    _Set *frontier = ct_calloc(sizeof(_Set), 1);
    Room_Cell *c = r->cells[random_with_max(ROOM_H - 1)][random_with_max(ROOM_W - 1)];
    _prims_add_to_maze(c, frontier, TRUE);
    while (frontier->len > 0) {
        c = (Room_Cell *)_Set_random_pop(frontier);
        _prims_add_to_maze(c, frontier, FALSE);
    }
    free(frontier);
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
            if (row < ROOM_H - 1) {
                c->down_wall = ct_calloc(1, sizeof(bool));
                *(c->down_wall) = TRUE;
            }
            if (col < ROOM_W - 1) {
                c->right_wall = ct_calloc(1, sizeof(bool));
                *(c->right_wall) = TRUE;
            }
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

    _prims(r);

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
