#ifndef ROOM_H_123
#define ROOM_H_123

#define ROOM_W 9
#define ROOM_H 8
#define ROOM_MAX_WALLS 31

#include "bh.h"

typedef enum {
    WALL_OPEN,
    WALL_CLOSED,
    WALL_DOOR
} Room_Wall;


struct Room_Cell_s {
    u8 row;
    u8 col;
    struct Room_Cell_s *up;
    struct Room_Cell_s *down;
    struct Room_Cell_s *left;
    struct Room_Cell_s *right;
    bool *up_wall; // ref
    bool *down_wall; // owned
    bool *left_wall; // ref
    bool *right_wall; // owned
    u8 region;
    bool marked;
};

typedef struct _Set_s {
    void *data[ROOM_MAX_WALLS];
    u8 len;
} _Set;

struct Room_s {
    Room_Cell *cells[ROOM_H][ROOM_W];
    u8 start_row;
    u8 start_col;
};

Room *Room_new(u8 start_row, u8 start_col, u8 n_splits);
void Room_del(Room *r);

#endif
