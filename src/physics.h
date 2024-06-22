#ifndef PHYSICS_H
#define PHYSICS_H

#include "bh.h"

struct Physics_s {
    const SpriteDefinition *spriteDef;
    u16 uid; // TODO not really unique, just distributed
    u8 pal;
    Sprite *sp;
    Sprite *shadow;
    u16 anim_no;
    u16 n_tiles;
    u16 h, w;
    fixy y, col_y, hbox_offset_y;
    fixx x, col_x, hbox_offset_x;
    fixz z, col_z;
    fixx start_x;
    fixy start_y;
    fixz start_z;

    fix16 dx, dy, ddx, ddy, dz, ddz; 
    Facing f;
    u16 facing_degrees;
    s16 ttl;
    s16 reg;
    fixx center_offset_x;
    fixy center_offset_y;
    fixz center_offset_z;
    u16 thresh_sq;
    Thing what;
    u8 iframes;
    u16 frames_alive;
    Phy *partner;
    Phy *tgt;
    fixx goal_x;
    fixy goal_y;
    fixz goal_z;
    u16 theta;

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

    bool bg_element;
    u16 bg_tile_idx;

    u8 state;
    u16 state_frames;

    fix16 mass;

    Player *pl;

    u8 live_spawn;
    u8 *instance_counter;

    fix16 dx_after_dash, dy_after_dash;

    bool blocked;

    fix16 buffer_dx;
    fix16 buffer_dy;

    // TODO dbg
    Sprite *cursor;
};

s16 Physics_register(Physics *p);

void Physics_del(Physics *p, Enc *e);

Physics *Physics_new(
        Enc *e,
        const SpriteDefinition *spriteDef,
        u8 pal,
        fixx x,
        fixy y,
        bool bg_element
        );

void Physics_update(Encounter *enc, Physics *p);
void Physics_update_all(Encounter *enc);

bool collision_box(Phy *p1, Phy *p2);
u32 Physics_dist(Physics *p1, Physics *p2);
void Physics_del_all(Enc *e);
void Physics_bg_element_redraw(Physics *p, bool hflip);
Phy *Physics_collides_with_anything(Phy *p);
Phy *Physics_collides_with_anything_visual(Phy *p);
u8 Physics_count(Thing t);

extern Physics **ALL_PHYSICS_OBJS;

#endif
