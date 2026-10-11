/*
 * Eye of the Beholder II - START.EXE
 * The 3D view: overlay module 3A5D (Ghidra 4A5D), stub segment 2ABA.
 *
 * Addresses in comments are Ghidra addresses (code segment + 0x1000) in
 * START_FLAT.EXE (the overlays unpacked by unoverlay.py).
 * Functions in address order, as in the original file; compiled with
 *     bcc -c -ml -G -O -Z -y -Y view.c   (-Y: overlay code: the address of a function goes through its stub)
 * [asm] = identical to START.EXE byte for byte (tools/bcc.py).
 *
 * How EOB draws a dungeon
 * -----------------------
 * The view window is 176x120 pixels = 22x15 tiles of 8x8. The walls are not
 * drawn with pixels: the view is first built as a map of tile numbers.
 *
 *   ViewTiles[0][15][22]  walls, cleared every frame (0 = transparent)
 *   ViewTiles[1][15][22]  background (floor + ceiling), fully rewritten
 *
 * A tile word is: bit 15 = wall tile with holes (composed over the
 * background), bit 14 = mirrored horizontally, bits 0-13 = tile in the .VCN.
 *
 * Wall pictures are tile layouts stored in the .VMP file (DUNG.VMP...):
 *     u16 count, background[330], then one 431-word block per wall set.
 * A wall seen at a given distance/position is a w x h rectangle at a fixed
 * offset inside its set's block. Right-hand walls reuse the left-hand ones,
 * mirrored. Walls are painted from the farthest squares to the nearest, so
 * the nearer ones overwrite the farther ones (painter's algorithm).
 *
 * The 18 visible squares (party = Q, looking up):
 *
 *        A  B  C  D  E  F  G      distance 3   (indices 0-6)
 *           H  I  J  K  L         distance 2   (7-11)
 *              M  N  O            distance 1   (12-14)
 *              P  Q  R            the party's row (15-17)
 *
 * The background is drawn normal or mirrored depending on the parity of
 * (x + y + direction): every step or turn flips it, which is what makes the
 * floor and ceiling seem to move.
 *
 * Then compose_view (2A68, assembler) turns the tile map into pixels with
 * the .VCN tiles, and everything else (decorations, items, doors, monsters,
 * flying objects) is drawn over it as shapes, square by square from the
 * farthest to the nearest (DrawOrder), each clipped to the columns of its
 * square that nearer walls do not hide (view_clip).
 *
 * tools/render_view.py runs the wall algorithm on the game files and
 * produces the game's views (out/view_test.png).
 */
#include "eob2.h"

typedef unsigned char byte;

extern struct Dim { int x, y, w, h, a, b, c, d; } Dims[];   /* 1CFC the windows (x, w in bytes) */
extern signed char    WallSet[256];      /* 2178 wall type -> VMP wall set (0 = nothing to draw) */
extern signed char    WallTypeFlags[256];   /* 2268 01 passable, 08 see-through, 10 door open,
                                            20 door closed, 80 door */
extern signed char    FaceFront[4];      /* 22B8 the wall face looking at the party, per direction */
extern signed char    ViewSquares[4 * 18];  /* 22C4 block offsets of A..R for each direction */
extern signed char    SquareRow[18];     /* 2330 0 = farthest row .. 3 = party row */
extern unsigned       ViewTiles[2][15][22];   /* 987E */
extern int            ViewBlock[18];     /* 9446 */
extern char           Front;             /* 9444 the face of the walls looking at the party */
extern char           Left, Right;       /* DF2B, DF4A the faces seen on the left, on the right */
extern int            PartyDirection;    /* 6DD4 */
extern int            FloorParity;       /* E54C the background is mirrored */
extern signed char    FaceLeft[4];       /* 22BC */
extern signed char    FaceRight[4];      /* 22C0 */
extern LevelBlock     LevelMap[1024];    /* A360 */
extern int            ViewClip[36];      /* CB7A per square: the columns not hidden (left, right),
                                            -1 = not computed yet */
extern int            ViewTop[18];       /* CBF0 per square: the lines of a door frame */
extern int            ViewBottom[18];    /* DDFC */
extern char far      *ViewWalls[18];     /* DEDC &LevelMap[ViewBlock[i]].walls */
extern unsigned far  *WallLayouts;       /* E4CE .VMP data after the count word */
extern Item           Items[600];        /* 71CA */
extern Monster        Monsters[30];      /* 6E2A */
extern MonsterType    MonsterTypes[];    /* CD1A */
extern unsigned long  PartyEffects;      /* 6DDC 0x220: see the invisible */
extern char           ItemSize[];        /* 2590 per item icon: its floor shape (FloorShapes) */
extern int            FloorObjectX[];    /* 23EC [(square * 5 + place) * 2] x, y of the 4 floor places
                                            and the centre, from the view centre */
extern signed char    PlaceSeen[16];     /* 2352 [party direction * 4 + place] the place on the screen */
extern int            InWallX[18];       /* 230C the x of an item in a wall niche */
extern byte           Remap_23A8[16], Remap_23B8[16];   /* 23A8, 23B8 colour tables of the monsters */
extern byte far      *Var_9318;          /* 9318 the colour variants of the monsters (16 each) */
typedef struct Accessory {
	char far     *shape;
	char          x, y;
} Accessory;                             /* 6 bytes */
extern Accessory      MonsterAccessories[];   /* 936C the accessories of the monsters (.DCR) */
extern char far      *MonsterGfx[];      /* A2D0 slot * 18 + picture - 1 */
extern char far      *FloorShapes[];     /* DE20 the items on the floor (ITEML1, ITEMS1) */
extern byte far      *ShrinkBuffer;      /* CBC4 */
/* Uninitialised data of the module (DS:6C68-6C6B, _BSS), in address order */
int ClipLeft;  /* 6C68 */
int ClipRight; /* 6C6A */
extern void (far     *Drv_654E)(byte far *table);   /* 654E sets the colour remapping of the shapes */
extern void (far     *Drv_6576)(int shade);   /* 6576 */
extern void (far     *Drv_65CE)(char far *shape, byte far *dst);   /* 65CE shrinks a shape one step */

typedef struct Decoration {
	byte          shape[10];     /* +0 the shape of each place, FF = none */
	char          next;          /* +A the next decoration of the wall (from 1), 0 = none */
	char          flags;         /* +B 1 mirrored, 2 mirrored with the floor, 4 x from the centre */
	int           x[10], y[10];  /* +C */
} Decoration;
extern Decoration     Decorations[60];   /* D1CC */
extern char           WallDecoration[256];   /* 21C8 the first decoration of a wall type (from 1) */
extern char far      *DecoShapes[300];  /* DF50 */
void draw_shape_mirrored(int page, char far *shape, int x, int y, int clip);   /* 18D7:06EE */
typedef struct DoorButton { char far *shape; byte x, y; } DoorButton;
extern DoorButton     DoorButtons[];     /* 931E the knob drawn on doors */
extern char far      *DoorShapes[6];    /* EF06 slot * 3 + size (0 = nearest) */
extern int            DoorKind[2];       /* 9366 per slot: 0 sliding, 1 two layers, 2 two halves */
extern int            DoorNoKnob[2];     /* EE34 */
extern char           WallClickType[256];   /* 2218 */
extern Missile        Thrown[10];        /* CC5C flying items and spells */
extern char far      *ThrownShapes[];   /* E404 */
extern signed char    ThrownOfSize[];    /* 2601 per floor shape: its flying shapes, -1 none */
extern char far      *Var_E54E[6];       /* E54E the sparkles of wall type 0x2C */
extern char far      *Var_DED4;          /* DED4 the lightning */
extern char far      *Shapes[];         /* CC34 */
extern int            AnimPhase;         /* 6D7E toggled every 10 ticks */
int  roll_dice(char count, char sides, char add);                /* 18D7:0505 */
extern int            PartyBlock;        /* 6DD2 */
extern byte far      *VcnTiles;          /* CB72 */
extern char           FadeInPending;     /* EE3B */
extern volatile unsigned long Countdown;   /* 668C counts down to 0 */
extern volatile unsigned long TimerTicks;  /* 667C */
extern char           Var_DF2C;          /* DF2C the party has moved */
extern unsigned long  Var_EEF2;          /* EEF2 not before this tick after a move */
extern int            NoViewCopy;        /* E4D2 */
extern int            InDialogue;        /* EE38 */
extern char           BookOpen;         /* E441 the spell book is open */
extern char           SceneDirty;        /* D0AA */
extern char far      *Var_92A8[12];      /* 92A8 the compass: 3 parts x 4 directions */
extern byte far      *Palette;           /* F168 */
extern int            Var_62A5;          /* 62A5 the page drawn on */
extern void (far     *Drv_657A)(int x1, int y1, int x2, int y2, int colour);   /* 657A fills */
void compose_view(byte far *vcn);                                /* 2A68:03ED (assembler) */
void play_sound_here(int sound);                                 /* 2A30:0000 */
void mouse_hide_rect(int x1, int y1, int x2, int y2);            /* 338B:0206 */
void mouse_show_rect(void);                                      /* 338B:02CA */
void put_shape(int page, char far *shape, int x, int y, int dim);   /* 2DA3:004A */
void set_palette(byte far *pal);                                 /* 3111:000D */
void copy_page(int x, int y, int x2, int y2, int w, int h, int from, int to);   /* 2D92:000A */
void clip_window(int page, int x, int y, int w, int h);          /* 310C:0009 x, w in columns */
void draw_shape(int page, char far *shape, int x, int y, int clip);   /* 18D7:06AD */
void far view_setup(int block, int dir);
void far set_clip(int page, int x, int y, int x2, int y2);
void far view_clip(int sq, int far *left, int far *right, int page);
void far door_clip(char sq, int far *top, int far *bottom, int page);
void far draw_wall(char x, char y, char type, int offset, char w, char h);
void far draw_wall_mirrored(char x, char y, char type, int offset, char w, char h);
int far is_solid(int type);
void far build_view(int block, int dir);
char far * far scaled_shape(char far *shape, byte far *buffer, int level);
void far draw_items(int sq, int item);
void far draw_monster_shape(char far *shape, int x, int y, int mode, int flags, int remap);
void far draw_monsters(int sq);

void far draw_decorations(int sq);

void far draw_door(int sq);

void far draw_thrown(int sq);

void far draw_special_2c(int sq);
void far draw_special_4a(int sq);
void far draw_lightning(void);
void far draw_view_contents(int first);

void far draw_compass(int force);
void far draw_scene(int full);
void far view_nothing(void);

void draw_shape_2(int page, char far *shape, int x, int y, int clip);   /* 18D7:072F */
void draw_shape_3(int page, char far *shape, int x, int y, int clip);   /* 18D7:0770 */

/* Initialised data of the module (DS:0092), made by tools/viewdata.py; the far
 * table Sparkles is the far data of the module (DS:0090 is the end of C0L's
 * __heaptop, DS:0091 the padding before this module) */
signed char Occlusion[324] = { /* 0092 [sq * 18 + other] a solid wall on other: -41 hides sq,
 *  -40 nothing, n > 0 cuts it on the right at column n, -n on the left */
	-41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -40, -41,
	-40, -40, 2, -40, -40, -40, -40, -2, -41, -40, -40, -40, -41, 3, -40, -3, -40, -40,
	-40, -2, -40, 8, -40, -40, -40, -2, -41, 6, -40, -40, -6, 3, -40, -3, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40, -6, -41, 16, -40, -3, -41, 19, -40, -40, -40,
	-40, -40, -40, -14, -40, 20, -40, -40, -40, -16, -41, 20, -40, -19, 16, -40, -40, 19,
	-40, -40, -40, -40, -20, -40, -40, -40, -40, -40, -41, 20, -40, -19, -41, -40, -40, 19,
	-41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -40, -41,
	-41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -40, -41,
	-40, -40, -40, -40, -40, -40, -40, -40, -40, 6, -40, -40, -6, 3, -40, -3, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40, -6, -40, 16, -40, -3, -41, 19, -3, -40, 19,
	-40, -40, -40, -40, -40, -40, -40, -40, -40, -16, -40, -40, -40, -19, 16, -40, -40, 19,
	-41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -41, -40, -41,
	-40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, 3, -40, -3, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -3, -40, 19, -3, -40, 19,
	-40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -19, -40, -40, -40, 19,
	-40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40, -40
};
signed char OcclusionSeeThrough[648] = { /* 01D6 [(sq * 18 + other) * 2] left, right: a see-through wall */
	22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 0, 22, 22, 0,
	0, 22, 0, 22, 0, 2, 0, 22, 0, 22, 0, 22, 0, 22, 2, 22, 0, 4, 0, 22, 0, 22, 0, 22, 22, 0, 0, 3, 0, 22, 3, 22, 0, 22, 0, 22,
	0, 22, 2, 22, 0, 22, 0, 8, 0, 22, 0, 22, 0, 22, 0, 2, 22, 0, 0, 6, 0, 22, 0, 22, 6, 22, 0, 3, 0, 22, 3, 0, 0, 22, 0, 22,
	0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 6, 22, 8, 14, 0, 16, 0, 22, 3, 22, 6, 16, 0, 19, 0, 22, 0, 22, 0, 22,
	0, 22, 0, 22, 0, 22, 14, 22, 0, 22, 0, 20, 0, 22, 0, 22, 0, 22, 16, 22, 22, 0, 0, 20, 0, 22, 19, 22, 0, 16, 0, 22, 0, 22, 0, 19,
	0, 22, 0, 22, 0, 22, 0, 22, 20, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 18, 22, 20, 22, 0, 22, 19, 22, 22, 0, 0, 22, 0, 22, 0, 19,
	22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 0, 22, 22, 0,
	22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 0, 22, 22, 0,
	0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 6, 0, 22, 0, 22, 6, 22, 0, 3, 0, 22, 3, 22, 0, 22, 0, 22,
	0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 6, 22, 0, 22, 0, 16, 0, 22, 3, 22, 7, 15, 0, 19, 3, 22, 0, 22, 0, 19,
	0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 16, 22, 0, 22, 0, 22, 0, 22, 19, 22, 0, 16, 0, 22, 0, 22, 0, 19,
	22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 0, 22, 22, 0,
	0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 3, 0, 22, 3, 22, 0, 22, 0, 22,
	0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 3, 22, 0, 22, 0, 19, 3, 22, 0, 22, 0, 19,
	0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 19, 22, 0, 22, 0, 22, 0, 22, 0, 19,
	0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22,
	0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22,
	0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22, 0, 22
};
signed char DoorFrameTop[4] = { /* 045E [row] the lines of a door frame */
	30, 24, 16, 0
};
signed char DoorFrameBottom[4] = { /* 0462 */
	59, 71, 86, 120
};
signed char SquareBehind[18] = { /* 0466 the square seen through a see-through wall, -1 none */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, 3, -1, -1, -1, 9, -1, 11, 12, 13
};
signed char ItemScale[16] = { /* 0478 [row * 4 + place] how much an item is shrunk, -1 not seen */
	-1, -1, 3, 3, 2, 2, 2, 2, 1, 1, 1, 1, 0, 0, -1, -1
};
int AirY[3] = { /* 0488 [row] the y of an item in a niche (row 3 reads ItemJitter) */
	37, 49, 56
};
int ItemJitter[8] = { /* 048E */
	0, -2, 1, -1, 2, 0, 1, -1
};
void (far *DrawShapeFns[4])() = { /* 049E */
	draw_shape, draw_shape_mirrored, draw_shape_2, draw_shape_3
};
signed char MonsterScale[3] = { /* 04AE [row] (row 3 reads MonsterPose2) */
	2, 1, 0
};
signed char MonsterPose2[32] = { /* 04B1 [phase * 16 + party dir * 4 + monster dir] */
	4, -2, 1, 3, 3, 4, -2, 1, 1, 3, 4, -2, -2, 1, 3, 4,
	-4, -3, -1, 2, 2, -4, -3, -1, -1, 2, -4, -3, -3, -1, 2, -4
};
signed char MonsterPose[32] = { /* 04D1 */
	4, -2, 1, 3, 3, 4, -2, 1, 1, 3, 4, -2, -2, 1, 3, 4,
	4, -3, 1, 2, 2, 4, -3, 1, 1, 2, 4, -3, -3, 1, 2, 4
};
signed char MonsterPlace[20] = { /* 04F1 [party dir * 5 + pos] */
	0, 1, 2, 3, 4, 2, 0, 3, 1, 4, 2, 3, 0, 1, 4, 1, 3, 0, 2, 4
};
signed char BigMonsterY[3] = { /* 0505 */
	20, 12, 4
};
signed char BigMonsterY2[3] = { /* 0508 */
	4, 2, 0
};
char far *DecoFace[36] = { /* 050B [sq * 2 + side] the face seen, 0 = none */
	0, 0, &Front, &Left, &Front, &Left,
	&Front, 0, &Front, &Right, &Front, &Right,
	0, 0, &Front, &Left, &Front, &Left,
	&Front, 0, &Front, &Right, &Front, &Right,
	&Front, &Left, &Front, 0, &Front, &Right,
	0, &Left, &Front, 0, 0, &Right
};
signed char DecoPlace[36] = { /* 059B its place (1-10), < 0 mirrored */
	1, 1, 4, 10, 4, 8, 4, 1, 4, -8, 4, -10, 0, 0, 3, 9, 3, 7,
	3, 1, 3, -7, 3, -9, 2, 6, 2, 1, 2, -6, 1, 5, 1, 1, 1, -5
};
int DecoFrontX[18] = { /* 05BF */
	-144, -96, -48, 0, 48, 96, 144, -160, -80, 0, 80, 160, -128, 0, 128, 0, 0, 0
};
signed char DoorBottomY[4] = { /* 05E3 [row] */
	59, 71, 86, 0
};
signed char DoorTopY[4] = { /* 05E7 */
	31, 24, 15, 0
};
signed char DoorClosedType[53] = { /* 05EB [wall type] the closed state of its door */
	0, 0, 0, 3, 3, 3, 3, 3, 8, 8, 8, 8, 8, 13, 13, 13, 13, 13,
	18, 18, 18, 18, 18, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 51, 52
};
signed char DoorSlotOfType[53] = { /* 0620 [wall type] the door shapes (slot) */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1
};
signed char DoorStep[8] = { /* 0655 [row] pixels per opening step */
	8, 12, 18, 0, 0, 2, 4, 0
};
signed char DoorHalfStep[4] = { /* 065D */
	5, 8, 12, 0
};
signed char DoorLift[6] = { /* 0661 */
	5, 3, 1, 5, 3, 1
};
signed char ThrownScale[72] = { /* 0667 [sq * 4 + place] */
	-1, -1, -1, -1, -1, -1, -1, 3, -1, -1, 3, 3, -1, -1, 3, 3, -1, -1,
	3, 3, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, -1, -1, -1, -1, -1, 1, -1, 1, 1, 1,
	1, 1, 1, -1, 1, -1, -1, -1, -1, -1, 0, 0, -1, -1, -1, -1, -1, -1
};
signed char ThrownPlace[20] = { /* 06AF [party dir * 5 + pos] */
	0, 1, 2, 3, 4, 2, 0, 3, 1, 4, 2, 3, 0, 1, 4, 1, 3, 0, 2, 4
};
signed char ThrownMirror[16] = { /* 06C3 [dir * 4 + pos] */
	0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 0
};
signed char Sparkle2cX[3] = { /* 06D3 */
	40, 28, 18
};
signed char Sparkle2cY[3] = { /* 06D6 */
	13, 21, 26
};
int Special4aX[18] = { /* 06D9 */
	-52, -12, 28, 68, 108, 148, 188, -72, -8, 56, 120, 184, -56, 40, 136, 0, 0, 0
};
signed char Special4aRows[3] = { /* 06FD */
	2, 6, 9
};
signed char Special4aCols[3] = { /* 0700 */
	1, 2, 3
};
signed char Special4aY[3] = { /* 0703 */
	32, 24, 16
};
signed char Special4aFrame[3] = { /* 0706 */
	4, 2, 0
};
void (far *LightningFns[2])() = { /* 0709 */
	draw_shape, draw_shape_mirrored
};
signed char DrawOrder[18] = { /* 0711 the squares from the farthest: A G B F C E D  H L I K J  M O N  P R Q */
	0, 6, 1, 5, 2, 4, 3, 7, 11, 8, 10, 9, 12, 14, 13, 15, 17, 16
};
int CompassX[3] = { /* 0723 */
	114, 79, 151
};
int CompassY[3] = { /* 0729 */
	131, 158, 158
};
int CompassDir = -1; /* 072F the direction drawn */
signed char far Sparkles[6][13][2] = {   /* 3409:0000 the sparkles of wall type 0x2C */
	{ { 12, 7 }, { 26, 1 }, { 62, 3 }, { 12, 26 }, { 42, 19 }, { 64, 24 }, { 2, 45 }, { 22, 37 }, { 40, 50 }, { 54, 39 }, { 10, 62 }, { 22, 73 }, { 62, 68 } },
	{ { 6, 6 }, { 42, 4 }, { 55, 10 }, { 4, 27 }, { 26, 22 }, { 55, 29 }, { 14, 42 }, { 27, 53 }, { 46, 40 }, { 66, 48 }, { 6, 71 }, { 6, 71 }, { 45, 76 } },
	{ { 10, 4 }, { 20, 0 }, { 46, 1 }, { 12, 16 }, { 31, 16 }, { 47, 16 }, { 18, 24 }, { 40, 29 }, { 1, 33 }, { 8, 42 }, { 17, 50 }, { 47, 46 }, { 31, 37 } },
	{ { 2, 2 }, { 1, 17 }, { 1, 47 }, { 8, 30 }, { 17, 14 }, { 17, 38 }, { 28, 1 }, { 30, 25 }, { 31, 51 }, { 36, 17 }, { 38, 5 }, { 40, 43 }, { 47, 34 } },
	{ { 0, 19 }, { 5, 1 }, { 6, 8 }, { 9, 12 }, { 4, 26 }, { 8, 31 }, { 18, 5 }, { 18, 21 }, { 22, 16 }, { 26, 8 }, { 26, 29 }, { 10, 0 }, { 10, 0 } },
	{ { 0, 9 }, { 0, 30 }, { 4, 17 }, { 8, 22 }, { 8, 6 }, { 16, 0 }, { 17, 13 }, { 18, 32 }, { 21, 2 }, { 20, 9 }, { 22, 27 }, { 26, 20 }, { 26, 20 } }
};

/* 4A5D:0000 Which squares are visible from block, facing dir */
void far view_setup(int block, int dir)
{
	int b;
	register int i;

	dir *= 18;
	for (i = 0; i < 18; i++) {
		b = ViewBlock[i] = (block + ViewSquares[dir + i]) & 0x3FF;
		ViewWalls[i] = LevelMap[b].walls;
		ViewClip[i] = ViewClip[i + 18] = -1;
	}
}

/* 4A5D:0066 The clip window of the shapes, in columns and tile lines */
void far set_clip(int page, int x, int y, int x2, int y2)
{
	clip_window(page, x, y * 8, x2 - x, (y2 - y) * 8);
}

/* 4A5D:009C The columns of square sq not hidden by nearer walls (kept in
 * ViewClip), and the clip window set to them */
void far view_clip(int sq, int far *left, int far *right, int page)
{
	int i, n, v, top, bottom, s;
	register int k, w;

	s = sq;
	k = sq * 2;
	if (ViewClip[k] != -1) {
		*left = ViewClip[k];
		*right = ViewClip[k + 1];
	} else {
		*left = 0;
		*right = 22;
		top = 0;
		bottom = 120;
		n = sq * 2;
		sq *= 18;
		for (i = 0; i < 18; i++) {
			w = ViewWalls[i][Front];
			if (!(WallTypeFlags[w] & 8)) {
				v = Occlusion[sq + i];
				if (WallSet[w] == 0)
					continue;
				if (v == -40)
					continue;
				if (v == -41) {
					*left = 22;
					*right = 0;
					break;
				}
				if (v > 0 && *right > v)
					*right = v;
				if (v < 0 && -v > *left)
					*left = -v;
			} else {
				k = (sq + i) * 2;
				if (OcclusionSeeThrough[k] > *left) {
					*left = OcclusionSeeThrough[k];
					if (!(WallTypeFlags[w] & 0x10))
						door_clip(s, &top, &bottom, -1);
				}
				k++;
				if (OcclusionSeeThrough[k] < *right) {
					*right = OcclusionSeeThrough[k];
					if (!(WallTypeFlags[w] & 0x10))
						door_clip(s, &top, &bottom, -1);
				}
			}
			if (*right < *left)
				break;
		}
		ViewTop[s] = top;
		ViewBottom[s] = bottom;
		ViewClip[n] = *left;
		n++;
		ViewClip[n] = *right;
	}
	set_clip(page, *left, 0, *right, 15);
}

/* 4A5D:02A2 The lines of the door frame of square sq; page != -1: the clip
 * window of page set to them */
void far door_clip(char sq, int far *top, int far *bottom, int page)
{
	register int row;

	row = SquareRow[sq];
	if (page == -1 && row != 3)
		row++;
	*top = DoorFrameTop[row];
	*bottom = DoorFrameBottom[row];
	if (page != -1)
		clip_window(page, Dims[page].x, *top, Dims[page].w, *bottom - *top);
}

/* 4A5D:030C Copies a w x h tile rectangle of a wall layout to (x, y),
 * clipped to the 22 columns; 0 tiles are transparent */
void far draw_wall(char x, char y, char type, int offset, char w, char h)
{
	unsigned far *dst;
	unsigned far *src;
	register int col, row;

	if (WallSet[type] == 0)
		return;
	src = WallLayouts + (WallSet[type] - 1) * 431 + offset;
	src += 330;
	for (row = 0; row < h; row++) {
		dst = &ViewTiles[0][0][(y + row) * 22 + x];
		for (col = 0; col < w; col++)
			if (x + col >= 0 && x + col < 22) {
				if (*src)
					*dst++ = *src++;
				else {
					dst++;
					src++;
				}
			} else {
				dst++;
				src++;
			}
	}
}

/* 4A5D:03D1 The same, mirrored: the columns read right to left, bit 14
 * toggled */
void far draw_wall_mirrored(char x, char y, char type, int offset, char w, char h)
{
	int row;
	unsigned far *src;
	int col;
	register int t;

	if (WallSet[type] == 0)
		return;
	src = WallLayouts + (WallSet[type] - 1) * 431 + offset;
	src += 330;
	for (row = 0; row < h; row++)
		for (col = 0; col < w; col++)
			if (x + col >= 0 && x + col < 22) {
				t = *(src + w * row + (w - 1 - col));
				if (t) {
					if (t & 0x4000)
						t -= 0x4000;
					else
						t |= 0x4000;
					ViewTiles[0][0][(y + row) * 22 + x + col] = t;
				}
			}
}

/* 4A5D:049B A wall that hides what is behind it */
int far is_solid(int type)
{
	return type != 0 && !(WallTypeFlags[type] & 8);
}

#define WALL(sq, face)  ViewWalls[sq][face]

/* 4A5D:04BA Builds the tile map of the view: the background, then the
 * walls from the farthest to the nearest */
void far build_view(int block, int dir)
{
	int a, b;
	register int i;

	Front = FaceFront[PartyDirection];
	Left = FaceLeft[PartyDirection];
	Right = FaceRight[PartyDirection];
	for (i = 0; i < 330; i++)
		ViewTiles[0][0][i] = 0;
	FloorParity = ((block >> 5) + (block & 0x1F) + PartyDirection) & 1;
	if (FloorParity == 0)
		draw_wall(0, 15, 1, -330, 22, 15);  /* the background is before the first set */
	else
		draw_wall_mirrored(0, 15, 1, -330, 22, 15);
	view_setup(block, dir);

	/* distance 3: the side faces */
	if ((a = WALL(0, Left)) != 0)
		draw_wall(-2, 3, a, 0x66, 3, 5);
	if ((a = WALL(6, Right)) != 0)
		draw_wall_mirrored(21, 3, a, 0x66, 3, 5);
	a = WALL(1, Left);
	b = WALL(2, Front);
	if (is_solid(a) && !(WallTypeFlags[b] & 8))
		draw_wall(2, 3, a, 0x66, 3, 5);
	else if (a && (WallTypeFlags[b] & 8))
		draw_wall(2, 3, b, 0x66, 3, 5);
	a = WALL(5, Right);
	b = WALL(4, Front);
	if (is_solid(a) && !(WallTypeFlags[b] & 8))
		draw_wall_mirrored(17, 3, a, 0x66, 3, 5);
	else if (a && (WallTypeFlags[b] & 8))
		draw_wall_mirrored(17, 3, b, 0x66, 3, 5);
	if ((a = WALL(2, Left)) != 0)
		draw_wall(8, 3, a, 0x61, 1, 5);
	if ((a = WALL(4, Right)) != 0)
		draw_wall_mirrored(13, 3, a, 0x61, 1, 5);
	/* distance 3: the front faces */
	if (is_solid(WALL(1, Front)))
		draw_wall(-4, 3, WALL(1, Front), 0x81, 6, 5);
	if (is_solid(WALL(5, Front)))
		draw_wall(20, 3, WALL(5, Front), 0x81, 6, 5);
	if (is_solid(WALL(2, Front)))
		draw_wall(2, 3, WALL(2, Front), 0x81, 6, 5);
	if (is_solid(WALL(4, Front)))
		draw_wall(14, 3, WALL(4, Front), 0x81, 6, 5);
	if ((a = WALL(3, Front)) != 0)
		draw_wall(8, 3, a, 0x81, 6, 5);

	/* distance 2 */
	if ((a = WALL(7, Left)) != 0)
		draw_wall(0, 3, a, 0x75, 2, 6);
	if ((a = WALL(11, Right)) != 0)
		draw_wall_mirrored(20, 3, a, 0x75, 2, 6);
	if ((a = WALL(8, Left)) != 0)
		draw_wall(6, 2, a, 0x51, 2, 8);
	if ((a = WALL(10, Right)) != 0)
		draw_wall_mirrored(14, 2, a, 0x51, 2, 8);
	if (is_solid(WALL(8, Front)))
		draw_wall(-4, 2, WALL(8, Front), 0x9F, 10, 8);
	if (is_solid(WALL(10, Front)))
		draw_wall(16, 2, WALL(10, Front), 0x9F, 10, 8);
	if ((a = WALL(9, Front)) != 0)
		draw_wall(6, 2, a, 0x9F, 10, 8);

	/* distance 1 */
	if ((a = WALL(12, Left)) != 0)
		draw_wall(3, 1, a, 0x2D, 3, 12);
	if ((a = WALL(14, Right)) != 0)
		draw_wall_mirrored(16, 1, a, 0x2D, 3, 12);
	a = WALL(12, Front);
	b = WALL(13, Front);
	if (!(WallTypeFlags[a] & 8))
		draw_wall(-13, 1, a, 0xEF, 16, 12);
	a = WALL(14, Front);
	if (!(WallTypeFlags[a] & 8))
		draw_wall(19, 1, a, 0xEF, 16, 12);
	if ((a = WALL(13, Front)) != 0)
		draw_wall(3, 1, a, 0xEF, 16, 12);

	/* the walls beside the party */
	a = WALL(15, Left);
	b = WALL(17, Right);
	if (a)
		draw_wall(0, 0, a, 0, 3, 15);
	if (b)
		draw_wall_mirrored(19, 0, b, 0, 3, 15);
}

/* 4A5D:0B09 The shape shrunk level times (in buffer) */
char far * far scaled_shape(char far *shape, byte far *buffer, int level)
{
	Drv_6576(level);
	if (!level)
		return shape;
	while (level) {
		Drv_65CE(shape, buffer);
		shape = buffer;
		level--;
	}
	return buffer;
}

/* 4A5D:0B50 The items lying on square sq (the list after item) */
void far draw_items(register int sq, int item)
{
	int x, level, place, first, k, flags;
	char loop;
	char far *shape;
	char inwall;
	char wall;
	char behind;
	Item far *it;
	register int y;

	wall = LevelMap[ViewBlock[sq]].walls[Front];
	if (sq == 16)
		flags = 0x80;
	else
		flags = WallTypeFlags[wall];
	if (WallSet[wall] != 0 && !(flags & 0x80))
		return;
	first = item = Items[item].next;
	loop = 1;
	while (item != first || loop) {
		it = &Items[item];
		inwall = it->pos == 8;
		if (inwall == 1 || it->pos < 4) {
			behind = -1;
			place = PlaceSeen[PartyDirection * 4 + (it->pos & 3)];
			if (inwall)
				place = 4;
			k = (sq * 5 + place) * 2;
			x = FloorObjectX[k] + 88;
			if (inwall) {
				x = InWallX[sq];
				y = AirY[SquareRow[sq]];
				place = 0;
			} else
				y = FloorObjectX[k + 1] + 124;
			level = ItemScale[SquareRow[sq] * 4 + place];
			if ((flags & 8) && place < 2 && level) {
				behind = SquareBehind[sq];
				if (behind != -1)
					view_clip(behind, &ClipLeft, &ClipRight, 5);
				y -= 4;
			}
			if (level >= 0) {
				shape = scaled_shape(FloorShapes[ItemSize[it->icon]], ShrinkBuffer, level);
				x -= shape[2] * 8 >> 1;
				x += ItemJitter[item & 7] * 2;
				y -= shape[1];
				if (!inwall)
					y += ItemJitter[(item >> 1) & 7];
				draw_shape(2, shape, x, y, 5);
				Drv_6576(0);
			}
		}
		item = it->next;
		loop = 0;
		/* ORIGINAL BUG: behind is not set when the first item is not drawn */
		if (behind != -1)
			view_clip(sq, &ClipLeft, &ClipRight, 5);
	}
}

/* 4A5D:0DBB A monster shape, with its colours (flags 2 or 0x20, or one of
 * its variants: remap); mode 1 mirrored */
void far draw_monster_shape(char far *shape, int x, int y, int mode, int flags, int remap)
{
	if (flags & 2) {
		Drv_654E(Remap_23A8);
		mode += 2;
	} else if (flags & 0x20) {
		Drv_654E(Remap_23B8);
		mode += 2;
	}
	if (remap != -1 && mode < 2) {
		mode += 2;
		Drv_654E(Var_9318 + remap);
	}
	DrawShapeFns[mode](2, shape, x, y, 5);
}

/* 4A5D:0E2A The monsters on square sq, with their accessories */
void far draw_monsters(int sq)
{
	MonsterType far *mt;
	Monster far *m;
	int pose, k, frame, ax, ay, n_acc, remap, level, blk, i, place, j, n, d4, slot, w8, sx, sy, a, big;
	char far *shape;
	char far *acc;
	Monster far *at[5];
	register int x, y;

	level = MonsterScale[SquareRow[sq]];
	if ((blk = ViewBlock[sq]) == 0)
		return;
	j = PartyDirection * 5;
	d4 = PartyDirection * 4;
	for (i = 0; i < 5; i++)
		at[i] = 0;
	m = Monsters;
	for (i = 0; i < 30; i++, m++)
		if (m->block == blk)
			at[MonsterPlace[j + m->pos]] = m;
	for (j = 0; j < 5; j++) {
		if (!at[j])
			continue;
		m = at[j];
		mt = &MonsterTypes[m->type];
		if ((mt->flags & 0x100) && !(PartyEffects & 0x220) && (m->flags & 2) == 0)
			continue;
		if (mt->flags & 2)
			pose = MonsterPose2[((char)m->phase << 4) + d4 + m->facing];
		else
			pose = MonsterPose[((char)m->phase << 4) + d4 + m->facing];
		if (level == 0 && m->attack < 0)
			pose = m->attack + 7;
		frame = pose < 0 ? -pose : pose;
		if (m->slot)
			slot = 18;
		else
			slot = 0;
		if (slot == 18)
			remap = frame + 5;
		else
			remap = frame - 1;
		if (m->colour)
			remap = (remap << 5) + ((char)m->colour - 1 << 4);
		else
			remap = -1;
		shape = scaled_shape(MonsterGfx[frame + slot - 1], ShrinkBuffer, level);
		big = frame == 1 || frame > 3;
		if (m->pos != 4)
			place = PlaceSeen[d4 + m->pos];
		else
			place = 4;
		k = (sq * 5 + place) * 2;
		x = FloorObjectX[k] + 88;
		y = FloorObjectX[k + 1] + 127;
		if (mt->big == 1) {
			if (big) {
				y -= FloorObjectX[k + 1];
				k = ((k >> 1) - place) * 2;
				y += FloorObjectX[k + 1];
				if (place == 4)
					y += BigMonsterY[level];
				else
					y += BigMonsterY2[level];
			} else {
				x -= FloorObjectX[k];
				k = ((k >> 1) - place + 4) * 2;
				x += FloorObjectX[k];
			}
		}
		w8 = shape[2] * 8;
		x -= w8 >> 1;
		y -= shape[1];
		x += ((char)m->idle & 0xF0) >> 4;
		y += (char)m->idle & 0xF;
		draw_monster_shape(shape, x, y, pose < 0, m->flags, remap);
		sx = x;
		sy = y;
		for (a = 0; a < 3; a++) {
			if (mt->accessory[a] == 0)
				continue;
			n_acc = slot + ((char)mt->accessory[a] - 1) * 6 + frame - 1;
			acc = MonsterAccessories[n_acc].shape;
			if (!acc)
				continue;
			x = sx;
			y = sy;
			ax = MonsterAccessories[n_acc].x;
			ay = MonsterAccessories[n_acc].y;
			if (mt->accessory[a])
				n = level;
			else
				n = 0;
			while (n) {
				Drv_65CE(acc, ShrinkBuffer);
				ax = ax * 2 / 3;
				ay = ay * 2 / 3;
				acc = ShrinkBuffer;
				n--;
			}
			if (pose >= 0)
				x += ax;
			else
				x = x + w8 - ax - acc[2] * 8;
			y += ay;
			draw_monster_shape(acc, x, y, pose < 0, m->flags, -1);
		}
		Drv_6576(0);
	}
}

/* 4A5D:12E4 The decorations of the walls of square sq: the side face
 * first, then the front face */
void far draw_decorations(int sq)
{
	char fa, fb, a, b, pl;
	int y, place, flags;
	byte shp;
	register int d, x;

	a = sq * 2;
	b = a + 1;
	if (DecoFace[b]) {
		fb = *DecoFace[b];
		for (d = WallDecoration[ViewWalls[sq][fb]]; d > 0; d = Decorations[d].next) {
			d--;
			place = DecoPlace[b];
			pl = (place < 0 ? -place : place) - 1;
			shp = Decorations[d].shape[pl];
			if (shp == 0xFF)
				continue;
			if (place >= 0)
				x = Decorations[d].x[pl];
			else {
				x = 176 - Decorations[d].x[pl];
				x -= DecoShapes[shp][2] * 8;
			}
			y = Decorations[d].y[pl];
			if (place >= 0)
				draw_shape(2, DecoShapes[shp], x, y, 5);
			else
				draw_shape_mirrored(2, DecoShapes[shp], x, y, 5);
		}
	}
	if (DecoFace[a]) {
		fa = *DecoFace[a];
		for (d = WallDecoration[ViewWalls[sq][fa]]; d > 0; d = Decorations[d].next) {
			d--;
			flags = Decorations[d].flags;
			place = DecoPlace[a];
			if ((flags & 1) || ((flags & 2) && FloorParity))
				place = -place;
			pl = (place < 0 ? -place : place) - 1;
			shp = Decorations[d].shape[pl];
			if (shp == 0xFF)
				continue;
			if (place >= 0) {
				if (flags & 4)
					x = Decorations[d].x[pl] + FloorObjectX[(sq * 5 + 4) * 2];
				else
					x = Decorations[d].x[pl] + DecoFrontX[sq];
			} else {
				if (flags & 4)
					x = 176 - Decorations[d].x[pl] + FloorObjectX[(sq * 5 + 4) * 2];
				else
					x = 176 - Decorations[d].x[pl] + DecoFrontX[sq];
				x -= DecoShapes[shp][2] * 8;
			}
			y = Decorations[d].y[pl];
			if (place >= 0)
				draw_shape(2, DecoShapes[shp], x, y, 5);
			else
				draw_shape_mirrored(2, DecoShapes[shp], x, y, 5);
		}
	}
}

/* 4A5D:166A The door on square sq (its frame clipped), and its knob */
void far draw_door(int sq)
{
	int top, bottom, x, row, n, cx, slot, off;
	char far *shape;
	register int w, y;

	w = ViewWalls[sq][Front];
	slot = DoorSlotOfType[w];
	row = SquareRow[sq];
	n = slot * 3 + 2 - row;
	shape = DoorShapes[n];
	cx = FloorObjectX[(sq * 5 + 4) * 2];
	x = cx + 88;
	door_clip(sq, &top, &bottom, 5);
	switch (DoorKind[slot]) {
	case 0:
	case 1:
		y = DoorBottomY[row] - shape[1];
		x -= shape[2] * 8 >> 1;
		if (DoorKind[slot] == 1) {
			draw_shape(2, DoorShapes[n], x, y, 5);
			shape = DoorShapes[n + 3];
		}
		y -= (w - DoorClosedType[w]) * DoorStep[row];
		if (WallClickType[w] == 5)
			y -= DoorLift[n];
		draw_shape(2, shape, x, y, 5);
		break;
	case 2:
		x -= shape[2] * 8 >> 1;
		y = DoorTopY[row];
		off = (w - DoorClosedType[w]) * DoorHalfStep[row];
		draw_shape(2, shape, x, y - off, 5);
		shape = DoorShapes[n + 3];
		y = DoorBottomY[row] - shape[1];
		off >>= 1;
		off--;
		draw_shape(2, shape, x, y + off, 5);
		break;
	}
	if (WallDecoration[w] == -1)
		switch (DoorNoKnob[slot]) {
		case 0:
			x = DoorButtons[n].x + cx;
			y = DoorButtons[n].y;
			draw_shape(2, DoorButtons[n].shape, x, y, 5);
		}
	set_clip(5, ClipLeft, 0, ClipRight, 15);
}

/* 4A5D:18DF The items and spells flying through square sq */
void far draw_thrown(int sq)
{
	Missile far *t;
	int view, place, x, level, blk, mirror, sq4, d4, sq5, k, wall, j, size, shade;
	char far *shape;
	Missile far *at[5];
	register int y, i;

	blk = ViewBlock[sq];
	wall = LevelMap[blk].walls[Front];
	if (WallSet[wall] != 0 && (WallTypeFlags[wall] & 0x80) == 0)
		return;
	sq4 = sq * 4;
	d4 = PartyDirection * 4;
	sq5 = sq * 5;
	j = PartyDirection * 5;
	for (i = 0; i < 5; i++)
		at[i] = 0;
	t = Thrown;
	for (i = 0; i < 10; i++, t++)
		if (t->active && t->block == blk)
			at[ThrownPlace[j + (t->pos & 3)]] = t;
	for (j = 0; j < 5; j++) {
		if (!at[j])
			continue;
		t = at[j];
		place = PlaceSeen[d4 + (t->pos & 3)];
		k = (sq5 + place) * 2;
		x = FloorObjectX[k] + 88;
		level = ThrownScale[sq4 + place];
		mirror = 0;
		if (level >= 0) {
			if (t->active == 1) {
				size = ItemSize[Items[t->item].icon];
				if (t->dir == PartyDirection)
					view = 0;
				else if (t->dir == (PartyDirection ^ 2))
					view = 1;
				else
					view = -1;
				if (view != -1 && ThrownOfSize[size] != -1) {
					shape = ThrownShapes[ThrownOfSize[size] + view];
					mirror = ThrownMirror[t->dir * 4 + t->pos];
				} else {
					shape = FloorShapes[size];
					mirror = t->dir == ((PartyDirection + 1) & 3);
				}
				y = 39;
			} else {
				shade = 1;
				shape = ThrownShapes[t->shape];
				mirror = ThrownMirror[t->dir * 4 + t->pos];
				if (t->flags & 0x40) {
					x = FloorObjectX[(sq5 + 4) * 2] + 88;
					y = 44;
				} else
					y = 39;
			}
			shape = scaled_shape(shape, ShrinkBuffer, level);
			/* sic: shade is not set for an item (only a spell sets it) */
			if (shade) {
				Drv_6576(0);
				shade = 0;
			}
			x -= shape[2] * 8 >> 1;
			y -= y != 44 ? shape[1] : shape[1] >> 1;
			if (!mirror)
				draw_shape(2, shape, x, y, 5);
			else
				draw_shape_mirrored(2, shape, x, y, 5);
		}
		Drv_6576(0);
	}
}

/* 4A5D:1C31 Wall type 0x2C: 13 sparkles twice (two frames alternating with
 * AnimPhase), on the squares 1 to 3 rows away */
void far draw_special_2c(int sq)
{
	int r, bx, by, ox, oy, i;
	char far *shape;
	register int k, n;

	r = 2 - SquareRow[sq];
	if (r < 0)
		return;
	bx = InWallX[sq] - Sparkle2cX[r];
	by = Sparkle2cY[r];
	for (i = 0; i < 2; i++) {
		n = r * 2 + i;
		ox = oy = 0;
		if (n == 0) {
			ox = -4;
			oy = -4;
		}
		shape = Var_E54E[n ^ AnimPhase];
		for (k = 0; k < 13; k++)
			draw_shape(2, shape, ox + bx + Sparkles[n][k][0], oy + by + Sparkles[n][k][1], 5);
	}
}

/* 4A5D:1D2A Wall type 0x4A: a pattern of shapes (Shapes[3..]), the frames
 * alternating row by row and with AnimPhase */
void far draw_special_4a(int sq)
{
	int row, rows, cols, y, frame, h, w8, i;
	register int x, j;

	row = SquareRow[sq];
	rows = Special4aRows[row];
	cols = Special4aCols[row];
	y = Special4aY[row];
	frame = Special4aFrame[row] + AnimPhase;
	h = Shapes[3 + frame][1];
	w8 = Shapes[3 + frame][2] * 8;
	for (i = 0; i < rows; i++) {
		x = Special4aX[sq];
		for (j = 0; j < cols; j++) {
			draw_shape(2, Shapes[3 + frame], x, y, 5);
			x += w8;
		}
		y += h;
		frame ^= 1;
	}
}

/* 4A5D:1DEC A flash of lightning down the middle of the view (the script
 * effect E2 0) */
void far draw_lightning(void)
{
	int m;
	register int y, i;

	y = 0;
	m = roll_dice(1, 2, -1);
	for (i = 0; i < 6; i++) {
		m ^= 1;
		LightningFns[m](2, Var_DED4, 72, y, 5);
		y += 64;
	}
}

/* 4A5D:1E44 What lies on the squares, from the farthest (first: the place in
 * DrawOrder) to the nearest */
void far draw_view_contents(int first)
{
	int w, k;
	register int sq;

	for (; first < 18; first++) {
		sq = DrawOrder[first];
		view_clip(sq, &ClipLeft, &ClipRight, 5);
		if (ClipRight < ClipLeft)
			continue;
		k = sq * 2;
		draw_decorations(sq);
		w = ViewWalls[sq][Front];
		if (((LevelBlock far *)ViewWalls[sq])->items)
			draw_items(sq, ((LevelBlock far *)ViewWalls[sq])->items);
		if (sq < 15) {
			if (WallTypeFlags[w] & 8)
				draw_door(sq);
			if (((LevelBlock far *)ViewWalls[sq])->flags & 7) {
				clip_window(5, Dims[5].x, ViewTop[sq], Dims[5].w, ViewBottom[sq] - ViewTop[sq]);
				draw_monsters(sq);
				set_clip(5, ViewClip[k], 0, ViewClip[k + 1], 15);
			}
			if (w == 0x4A)
				draw_special_4a(sq);
		}
		draw_thrown(sq);
		if (w == 0x2C)
			draw_special_2c(sq);
	}
}

/* 4A5D:1F7A The compass (3 shapes), when the direction changed or force */
void far draw_compass(int force)
{
	register int i;

	if (CompassDir != PartyDirection || force) {
		mouse_hide_rect(70, 121, 176, 175);
		for (i = 0; i < 3; i++)
			put_shape(Var_62A5, Var_92A8[PartyDirection + i * 4], CompassX[i], CompassY[i], 0);
		mouse_show_rect();
		CompassDir = PartyDirection;
	}
}

/* 4A5D:1FF4 Draws the 3D view: the tile map, the pixels, the contents of the
 * squares; full: on the screen too (and the compass) */
void far draw_scene(int full)
{
	build_view(PartyBlock, PartyDirection);
	compose_view(VcnTiles);
	draw_view_contents(0);
	if (FadeInPending) {
		if (full) {
			mouse_hide_rect(0, 0, 176, 120);
			Drv_657A(0, 0, 176, 120, 12);
			mouse_show_rect();
		}
		set_palette(Palette);
		FadeInPending = 0;
	}
	while (Countdown)
		;
	if (Var_DF2C)
		while (Var_EEF2 > TimerTicks)
			;
	if (full && !NoViewCopy) {
		mouse_hide_rect(0, 0, 176, 120);
		copy_page(0, 0, 0, 0, 22, 120, 2, 0);
		mouse_show_rect();
	}
	play_sound_here(0);
	if (!InDialogue && full && !BookOpen)
		draw_compass(0);
	if (Var_DF2C) {
		Var_DF2C = 0;
		Var_EEF2 = TimerTicks + 4;
	}
	SceneDirty = 0;
}

/* 4A5D:2127 Does nothing (called after a teleport) */
void far view_nothing(void)
{
	register int i;

	i = 0;
	i++;
}
