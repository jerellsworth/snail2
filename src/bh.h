#ifndef BH_H
#define BH_H

#define MAX_PHYSICS_OBJS 80
#define GRAVITY FIX16(0.125)
#define MAX_N_PLAYERS 4
#define COLLISION_THRESH 512

#define MAP_TILES_W 40
#define MAP_TILES_H 28

#define MAX_MSEG_SEGMENTS 32
#define SPRING_CONST FIX16(15)
#define COEF_FROM_CENTER FIX16(0.5)
#define SPRING_THRESH FIX16(0.1)

#define XBITS 16
#define YBITS 16

#define _fixx fix16
#define _fixy fix16

#define fixxToInt(x) fix16ToInt(x)
#define fixyToInt(x) fix16ToInt(x)
#define fixxToRoundedInt(x) fix16ToRoundedInt(x)
#define fixyToRoundedInt(x) fix16ToRoundedInt(x)
#define fixxToFix16(x) (x)
#define fixyToFix16(x) (x)
#define fix16ToFixx(x) (x)
#define fix16ToFixy(x) (x)

#define FIXX(x) FIX16(x)
#define FIXY(x) FIX16(x)

#undef TGT_WIN

/* Big 'ol Header file */

typedef struct Physics_s Physics;
typedef Physics Phy;

typedef struct Encounter_s Encounter;
typedef Encounter Enc;

typedef struct Player_s Player;

typedef struct BG_s BG;

typedef struct MSEG_s MSEG;
typedef struct MSEG_INSTR_s MSEG_INSTR;

typedef enum Thing_e Thing;

typedef struct KPM_s KPM;

typedef struct Menu_Item_s Menu_Item;
typedef struct Menu_s Menu;

typedef int (*Menu_Select_CB)(Menu_Item *);

typedef struct Room_s Room;
typedef struct Room_Cell_s Room_Cell;

#include <genesis.h>
#include <maths.h>
#include <memory.h>

typedef _fixx fixx;
typedef _fixy fixy;

#include "resources.h"
#include "utils.h"
#include "encounter.h"
#include "physics_init.h"
#include "physics.h"
#include "player.h"
#include "behavior.h"
#include "bg.h"
#include "sound.h"
#include "collision.h"
#include "intro.h"
#include "multiseg.h"
#include "precalc.h"
#include "kpm.h"
#include "menu.h"
#include "room.h"

#endif
