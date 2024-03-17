#ifndef PHYSICS_H
#define PHYSICS_H

#include "bh.h"

typedef enum {
    DOWN,
    LEFT,
    RIGHT,
    UP
} Facing;

struct Physics_s {
    const SpriteDefinition *spriteDef;
    u16 uid; // TODO not really unique, just distributed
    u8 pal;
    Sprite *sp;
    u16 h, w;
    fixy y, col_y;
    fixx x, col_x;
    fix16 dx, dy, ddx, ddy; 
    Facing f;
    u16 facing_degrees;
    s16 ttl;
    s16 reg;
    fixx center_offset_x;
    fixy center_offset_y;
    u16 thresh_sq;
    Thing what;
    u8 iframes;
    u16 frames_alive;
    Phy *partner;
    Phy *tgt;
    u16 theta;
    bool update_direction;

    fix16 terminal_velocity_up;
    fix16 terminal_velocity_down;
    fix16 lateral_resistance;

    bool collision; // can collide with other Physics and bg tiles
    bool grav_model; // apply downward force
    bool bouncy; // colliding will change the object's velocity
    bool elastic; // Object will _not_ lose momentum after hitting a bg tile
    bool drop_on_collide;
    bool calc_collisions; // if both objects have FALSE, don't bother calculating collision
    bool ignore_walls;

    bool get_in_able;

    u8 state;
    u16 state_frames;

    u8 dash_frames;
    fix16 dash_dx;
    fix16 dash_dy;

    fix16 mass;

    Player *pl;

    MSEG *ms;
    u8 ms_seg_no;

    u8 live_spawn;
    u8 *instance_counter;

    fix16 dx_after_dash, dy_after_dash;

    bool blocked;
    bool charged;
};

s16 Physics_register(Physics *p);

void Physics_del(Physics *p, Enc *e);

Physics *Physics_new(
        const SpriteDefinition *spriteDef,
        u8 pal
        );

void Physics_update(Encounter *enc, Physics *p);
void Physics_update_all(Encounter *enc);

bool collision(Physics *p1, Physics *p2, u32 thresh);
bool collision_box(Phy *p1, Phy *p2);
u32 Physics_dist(Physics *p1, Physics *p2);
void Physics_direction_to(Physics *from, Physics *to, fix16 *norm_x, fix16 *norm_y, u16 *theta);
void Physics_set_visibility_all(Thing t, u16 newVisibility);
void Physics_del_all(Enc *e);
void Physics_del_all_thing(Enc *e, Thing t);

extern Physics **ALL_PHYSICS_OBJS;
void Physics_spr_set_position_all(Enc *enc);

#endif
